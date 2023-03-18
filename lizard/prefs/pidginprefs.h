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

#ifndef PIDGIN_PREFS_H
#define PIDGIN_PREFS_H

#include <glib.h>
#include <purple.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_PREFS_WINDOW (pidgin_prefs_window_get_type())
G_DECLARE_FINAL_TYPE(PidginPrefsWindow, pidgin_prefs_window, PIDGIN,
                     PREFS_WINDOW, GtkDialog)

/**
 * pidgin_prefs_init:
 *
 * Initializes all UI-specific preferences.
 */
void pidgin_prefs_init(void);

/**
 * pidgin_prefs_checkbox:
 * @title: The text to be displayed as the checkbox label
 * @key:   The key of the purple bool pref that will be represented by the checkbox
 * @page:  The page to which the new checkbox will be added
 *
 * Add a new checkbox for a boolean preference
 *
 * Returns: (transfer full): The new checkbox
 */
GtkWidget *pidgin_prefs_checkbox(const char *title, const char *key,
		GtkWidget *page);

/**
 * pidgin_prefs_labeled_spin_button:
 * @page:  The page to which the spin button will be added
 * @title: The text to be displayed as the spin button label
 * @key:   The key of the int pref that will be represented by the spin button
 * @min:   The minimum value of the spin button
 * @max:   The maximum value of the spin button
 * @sg:    If not NULL, the size group to which the spin button will be added
 *
 * Add a new spin button representing an int preference
 *
 * Returns: (transfer full): An hbox containing both the label and the spinner.  Can be
 *          used to set the widgets to sensitive or insensitive based on the
 *          value of a checkbox.
 */
GtkWidget *pidgin_prefs_labeled_spin_button(GtkWidget *page,
		const gchar *title, const char *key, int min, int max, GtkSizeGroup *sg);

/**
 * pidgin_prefs_dropdown:
 * @page:  The page to which the dropdown will be added
 * @title: The text to be displayed as the dropdown label
 * @type:  The type of preference to be stored in the generated dropdown
 * @key:   The key of the pref that will be represented by the dropdown
 * @...:   The choices to be added to the dropdown, choices should be
 *              paired as label/value
 *
 * Add a new dropdown representing a preference of the specified type
 *
 * Returns: (transfer full): The new dropdown. 
 */
GtkWidget *pidgin_prefs_dropdown(GtkWidget *page, const gchar *title,
		PurplePrefType type, const char *key, ...);

/**
 * pidgin_prefs_dropdown_from_list:
 * @page:      The page to which the dropdown will be added
 * @title:     The text to be displayed as the dropdown label
 * @type:      The type of preference to be stored in the dropdown
 * @key:       The key of the pref that will be represented by the dropdown
 * @menuitems: (element-type PurpleKeyValuePair): The choices to be added to the dropdown, choices should
 *             be paired as label/value
 *
 * Add a new dropdown representing a preference of the specified type
 *
 * Returns: (transfer full): The new dropdown. 
 */
GtkWidget *pidgin_prefs_dropdown_from_list(GtkWidget *page,
		const gchar * title, PurplePrefType type, const char *key,
		GList *menuitems);

/**
 * pidgin_prefs_update_old:
 *
 * Rename legacy prefs and delete some that no longer exist.
 */
void pidgin_prefs_update_old(void);

G_END_DECLS

#endif /* PIDGIN_PREFS_H */
