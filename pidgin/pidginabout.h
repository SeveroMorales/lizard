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

#ifndef PIDGIN_ABOUT_H
#define PIDGIN_ABOUT_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_ABOUT_DIALOG (pidgin_about_dialog_get_type())
G_DECLARE_FINAL_TYPE(PidginAboutDialog, pidgin_about_dialog, PIDGIN,
                     ABOUT_DIALOG, GtkDialog)

/**
 * pidgin_about_dialog_new:
 *
 * Creates a new about window.
 *
 * Returns: (transfer full): A new instance of the about dialog.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_about_dialog_new(void);

G_END_DECLS

#endif /* PIDGIN_ABOUT_H */

