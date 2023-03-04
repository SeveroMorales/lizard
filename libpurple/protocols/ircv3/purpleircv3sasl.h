/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifndef PURPLE_IRCV3_SASL_H
#define PURPLE_IRCV3_SASL_H

#include <glib.h>

#include <purple.h>

#include "purpleircv3capabilities.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL void purple_ircv3_sasl_request(PurpleIRCv3Capabilities *capabilities);

G_GNUC_INTERNAL gboolean purple_ircv3_sasl_logged_in(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_logged_out(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_nick_locked(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_success(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_failed(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_message_too_long(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_aborted(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_already_authed(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_mechanisms(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_sasl_authenticate(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);

G_END_DECLS

#endif /* PURPLE_IRCV3_SASL_H */
