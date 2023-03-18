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

#ifndef PIDGIN_ICON_NAME_H
#define PIDGIN_ICON_NAME_H

#include <glib.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * pidgin_icon_name_from_status_primitive:
 * @primitive: The #PurpleStatusPrimitive.
 * @fallback: The icon name to return if an icon name can not be found.
 *
 * Gets the icon name to use for @primitive if found, otherwise returns
 * @fallback.
 *
 * Returns: The icon name to use for @primitive.
 *
 * Since: 3.0.0
 */
const gchar *pidgin_icon_name_from_status_primitive(PurpleStatusPrimitive primitive, const gchar *fallback);

/**
 * pidgin_icon_name_from_status_type:
 * @type: The #PurpleStatusType.
 * @fallback: The icon name to return if an icon name can not be found.
 *
 * Gets the icon name to use for @type if found, otherwise returns @fallback.
 *
 * Returns: The icon name to use for @type.
 *
 * Since: 3.0.0
 */
const gchar *pidgin_icon_name_from_status_type(PurpleStatusType *type, const gchar *fallback);

/**
 * pidgin_icon_name_from_status:
 * @status: The #PurpleStatus instance.
 * @fallback: The icon name to return if an icon name can not be found.
 *
 * Gets the icon name to use for @status if found, otherwise returns @fallback.
 *
 * Returns: The icon name to use for @status.
 *
 * Since: 3.0.0
 */
const gchar *pidgin_icon_name_from_status(PurpleStatus *status, const gchar *fallback);

/**
 * pidgin_icon_name_from_presence:
 * @presence: The #PurplePresence instance.
 * @fallback: The icon name to fall back to.
 *
 * Gets the icon name that should be used to represent @presence falling back
 * to @fallback if @presence is invalid.
 *
 * Returns: The icon name to represent @presence.
 *
 * Since: 3.0.0
 */
const gchar *pidgin_icon_name_from_presence(PurplePresence *presence, const gchar *fallback);

G_END_DECLS

#endif /* PIDGIN_ICON_NAME_H */
