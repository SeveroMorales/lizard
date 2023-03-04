/*
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#include "purpleprotocolim.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolIM, purple_protocol_im, PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_im_default_init(G_GNUC_UNUSED PurpleProtocolIMInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
gint
purple_protocol_im_send(PurpleProtocolIM *im, PurpleConnection *gc,
                        PurpleMessage *msg)
{
	PurpleProtocolIMInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_IM(im), -1);

	iface = PURPLE_PROTOCOL_IM_GET_IFACE(im);
	if(iface && iface->send) {
		return iface->send(im, gc, msg);
	}

	return -1;
}

guint
purple_protocol_im_send_typing(PurpleProtocolIM *im, PurpleConnection *gc,
		                       const gchar *name, PurpleIMTypingState state)
{
	PurpleProtocolIMInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_IM(im), 0);

	iface = PURPLE_PROTOCOL_IM_GET_IFACE(im);
	if(iface && iface->send_typing) {
		return iface->send_typing(im, gc, name, state);
	}

	return 0;
}

