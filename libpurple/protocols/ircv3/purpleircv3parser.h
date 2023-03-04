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

#ifndef PURPLE_IRCV3_PARSER_H
#define PURPLE_IRCV3_PARSER_H

#include <glib.h>
#include <glib-object.h>

#include <purple.h>

#include "purpleircv3messagehandlers.h"

G_BEGIN_DECLS

#define PURPLE_IRCV3_TYPE_PARSER (purple_ircv3_parser_get_type())
G_DECLARE_FINAL_TYPE(PurpleIRCv3Parser, purple_ircv3_parser, PURPLE_IRCV3,
                     PARSER, GObject)

/**
 * purple_ircv3_parser_new:
 *
 * Creates a new instance.
 *
 * Since: 3.0.0
 */
G_GNUC_INTERNAL PurpleIRCv3Parser *purple_ircv3_parser_new(void);

/**
 * purple_ircv3_parser_set_fallback_handler: (skip):
 * @parser: The instance.
 * @handler: A [func@PurpleIRCv3.MessageHandler].
 *
 * Sets @handler to be called for any messages that @parser doesn't know how to
 * handle.
 *
 * Since: 3.0.0
 */
G_GNUC_INTERNAL void purple_ircv3_parser_set_fallback_handler(PurpleIRCv3Parser *parser, PurpleIRCv3MessageHandler handler);

/**
 * purple_ircv3_parser_parse:
 * @parser: The instance.
 * @buffer: The buffer to parse.
 * @error: Return address for a #GError, or %NULL.
 * @data: (nullable): Optional data to pass to the handler.
 *
 * Parses @buffer with @parser.
 *
 * Returns: %TRUE if the buffer was parsed correctly or %FALSE with @error set.
 *
 * Since: 3.0.0
 */
G_GNUC_INTERNAL gboolean purple_ircv3_parser_parse(PurpleIRCv3Parser *parser, const gchar *buffer, GError **error, gpointer data);

/**
 * purple_ircv3_parser_add_default_handlers:
 * @parser: The instance.
 *
 * Adds all of the default handlers to @parser.
 *
 * Since: 3.0.0
 */
G_GNUC_INTERNAL void purple_ircv3_parser_add_default_handlers(PurpleIRCv3Parser *parser);

G_END_DECLS

#endif /* PURPLE_IRCV3_PARSER_H */
