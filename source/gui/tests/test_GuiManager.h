/* Copyright (C) 2020 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lib/self_test.h"

#include "gui/GUIManager.h"

#include "gui/CGUI.h"
#include "ps/ConfigDB.h"
#include "ps/Filesystem.h"
#include "ps/GameSetup/GameSetup.h"
#include "ps/Hotkey.h"
#include "ps/XML/Xeromyces.h"

class TestGuiManager : public CxxTest::TestSuite
{
	CConfigDB* configDB;
public:

	void setUp()
	{
		g_VFS = CreateVfs();
		TS_ASSERT_OK(g_VFS->Mount(L"", DataDir()/"mods"/"_test.gui", VFS_MOUNT_MUST_EXIST));
		TS_ASSERT_OK(g_VFS->Mount(L"cache", DataDir()/"mods"/"_testcache"));

		configDB = new CConfigDB;

		CXeromyces::Startup();

		g_GUI = new CGUIManager();
	}

	void tearDown()
	{
		delete g_GUI;
		CXeromyces::Terminate();
		delete configDB;
		g_VFS.reset();
		DeleteDirectory(DataDir()/"_testcache");
	}

	void test_hotkeysState()
	{
		// Load up a fake test hotkey when pressing 'a'.
		const char* test_hotkey_name = "hotkey.test";
		g_ConfigDB.SetValueString(CFG_USER, test_hotkey_name, "A");
		LoadHotkeys();

		// Load up a test page.
		const ScriptInterface& scriptInterface = *(g_GUI->GetScriptInterface());
		ScriptRequest rq(scriptInterface);
		JS::RootedValue val(rq.cx);
		scriptInterface.CreateObject(rq, &val);

		ScriptInterface::StructuredClone data = scriptInterface.WriteStructuredClone(JS::NullHandleValue, true);
		g_GUI->PushPage(L"hotkey/page_hotkey.xml", data, JS::UndefinedHandleValue);

		// Press 'a'.
		SDL_Event_ hotkeyNotification;
		hotkeyNotification.ev.type = SDL_KEYDOWN;
		hotkeyNotification.ev.key.keysym.scancode = SDL_SCANCODE_A;
		hotkeyNotification.ev.key.repeat = 0;

		// Init input and poll the event.
		InitInput();
		in_push_priority_event(&hotkeyNotification);
		SDL_Event_ ev;
		while (in_poll_event(&ev))
			in_dispatch_event(&ev);

		const ScriptInterface& pageScriptInterface = *(g_GUI->GetActiveGUI()->GetScriptInterface());
		ScriptRequest prq(pageScriptInterface);
		JS::RootedValue global(prq.cx, prq.globalValue());

		// Ensure that our hotkey state was synchronised with the event itself.
		bool hotkey_pressed_value = false;
		JS::RootedValue js_hotkey_pressed_value(prq.cx);

		pageScriptInterface.GetProperty(global, "state_before", &js_hotkey_pressed_value);
		ScriptInterface::FromJSVal(prq, js_hotkey_pressed_value, hotkey_pressed_value);
		TS_ASSERT_EQUALS(hotkey_pressed_value, true);

		hotkey_pressed_value = false;
		pageScriptInterface.GetProperty(global, "state_after", &js_hotkey_pressed_value);
		ScriptInterface::FromJSVal(prq, js_hotkey_pressed_value, hotkey_pressed_value);
		TS_ASSERT_EQUALS(hotkey_pressed_value, true);

		// We are listening to KeyDown events, so repeat shouldn't matter.
		hotkeyNotification.ev.key.repeat = 1;
		in_push_priority_event(&hotkeyNotification);
		while (in_poll_event(&ev))
			in_dispatch_event(&ev);

		hotkey_pressed_value = false;
		pageScriptInterface.GetProperty(global, "state_before", &js_hotkey_pressed_value);
		ScriptInterface::FromJSVal(prq, js_hotkey_pressed_value, hotkey_pressed_value);
		TS_ASSERT_EQUALS(hotkey_pressed_value, true);

		hotkey_pressed_value = false;
		pageScriptInterface.GetProperty(global, "state_after", &js_hotkey_pressed_value);
		ScriptInterface::FromJSVal(prq, js_hotkey_pressed_value, hotkey_pressed_value);
		TS_ASSERT_EQUALS(hotkey_pressed_value, true);

		hotkeyNotification.ev.type = SDL_KEYUP;
		in_push_priority_event(&hotkeyNotification);
		while (in_poll_event(&ev))
			in_dispatch_event(&ev);

		hotkey_pressed_value = true;
		pageScriptInterface.GetProperty(global, "state_before", &js_hotkey_pressed_value);
		ScriptInterface::FromJSVal(prq, js_hotkey_pressed_value, hotkey_pressed_value);
		TS_ASSERT_EQUALS(hotkey_pressed_value, false);

		hotkey_pressed_value = true;
		pageScriptInterface.GetProperty(global, "state_after", &js_hotkey_pressed_value);
		ScriptInterface::FromJSVal(prq, js_hotkey_pressed_value, hotkey_pressed_value);
		TS_ASSERT_EQUALS(hotkey_pressed_value, false);

	}
};
