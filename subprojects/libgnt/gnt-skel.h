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

#ifndef GNT_SKEL_H
#define GNT_SKEL_H

#include "gntwidget.h"
#include "gntcolors.h"
#include "gntkeys.h"

#define GNT_TYPE_SKEL gnt_skel_get_type()

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GntSkel, gnt_skel, GNT, SKEL, GntWidget)

GntWidget * gnt_skel_new();

G_END_DECLS

#endif /* GNT_SKEL_H */
