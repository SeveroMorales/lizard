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
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_COLOR_H
#define PIDGIN_COLOR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * pidgin_color_calculate_for_text:
 * @text: The text to calculate a color for.
 * @color: (out): The return address for a #GdkRGBA that will receive the
 *         color.
 *
 * This function is based heavily on the implementation that gajim uses from
 * python-nbxmpp in nbxmpp.util.text_to_color.  However, we don't have an
 * implementation of HSL let alone HSLuv, so we're using HSV which is why
 * the value is 1.0 instead of a luminance of 0.5.
 *
 * Currently there is no caching as GCache is deprecated and writing a fast LRU
 * in glib is going to take a bit of finesse.  Also we'll need to figure out how
 * to scale to ginormous Twitch channels which will constantly bust the cache.
 *
 * Since: 3.0.0
 */
void pidgin_color_calculate_for_text(const gchar *text,  GdkRGBA *color);

G_END_DECLS

#endif /* PIDGIN_COLOR_H */
