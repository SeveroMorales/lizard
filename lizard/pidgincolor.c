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

#include "pidgincolor.h"

/******************************************************************************
 * Public API
 *****************************************************************************/
void
pidgin_color_calculate_for_text(const gchar *text, GdkRGBA *color) {
    GdkRGBA background;
    GChecksum *checksum = NULL;
    guchar digest[20];
    gsize digest_len = sizeof(digest);
    gfloat hue = 0, red = 0, green = 0, blue = 0;

    g_return_if_fail(color != NULL);

#warning figure out how to get the background color
    gdk_rgba_parse(&background, "#FFFFFFFF");

    /* hash the string and get the first 2 bytes of the digest */
    checksum = g_checksum_new(G_CHECKSUM_SHA1);
    if(text != NULL) {
        g_checksum_update(checksum, (const guchar *)text, -1);
    }
    g_checksum_get_digest(checksum, digest, &digest_len);
    g_checksum_free(checksum);

    /* Calculate the hue based on the digest.  We need a value between 0 and 1
     * so we divide the value by 65535 which is the maximum value for 2 bytes.
     */
    hue = (digest[0] << 8 | digest[1]) / 65535.0f;

    /* Get the rgb values for the hue at full saturation and value. */
    gtk_hsv_to_rgb(hue, 1.0f, 1.0f, &red, &green, &blue);

    /* Finally calculate the color summing 20% of the inverted background color
     * with 80% of the color.
     */
    color->red = (0.2f * (1 - background.red)) + (0.8f * red);
    color->green = (0.2f * (1 - background.green)) + (0.8f * green);
    color->blue = (0.2f * (1 - background.blue)) + (0.8f * blue);
    color->alpha = 1.0f;
}
