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

#ifndef PIDGIN_PREFS_INTERNAL_H
#define PIDGIN_PREFS_INTERNAL_H

#include <purple.h>

typedef struct _PidginPrefCombo PidginPrefCombo;

struct _PidginPrefCombo {
	GtkWidget *combo;
	PurplePrefType type;
	const gchar *key;
};

enum {
	PIDGIN_PREF_COMBO_TEXT,
	PIDGIN_PREF_COMBO_VALUE,
	PIDGIN_PREF_COMBO_N_COLUMNS
};

G_BEGIN_DECLS

G_GNUC_INTERNAL
void pidgin_prefs_bind_checkbox(const char *key, GtkWidget *button);

G_GNUC_INTERNAL
void pidgin_prefs_bind_combo_row(const gchar *key, GtkWidget *widget);

G_GNUC_INTERNAL
void pidgin_prefs_bind_dropdown(PidginPrefCombo *combo);

G_GNUC_INTERNAL
void pidgin_prefs_bind_entry(const char *key, GtkWidget *entry);

G_GNUC_INTERNAL
void pidgin_prefs_bind_expander_row(const gchar *key, GtkWidget *widget);

G_GNUC_INTERNAL
void pidgin_prefs_bind_spin_button(const char *key, GtkWidget *spin);

G_GNUC_INTERNAL
void pidgin_prefs_bind_switch(const gchar *key, GtkWidget *widget);

G_END_DECLS

#endif /* PIDGIN_PREFS_INTERNAL_H */
