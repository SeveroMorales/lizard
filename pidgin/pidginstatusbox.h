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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_STATUS_BOX_H
#define PIDGIN_STATUS_BOX_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_STATUS_BOX (pidgin_status_box_get_type ())
G_DECLARE_FINAL_TYPE(PidginStatusBox, pidgin_status_box, PIDGIN, STATUS_BOX,
                     GtkBox)

GtkWidget *pidgin_status_box_new(void);

G_END_DECLS

#endif /* PIDGIN_STATUS_BOX_H */
