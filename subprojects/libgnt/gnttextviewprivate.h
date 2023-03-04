/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_TEXT_VIEW_PRIVATE_H
#define GNT_TEXT_VIEW_PRIVATE_H

#include "gnttextview.h"

G_BEGIN_DECLS
/* Private access to some internals. Contact us if you need these. */

/*
 * gnt_text_format_flag_to_chtype:
 * @flags: The GNT text format.
 *
 * Convert GNT-text formats to ncurses-text attributes.
 *
 * Returns: Ncurses text attribute.
 */
G_GNUC_INTERNAL
chtype gnt_text_format_flag_to_chtype(GntTextFormatFlags flags);

G_END_DECLS

#endif /* GNT_TEXT_VIEW_PRIVATE_H */
