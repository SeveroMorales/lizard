/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2023 Gary Kramlich <grim@reaperworld.com>
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

#if !defined(TALKATU_GLOBAL_HEADER_INSIDE) && !defined(TALKATU_COMPILATION)
#error "only <talkatu.h> may be included directly"
#endif

#ifndef TALKATU_FORMAT_H
#define TALKATU_FORMAT_H

#include <glib.h>

typedef enum {
	TALKATU_FORMAT_NONE = 0,
	TALKATU_FORMAT_BOLD = 1 << 0,
	TALKATU_FORMAT_ITALIC = 1 << 1,
	TALKATU_FORMAT_UNDERLINE = 1 << 2,
	TALKATU_FORMAT_STRIKETHROUGH = 1 << 3,
	TALKATU_FORMAT_GROW = 1 << 4,
	TALKATU_FORMAT_SHRINK = 1 << 5,
	TALKATU_FORMAT_RESET = 1 << 6,
	TALKATU_FORMAT_ATTACH_FILE = 1 << 7,
	TALKATU_FORMAT_ATTACH_IMAGE = 1 << 8,
	TALKATU_FORMAT_INSERT_LINK = 1 << 9,
	TALKATU_FORMAT_INSERT_CODE = 1 << 10,
	/* Helper presets. */
	TALKATU_FORMAT_HTML = TALKATU_FORMAT_BOLD |
	                      TALKATU_FORMAT_ITALIC |
	                      TALKATU_FORMAT_UNDERLINE |
	                      TALKATU_FORMAT_STRIKETHROUGH |
	                      TALKATU_FORMAT_GROW |
	                      TALKATU_FORMAT_SHRINK |
	                      TALKATU_FORMAT_RESET |
	                      TALKATU_FORMAT_ATTACH_FILE |
	                      TALKATU_FORMAT_ATTACH_IMAGE |
	                      TALKATU_FORMAT_INSERT_LINK |
	                      /* g-ir-scanner has issues with the last element
	                       * on a line by itself, so we trick it by or'n the
	                       * last item with a 0.
	                       *
	                       * see https://gitlab.gnome.org/GNOME/gobject-introspection/-/issues/454
	                       */
	                      TALKATU_FORMAT_INSERT_CODE | 0,
	TALKATU_FORMAT_MARKDOWN = TALKATU_FORMAT_BOLD |
	                          TALKATU_FORMAT_ITALIC |
	                          TALKATU_FORMAT_UNDERLINE |
	                          TALKATU_FORMAT_STRIKETHROUGH |
	                          TALKATU_FORMAT_RESET |
	                          TALKATU_FORMAT_ATTACH_FILE |
	                          TALKATU_FORMAT_ATTACH_IMAGE |
	                          TALKATU_FORMAT_INSERT_LINK |
	                          /* g-ir-scanner has issues with the last element
	                           * on a line by itself, so we trick it by or'n
	                           * the last item with a 0.
	                           *
	                           * see https://gitlab.gnome.org/GNOME/gobject-introspection/-/issues/454
	                           */
	                          TALKATU_FORMAT_INSERT_CODE | 0,
} TalkatuFormat;

G_BEGIN_DECLS


G_END_DECLS

#endif /* TALKATU_FORMAT_H */
