/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include "purpleconnectionerrorinfo.h"

G_DEFINE_BOXED_TYPE(PurpleConnectionErrorInfo, purple_connection_error_info,
                    purple_connection_error_info_copy,
                    purple_connection_error_info_free)

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleConnectionErrorInfo *
purple_connection_error_info_new(PurpleConnectionError type,
                                 const gchar *description)
{
	PurpleConnectionErrorInfo *info;

	g_return_val_if_fail(description != NULL, NULL);

	info = g_new(PurpleConnectionErrorInfo, 1);

	info->type = type;
	info->description = g_strdup(description);

	return info;
}

PurpleConnectionErrorInfo *
purple_connection_error_info_copy(PurpleConnectionErrorInfo *info)
{
	g_return_val_if_fail(info != NULL, NULL);

	return purple_connection_error_info_new(info->type, info->description);
}

void
purple_connection_error_info_free(PurpleConnectionErrorInfo *info) {
	g_return_if_fail(info != NULL);

	g_free(info->description);
	g_free(info);
}
