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

#ifndef PURPLE_IRCV3_MESSAGE_HANDLERS_H
#define PURPLE_IRCV3_MESSAGE_HANDLERS_H

#include <glib.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PurpleIRCv3MessageHandler:
 * @tags: (element-type utf8 utf8): A [struct@GLib.HashTable] of tags for this
 *        message.
 * @source: The source of the command.
 * @command: The command or numeric of the message.
 * @n_params: The number of parameters.
 * @params: (array_length=n_params): The parameters of the message.
 * @error: (nullable) (optional): A return address for a [type@GLib.Error].
 * @data: The user data that was passed to [method@PurpleIRCv3.Parser.parse].
 *
 * Defines the type of a function that will be called to handle a message.
 *
 * Returns: %TRUE if the command was handled properly, otherwise %FALSE and
 *          @error may be set.
 *
 * Since: 3.0.0
 */
typedef gboolean (*PurpleIRCv3MessageHandler)(GHashTable *tags,
                                              const char *source,
                                              const char *command,
                                              guint n_params,
                                              GStrv params,
                                              GError **error,
                                              gpointer data);

G_GNUC_INTERNAL gboolean purple_ircv3_message_handler_fallback(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_message_handler_ping(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);
G_GNUC_INTERNAL gboolean purple_ircv3_message_handler_privmsg(GHashTable *tags, const char *source, const char *command, guint n_params, GStrv params, GError **error, gpointer data);

G_END_DECLS

#endif /* PURPLE_IRCV3_MESSAGE_HANDLERS_H */
