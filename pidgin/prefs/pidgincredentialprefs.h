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

#ifndef PIDGIN_CREDENTIAL_PREFS_H
#define PIDGIN_CREDENTIAL_PREFS_H

#include <glib.h>

#include <gtk/gtk.h>
#include <adwaita.h>

G_BEGIN_DECLS

/**
 * PIDGIN_TYPE_CREDENTIAL_PREFS:
 *
 * The standard _get_type macro for #PidginCredentialPrefs.
 *
 * Since: 3.0.0
 */
#define PIDGIN_TYPE_CREDENTIAL_PREFS (pidgin_credential_prefs_get_type())
G_DECLARE_FINAL_TYPE(PidginCredentialPrefs, pidgin_credential_prefs,
                     PIDGIN, CREDENTIAL_PREFS, AdwPreferencesPage)

/**
 * PidginCredentialPrefs:
 *
 * #PidginCredentialPrefs is a widget for the preferences window to let users
 * choose and configure their credential provider.
 *
 * Since: 3.0.0
 */

/**
 * pidgin_credential_prefs_new:
 *
 * Creates a new #PidginCredentialPrefs instance.
 *
 * Returns: (transfer full): The new #PidginCredentialPrefs instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_credential_prefs_new(void);

G_END_DECLS

#endif /* PIDGIN_CREDENTIAL_PREFS_H */
