/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_GDK_PIXBUF_H
#define PURPLE_GDK_PIXBUF_H

#include <glib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libpurple/image.h>

G_BEGIN_DECLS

/**
 * purple_gdk_pixbuf_make_round:
 * @pixbuf:  The buddy icon to transform
 *
 * Rounds the corners of a GdkPixbuf in place.
 */
void purple_gdk_pixbuf_make_round(GdkPixbuf *pixbuf);

/**
 * purple_gdk_pixbuf_is_opaque:
 * @pixbuf:  The pixbuf
 *
 * Returns TRUE if the GdkPixbuf is opaque, as determined by no
 * alpha at any of the edge pixels.
 *
 * Returns: TRUE if the pixbuf is opaque around the edges, FALSE otherwise
 */
gboolean purple_gdk_pixbuf_is_opaque(GdkPixbuf *pixbuf);

/**
 * purple_gdk_pixbuf_from_data:
 * @buf: The raw binary image data.
 * @count: The length of buf in bytes.
 *
 * Create a GdkPixbuf from a chunk of image data.
 *
 * Returns: (transfer full): A GdkPixbuf created from the image data, or NULL if
 *         there was an error parsing the data.
 */
GdkPixbuf *purple_gdk_pixbuf_from_data(const guchar *buf, gsize count);

/**
 * purple_gdk_pixbuf_from_image:
 * @image: a PurpleImage.
 *
 * Create a GdkPixbuf from a PurpleImage.
 *
 * Returns: (transfer full): a GdkPixbuf created from the @image.
 */
GdkPixbuf *
purple_gdk_pixbuf_from_image(PurpleImage *image);

/**
 * purple_gdk_pixbuf_new_from_file:
 * @filename: Name of file to load, in the GLib file name encoding
 *
 * Helper function that calls gdk_pixbuf_new_from_file() and checks both
 * the return code and the GError and returns NULL if either one failed.
 *
 * The gdk-pixbuf documentation implies that it is sufficient to check
 * the return value of gdk_pixbuf_new_from_file() to determine
 * whether the image was able to be loaded.  However, this is not the case
 * with gdk-pixbuf 2.23.3 and probably many earlier versions.  In some
 * cases a GdkPixbuf object is returned that will cause some operations
 * (like gdk_pixbuf_scale_simple()) to rapidly consume memory in an
 * infinite loop.
 *
 * This function shouldn't be necessary once Pidgin requires a version of
 * gdk-pixbuf where the aforementioned bug is fixed.  However, it might be
 * nice to keep this function around for the debug message that it logs.
 *
 * Returns: (transfer full): The GdkPixbuf if successful.  Otherwise NULL is returned and
 *         a warning is logged.
 */
GdkPixbuf *purple_gdk_pixbuf_new_from_file(const char *filename);

/**
 * purple_gdk_pixbuf_new_from_file_at_size:
 * @filename: Name of file to load, in the GLib file name encoding
 * @width: The width the image should have or -1 to not constrain the width
 * @height: The height the image should have or -1 to not constrain the height
 *
 * Helper function that calls gdk_pixbuf_new_from_file_at_size() and checks
 * both the return code and the GError and returns NULL if either one failed.
 *
 * The gdk-pixbuf documentation implies that it is sufficient to check
 * the return value of gdk_pixbuf_new_from_file_at_size() to determine
 * whether the image was able to be loaded.  However, this is not the case
 * with gdk-pixbuf 2.23.3 and probably many earlier versions.  In some
 * cases a GdkPixbuf object is returned that will cause some operations
 * (like gdk_pixbuf_scale_simple()) to rapidly consume memory in an
 * infinite loop.
 *
 * This function shouldn't be necessary once Pidgin requires a version of
 * gdk-pixbuf where the aforementioned bug is fixed.  However, it might be
 * nice to keep this function around for the debug message that it logs.
 *
 * Returns: (transfer full): The GdkPixbuf if successful.  Otherwise NULL is returned and
 *         a warning is logged.
 */
GdkPixbuf *purple_gdk_pixbuf_new_from_file_at_size(const char *filename, int width, int height);

G_END_DECLS

#endif /* PURPLE_GDK_PIXBUF_H */
