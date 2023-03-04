/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include "talkatu/talkatucodeset.h"

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_codeset_coerce_utf8:
 * @data: The raw data to coerce.
 * @length: The length of the raw data in bytes.
 * @text_length: A return address for the length of the coerced text.
 * @error: A return address for a #GError if anything goes wrong.
 *
 * Attempts to coerce the raw @data into UTF-8.
 *
 * Currently it handles UTF-8, UTF-16 (host encoding), UTF-16 littled endian,
 * and UTF-16 big endian.
 *
 * Returns: (transfer full): The coerced @data as UTF-8, or #NULL with @error
 *          set on error.
 */
gchar *
talkatu_codeset_coerce_utf8(const guint8 *data, gsize length, gsize *text_length, GError **error) {
	gchar *text = NULL;
	gsize real_length = length;

	if(g_utf8_validate((gchar *)data, -1, NULL)) {
		text = g_strdup((gchar *)data);
	} else {
		const gchar *from_codeset = "UTF-16";

		if(length > 2) {
			if(data[0] == 0xFF && data[1] == 0xFE) {
				/* handle little endian byte order mark */
				from_codeset = "UTF-16LE";
				data += 2;
				length -= 2;
			} else if(data[0] == 0xFE && data[1] == 0xFF) {
				/* handle big endian byte order mark */
				from_codeset = "UTF-16BE";
				data += 2;
				length -= 2;
			}
		}

		text = g_convert((gchar *)data, length, "UTF-8", from_codeset, NULL, &real_length, error);
		if(error && *error) {
			return FALSE;
		}
	}

	if(text_length) {
		if(real_length == -1) {
			real_length = g_utf8_strlen(text, -1);
		}

		*text_length = real_length;
	}

	return text;
}
