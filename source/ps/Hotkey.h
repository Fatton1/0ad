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

#ifndef INCLUDED_HOTKEY
#define INCLUDED_HOTKEY

/**
 * @file
 * Hotkey system.
 *
 * Hotkeys consist of a name (an arbitrary string), and a key mapping.
 * The names and mappings are loaded from the config system (any
 * config setting with the name prefix "hotkey.").
 * When a hotkey is pressed one SDL_HOTKEYPRESS is triggered. While the key is
 * kept down repeated SDL_HOTKEYDOWN events are triggered at an interval
 * determined by the OS. When a hotkey is released an SDL_HOTKEYUP event is
 * triggered. All with the hotkey name stored in ev.user.data1 as a const char*.
 */

#include "CStr.h"
#include "lib/input.h"

#include <unordered_map>

// SDL_Scancode is an enum, we'll use an explicit int to avoid including SDL in this header.
using SDL_Scancode_ = int;

// 0x8000 is SDL_USEREVENT, this is static_asserted in Hotkey.cpp
// We do this to avoid including SDL in this header.
const uint SDL_USEREVENT_ = 0x8000;
const uint SDL_HOTKEYPRESS = SDL_USEREVENT_;
const uint SDL_HOTKEYDOWN = SDL_USEREVENT_ + 1;
const uint SDL_HOTKEYUP = SDL_USEREVENT_ + 2;

struct SKey
{
	SDL_Scancode_ code; // scancode or MOUSE_ or UNIFIED_ value
	bool negated; // whether the key must be pressed (false) or unpressed (true)

	bool operator<(const SKey& o) const { return code < o.code && negated < o.negated; }
	bool operator==(const SKey& o) const { return code == o.code && negated == o.negated; }
};

// Hotkey data associated with an externally-specified 'primary' keycode
struct SHotkeyMapping
{
	CStr name; // name of the hotkey
	bool negated; // whether the primary key must be pressed (false) or unpressed (true)
	std::vector<SKey> requires; // list of non-primary keys that must also be active
};

typedef std::vector<SHotkeyMapping> KeyMapping;

// A mapping of scancodes onto the hotkeys that are associated with that key.
// (A hotkey triggered by a combination of multiple keys will be in this map
// multiple times.)
extern std::unordered_map<SDL_Scancode_, KeyMapping> g_HotkeyMap;

// The current pressed status of hotkeys
extern std::unordered_map<std::string, bool> g_HotkeyStatus;

extern void LoadHotkeys();
extern void UnloadHotkeys();

extern InReaction HotkeyStateChange(const SDL_Event_* ev);
extern InReaction HotkeyInputHandler(const SDL_Event_* ev);

extern bool HotkeyIsPressed(const CStr& keyname);

#endif // INCLUDED_HOTKEY
