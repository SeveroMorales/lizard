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

#include "pidginiconname.h"

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
pidgin_icon_name_from_status_primitive(PurpleStatusPrimitive primitive,
                                       const gchar *fallback)
{
	switch(primitive) {
		case PURPLE_STATUS_OFFLINE:
			return "pidgin-user-offline";
			break;
		case PURPLE_STATUS_AVAILABLE:
			return "pidgin-user-available";
			break;
		case PURPLE_STATUS_UNAVAILABLE:
			return "pidgin-user-unavailable";
			break;
		case PURPLE_STATUS_AWAY:
			return "pidgin-user-away";
			break;
		case PURPLE_STATUS_INVISIBLE:
			return "pidgin-user-invisible";
			break;
		case PURPLE_STATUS_EXTENDED_AWAY:
			return "pidgin-user-extended-away";
			break;
		case PURPLE_STATUS_MOBILE:
		case PURPLE_STATUS_TUNE:
		case PURPLE_STATUS_MOOD:
		case PURPLE_STATUS_UNSET:
		default:
			break;
	}

	return fallback;
}

const gchar *
pidgin_icon_name_from_status_type(PurpleStatusType *type, const gchar *fallback)
{
	PurpleStatusPrimitive primitive;

	if(type == NULL) {
		return fallback;
	}

	primitive = purple_status_type_get_primitive(type);

	return pidgin_icon_name_from_status_primitive(primitive, fallback);
}

const gchar *
pidgin_icon_name_from_status(PurpleStatus *status, const gchar *fallback) {
	PurpleStatusType *type = NULL;

	if(!PURPLE_IS_STATUS(status)) {
		return fallback;
	}

	type = purple_status_get_status_type(status);
	if(type == NULL) {
		return fallback;
	}

	return pidgin_icon_name_from_status_type(type, fallback);
}

const gchar *
pidgin_icon_name_from_presence(PurplePresence *presence, const gchar *fallback)
{
	PurpleStatus *status = NULL;

	if(!PURPLE_IS_PRESENCE(presence)) {
		return fallback;
	}

	status = purple_presence_get_active_status(presence);

	return pidgin_icon_name_from_status(status, fallback);
}
