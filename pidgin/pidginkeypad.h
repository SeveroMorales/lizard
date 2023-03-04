/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_KEYPAD_H
#define PIDGIN_KEYPAD_H

#include <gtk/gtk.h>

#include <purple.h>

/**
 * PidginKeypad:
 *
 * #PidginKeypad is a widget that displays a DTMF keypad, with the digits 0-9,
 * an asterisk, and a number sign.
 *
 * Since: 3.0.0
 */

G_BEGIN_DECLS

#define PIDGIN_TYPE_KEYPAD pidgin_keypad_get_type()
G_DECLARE_FINAL_TYPE(PidginKeypad, pidgin_keypad, PIDGIN, KEYPAD, GtkGrid)

/**
 * pidgin_keypad_new:
 *
 * Creates a new #PidginKeypad.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_keypad_new(void);

/**
 * pidgin_keypad_set_key_capture_widget:
 * @keypad: The keypad.
 * @widget: A widget to capture keys from.
 *
 * Sets @widget as the widget that @keypad will capture key events from.
 *
 * If key events are handled by the keypad, the DTMF digits will be captured
 * and trigger the pressed signal on @keypad.
 *
 * Since: 3.0.0
 */
void pidgin_keypad_set_key_capture_widget(PidginKeypad *keypad, GtkWidget *widget);

G_END_DECLS

#endif /* PIDGIN_KEYPAD_H */
