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

#ifndef PIDGIN_PRESENCE_ICON_H
#define PIDGIN_PRESENCE_ICON_H

#include <gtk/gtk.h>

#include <purple.h>

/**
 * PidginPresenceIcon:
 *
 * A #GtkImage subclass that will automatically update when the given presence
 * changes.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_PRESENCE_ICON (pidgin_presence_icon_get_type())
G_DECLARE_FINAL_TYPE(PidginPresenceIcon, pidgin_presence_icon,
                     PIDGIN, PRESENCE_ICON, GtkBox)

G_BEGIN_DECLS

/**
 * pidgin_presence_icon_new:
 * @presence: (nullable): The #PurplePresence to represent.
 * @fallback: The name of the icon to use as a fallback.
 * @icon_size: The #GtkIconSize used to render the icon.
 *
 * Creates a new #PidginPresenceIcon.
 *
 * Returns: (transfer full): The new presence icon.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_presence_icon_new(PurplePresence *presence, const gchar *fallback, GtkIconSize icon_size);

/**
 * pidgin_presence_icon_get_presence:
 * @icon: The #PidginPresenceIcon instance.
 *
 * Gets the #PurplePresence that @icon is displaying.
 *
 * Returns: (transfer none): The #PurplePresence if set otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurplePresence *pidgin_presence_icon_get_presence(PidginPresenceIcon *icon);

/**
 * pidgin_presence_icon_set_presence:
 * @icon: The #PidginPresenceIcon instance.
 * @presence: (nullable): The new #PurplePresence to set, or %NULL to unset.
 *
 * Sets the #PurplePresence for @icon to display.
 *
 * Since: 3.0.0
 */
void pidgin_presence_icon_set_presence(PidginPresenceIcon *icon, PurplePresence *presence);

/**
 * pidgin_presence_icon_get_fallback:
 * @icon: The #PidginPresenceIcon instance.
 *
 * Gets the name of the fallback icon from @icon.
 *
 * Returns: The fallback icon name for @icon.
 *
 * Since: 3.0.0
 */
const gchar *pidgin_presence_icon_get_fallback(PidginPresenceIcon *icon);

/**
 * pidgin_presence_icon_set_fallback:
 * @icon: The #PidginPresenceIcon instance.
 * @fallback: The name of the fallback icon name.
 *
 * Sets the fallback icon name for @icon to @fallback.
 *
 * Since: 3.0.0
 */
void pidgin_presence_icon_set_fallback(PidginPresenceIcon *icon, const gchar *fallback);

/**
 * pidgin_presence_icon_get_icon_size:
 * @icon: The #PidginPresenceIcon instance.
 *
 * Gets the #GtkIconSize that @icon is using to render the icon.
 *
 * Returns: The #GtkIconSize that @icon is using to render.
 *
 * Since: 3.0.0
 */
GtkIconSize pidgin_presence_icon_get_icon_size(PidginPresenceIcon *icon);

/**
 * pidgin_presence_icon_set_icon_size:
 * @icon: The #PidginPresenceIcon instance.
 * @icon_size: The #GtkIconSize to render the icon at.
 *
 * Sets the #GtkIconSize that @icon will use to render the icon.
 *
 * Since: 3.0.0
 */
void pidgin_presence_icon_set_icon_size(PidginPresenceIcon *icon, GtkIconSize icon_size);

G_END_DECLS

#endif /* PIDGIN_PRESENCE_ICON_H */
