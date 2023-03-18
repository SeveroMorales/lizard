/*
 * pidgin
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_MOOD_DIALOG_H
#define PIDGIN_MOOD_DIALOG_H

#include <glib.h>

#include <gtk/gtk.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * pidgin_mood_dialog_show:
 * @account: (nullable): The #PurpleAccount whose mood to set, or %NULL for the
 *           global mood.
 *
 * Presents a dialog to select the mood for @account or the global mood if
 * @account is %NULL.
 */
void pidgin_mood_dialog_show(PurpleAccount *account);

/**
 * pidgin_mood_get_icon_path:
 * @mood: The id of the mood.
 *
 * Gets the path to the icon for @mood.
 *
 * Returns: (transfer full): The location of the icon for @mood.
 */
gchar *pidgin_mood_get_icon_path(const gchar *mood);

G_END_DECLS

#endif /* PIDGIN_MOOD_DIALOG_H */

