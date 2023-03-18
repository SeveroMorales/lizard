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

#ifndef PIDGIN_APPLICATION_H
#define PIDGIN_APPLICATION_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * PidginApplication:
 *
 * #PidginApplication is a subclass of #GtkApplication that holds all of the
 * application wide actions.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_APPLICATION (pidgin_application_get_type())
G_DECLARE_FINAL_TYPE(PidginApplication, pidgin_application, PIDGIN,
                     APPLICATION, GtkApplication)

/**
 * pidgin_application_new:
 *
 * Creates a new #PidginApplication instance.
 *
 * Returns: (transfer full): The new #PidginApplication instance.
 *
 * Since: 3.0.0
 */
GApplication *pidgin_application_new(void);

/**
 * pidgin_application_add_action_group:
 * @application: The [class@Application] instance.
 * @prefix: The action prefix.
 * @action_group: The [iface@Gio.ActionGroup] to add.
 *
 * Adds @action_group to all of the windows that @application knows about and
 * will automatically add @action_group to any newly created application
 * windows.
 *
 * To remove @action_group from every window, call this function with the same
 * prefix, but %NULL for @action_group.
 *
 * Since: 3.0.0
 */
void pidgin_application_add_action_group(PidginApplication *application, const gchar *prefix, GActionGroup *action_group);

/**
 * pidgin_application_get_active_window:
 * @application: The instance.
 *
 * Calls [method@Gtk.Application.get_active_window] to get the active window.
 * If that returns %NULL, fallback to the first window of
 * [method@Gtk.Application.get_windows], and if that fails, return %NULL.
 *
 * Returns: (transfer none) (nullable): The active window or %NULL if one could
 *          not be found.
 *
 * Since: 3.0.0
 */
GtkWindow *pidgin_application_get_active_window(PidginApplication *application);

G_END_DECLS

#endif /* PIDGIN_APPLICATION_H */
