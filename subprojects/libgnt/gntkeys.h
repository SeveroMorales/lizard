/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_KEYS_H
#define GNT_KEYS_H

#include <term.h>

/*
 * terminfo/termcap doesn't provide all the information that I want to use, eg.
 * ctrl-up, ctrl-down etc. So I am going to hard-code some of the information
 * for some popular $TERMs
 */
extern char *gnt_key_cup;
extern char *gnt_key_cdown;
extern char *gnt_key_cleft;
extern char *gnt_key_cright;

/**
 * GNT_KEY_SAFE: (skip)
 * @x: A character.
 *
 * If @x is printable this evaluates to @x, otherwise "".
 */
#define GNT_KEY_SAFE(x)   ((cur_term && (x)) ? (x) : "")

/**
 * GNT_KEY_CONSTCH: (skip)
 * @x: A keycode string.
 *
 * This is needed so that g-ir-scanner does not take GNT_KEY_CTRL_* as
 * constants -- because if it does, the .gir contains the following invalid XML
 * characters, and parsing explodes.
 */
#define GNT_KEY_CONSTCH(x) x

#ifdef _WIN32

/* XXX: \xe1 is a hacky alias for \x00 key code */

#define GNT_KEY_POPUP GNT_KEY_CONSTCH("") /* not supported? */

#define GNT_KEY_UP GNT_KEY_CONSTCH("\033\xe0\x48")
#define GNT_KEY_DOWN GNT_KEY_CONSTCH("\033\xe0\x50")
#define GNT_KEY_LEFT GNT_KEY_CONSTCH("\033\xe0\x4B")
#define GNT_KEY_RIGHT GNT_KEY_CONSTCH("\033\xe0\x4D")

#define GNT_KEY_CTRL_UP GNT_KEY_CONSTCH("\033\xe0\x8d")
#define GNT_KEY_CTRL_DOWN GNT_KEY_CONSTCH("\033\xe0\x91")
#define GNT_KEY_CTRL_LEFT GNT_KEY_CONSTCH("\033\xe0\x73")
#define GNT_KEY_CTRL_RIGHT GNT_KEY_CONSTCH("\033\xe0\x74")

#define GNT_KEY_PGUP GNT_KEY_CONSTCH("\033\xe0\x49")
#define GNT_KEY_PGDOWN GNT_KEY_CONSTCH("\033\xe0\x51")
#define GNT_KEY_HOME GNT_KEY_CONSTCH("\033\xe0\x47")
#define GNT_KEY_END GNT_KEY_CONSTCH("\033\xe0\x4f")

#define GNT_KEY_ENTER GNT_KEY_CONSTCH("\x0d")

#define GNT_KEY_BACKSPACE GNT_KEY_CONSTCH("\x08")
#define GNT_KEY_DEL GNT_KEY_CONSTCH("\033\xe0\x53")
#define GNT_KEY_INS GNT_KEY_CONSTCH("\033\xe0\x52")
#define GNT_KEY_BACK_TAB GNT_KEY_CONSTCH("\033\xe1\x94")

#define GNT_KEY_F1 GNT_KEY_CONSTCH("\033\xe1\x3b")
#define GNT_KEY_F2 GNT_KEY_CONSTCH("\033\xe1\x3c")
#define GNT_KEY_F3 GNT_KEY_CONSTCH("\033\xe1\x3d")
#define GNT_KEY_F4 GNT_KEY_CONSTCH("\033\xe1\x3e")
#define GNT_KEY_F5 GNT_KEY_CONSTCH("\033\xe1\x3f")
#define GNT_KEY_F6 GNT_KEY_CONSTCH("\033\xe1\x40")
#define GNT_KEY_F7 GNT_KEY_CONSTCH("\033\xe1\x41")
#define GNT_KEY_F8 GNT_KEY_CONSTCH("\033\xe1\x42")
#define GNT_KEY_F9 GNT_KEY_CONSTCH("\033\xe1\x43")
#define GNT_KEY_F10 GNT_KEY_CONSTCH("\033\xe1\x44")
#define GNT_KEY_F11 GNT_KEY_CONSTCH("\033\xe0\x85")
#define GNT_KEY_F12 GNT_KEY_CONSTCH("\033\xe0\x86")

#else

#define GNT_KEY_POPUP   GNT_KEY_SAFE(key_f16)   /* Apparently */

/* Arrow keys */
#define GNT_KEY_LEFT   GNT_KEY_SAFE(key_left)
#define GNT_KEY_RIGHT  GNT_KEY_SAFE(key_right)
#define GNT_KEY_UP     GNT_KEY_SAFE(key_up)
#define GNT_KEY_DOWN   GNT_KEY_SAFE(key_down)

#define GNT_KEY_CTRL_UP     GNT_KEY_SAFE(gnt_key_cup)
#define GNT_KEY_CTRL_DOWN   GNT_KEY_SAFE(gnt_key_cdown)
#define GNT_KEY_CTRL_RIGHT  GNT_KEY_SAFE(gnt_key_cright)
#define GNT_KEY_CTRL_LEFT   GNT_KEY_SAFE(gnt_key_cleft)

#define GNT_KEY_PGUP   GNT_KEY_SAFE(key_ppage)
#define GNT_KEY_PGDOWN GNT_KEY_SAFE(key_npage)
#define GNT_KEY_HOME   GNT_KEY_SAFE(key_home)
#define GNT_KEY_END    GNT_KEY_SAFE(key_end)

#define GNT_KEY_ENTER  GNT_KEY_SAFE(carriage_return)

#define GNT_KEY_BACKSPACE GNT_KEY_SAFE(key_backspace)
#define GNT_KEY_DEL    GNT_KEY_SAFE(key_dc)
#define GNT_KEY_INS    GNT_KEY_SAFE(key_ic)
#define GNT_KEY_BACK_TAB ((cur_term && back_tab) ? back_tab : GNT_KEY_SAFE(key_btab))

#define GNT_KEY_F1         GNT_KEY_SAFE(key_f1)
#define GNT_KEY_F2         GNT_KEY_SAFE(key_f2)
#define GNT_KEY_F3         GNT_KEY_SAFE(key_f3)
#define GNT_KEY_F4         GNT_KEY_SAFE(key_f4)
#define GNT_KEY_F5         GNT_KEY_SAFE(key_f5)
#define GNT_KEY_F6         GNT_KEY_SAFE(key_f6)
#define GNT_KEY_F7         GNT_KEY_SAFE(key_f7)
#define GNT_KEY_F8         GNT_KEY_SAFE(key_f8)
#define GNT_KEY_F9         GNT_KEY_SAFE(key_f9)
#define GNT_KEY_F10        GNT_KEY_SAFE(key_f10)
#define GNT_KEY_F11        GNT_KEY_SAFE(key_f11)
#define GNT_KEY_F12        GNT_KEY_SAFE(key_f12)

#endif

#define GNT_KEY_CTRL_A     GNT_KEY_CONSTCH("\001")
#define GNT_KEY_CTRL_B     GNT_KEY_CONSTCH("\002")
#define GNT_KEY_CTRL_D     GNT_KEY_CONSTCH("\004")
#define GNT_KEY_CTRL_E     GNT_KEY_CONSTCH("\005")
#define GNT_KEY_CTRL_F     GNT_KEY_CONSTCH("\006")
#define GNT_KEY_CTRL_G     GNT_KEY_CONSTCH("\007")
#define GNT_KEY_CTRL_H     GNT_KEY_CONSTCH("\010")
#define GNT_KEY_CTRL_I     GNT_KEY_CONSTCH("\011")
#define GNT_KEY_CTRL_J     GNT_KEY_CONSTCH("\012")
#define GNT_KEY_CTRL_K     GNT_KEY_CONSTCH("\013")
#define GNT_KEY_CTRL_L     GNT_KEY_CONSTCH("\014")
#define GNT_KEY_CTRL_M     GNT_KEY_CONSTCH("\012")
#define GNT_KEY_CTRL_N     GNT_KEY_CONSTCH("\016")
#define GNT_KEY_CTRL_O     GNT_KEY_CONSTCH("\017")
#define GNT_KEY_CTRL_P     GNT_KEY_CONSTCH("\020")
#define GNT_KEY_CTRL_R     GNT_KEY_CONSTCH("\022")
#define GNT_KEY_CTRL_T     GNT_KEY_CONSTCH("\024")
#define GNT_KEY_CTRL_U     GNT_KEY_CONSTCH("\025")
#define GNT_KEY_CTRL_V     GNT_KEY_CONSTCH("\026")
#define GNT_KEY_CTRL_W     GNT_KEY_CONSTCH("\027")
#define GNT_KEY_CTRL_X     GNT_KEY_CONSTCH("\030")
#define GNT_KEY_CTRL_Y     GNT_KEY_CONSTCH("\031")

/**
 * gnt_init_keys:
 *
 * Initialize the keys.
 */
void gnt_init_keys(void);

/**
 * gnt_keys_refine:
 * @text:  The input text to refine.
 *
 * Refine input text. This usually looks at what the terminal claims it is,
 * and tries to change the text to work around some oft-broken terminfo entries.
 */
void gnt_keys_refine(char *text);

/**
 * gnt_key_translate:
 * @name:   The user-readable representation of an input (eg.: c-t)
 *
 * Translate a user-readable representation of an input to a machine-readable representation.
 *
 * Returns:  A machine-readable representation of the input.
 */
const char *gnt_key_translate(const char *name);

/**
 * gnt_key_lookup:
 * @key:  The machine-readable representation of an input.
 *
 * Translate a machine-readable representation of an input to a user-readable representation.
 *
 * Returns:  A user-readable representation of the input (eg.: c-t).
 */
const char *gnt_key_lookup(const char *key);

/**
 * gnt_keys_add_combination:
 * @key:  The key to add
 *
 * Add a key combination to the internal key-tree.
 */
void gnt_keys_add_combination(const char *key);

/**
 * gnt_keys_del_combination:
 * @key: The key to remove.
 *
 * Remove a key combination from the internal key-tree.
 */
void gnt_keys_del_combination(const char *key);

/**
 * gnt_keys_find_combination:
 * @key:  The input string.
 *
 * Find a combination from the given string.
 *
 * Returns: The number of bytes in the combination that starts at the beginning
 *         of key (can be 0).
 */
int gnt_keys_find_combination(const char *key);

/* A lot of commonly used variable names are defined in <term.h>.
 * #undef them to make life easier for everyone. */

#undef columns
#undef lines
#undef buttons
#undef newline
#undef set_clock

#endif
