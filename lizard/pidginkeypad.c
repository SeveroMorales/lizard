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

#include <glib/gi18n-lib.h>

#include "pidginkeypad.h"

struct _PidginKeypad {
	GtkGrid parent;
};

enum {
	SIG_PRESSED,
	N_SIGNALS
};
static guint signals[N_SIGNALS] = {0, };

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_keypad_action_activate(G_GNUC_UNUSED GSimpleAction *action,
                              GVariant *parameter,
                              gpointer data)
{
	PidginKeypad *keypad = PIDGIN_KEYPAD(data);
	gint32 num = 0;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT32)) {
		g_critical("PidginKeypad dtmf action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	num = g_variant_get_int32(parameter);

	g_signal_emit(keypad, signals[SIG_PRESSED], 0, num);
}

static gboolean
pidgin_keypad_key_pressed_cb(G_GNUC_UNUSED GtkEventControllerKey *controller,
                             guint keyval,
                             G_GNUC_UNUSED guint keycode,
                             G_GNUC_UNUSED GdkModifierType state,
                             gpointer data)
{
	PidginKeypad *keypad = PIDGIN_KEYPAD(data);

	if(GDK_KEY_KP_0 <= keyval && keyval <= GDK_KEY_KP_9) {
		/* Normalize to the same ASCII numbers. */
		keyval = (keyval - GDK_KEY_KP_0) + GDK_KEY_0;
	}

	if((GDK_KEY_0 <= keyval && keyval <= GDK_KEY_9) ||
		keyval == GDK_KEY_asterisk ||
		keyval == GDK_KEY_numbersign) {

		g_signal_emit(keypad, signals[SIG_PRESSED], 0, keyval);
		return TRUE;
	}

	return FALSE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginKeypad, pidgin_keypad, GTK_TYPE_GRID)

static void
pidgin_keypad_init(PidginKeypad *keypad) {
	GtkWidget *widget = GTK_WIDGET(keypad);
	GSimpleActionGroup *action_group = NULL;
	GActionEntry actions[] = {
		{
			.name = "dtmf",
			.activate = pidgin_keypad_action_activate,
			.parameter_type = "i",
		},
	};

	gtk_widget_init_template(widget);

	action_group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(action_group),
	                                actions, G_N_ELEMENTS(actions), keypad);
	gtk_widget_insert_action_group(widget, "keypad",
	                               G_ACTION_GROUP(action_group));
}

static void
pidgin_keypad_class_init(PidginKeypadClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	/**
	 * PidginKeypad::pressed:
	 * @keypad: The #PidginKeypad.
	 * @key: The key that was pressed, [const@Gdk.KEY_0] - [const@Gdk.KEY_9],
	 *       [const@Gdk.KEY_asterisk], or [const@Gdk.KEY_numbersign].
	 *
	 * Emitted when a key is pressed (by keyboard or from clicking a button).
	 */
	signals[SIG_PRESSED] = g_signal_new("pressed",
	                                    G_TYPE_FROM_CLASS(klass),
	                                    0, 0, NULL, NULL, NULL, G_TYPE_NONE,
	                                    1, G_TYPE_UINT);

	gtk_widget_class_set_template_from_resource(widget_class,
	                                            "/im/pidgin/Pidgin3/Keypad/keypad.ui");
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_keypad_new(void) {
	return g_object_new(PIDGIN_TYPE_KEYPAD, NULL);
}

void
pidgin_keypad_set_key_capture_widget(PidginKeypad *keypad, GtkWidget *widget) {
	GtkEventController *controller = NULL;

	g_return_if_fail(PIDGIN_IS_KEYPAD(keypad));
	g_return_if_fail(GTK_IS_WIDGET(widget));

	controller = gtk_event_controller_key_new();
	gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
	g_signal_connect(controller, "key-pressed",
	                 G_CALLBACK(pidgin_keypad_key_pressed_cb), keypad);
	gtk_widget_add_controller(widget, controller);
}
