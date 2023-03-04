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

#ifndef PURPLE_IRCV3_PROTOCOL_IM_H
#define PURPLE_IRCV3_PROTOCOL_IM_H

#include <glib.h>

#include <purple.h>

G_GNUC_INTERNAL void purple_ircv3_protocol_im_init(PurpleProtocolIMInterface *iface);

#endif /* PURPLE_IRCV3_PROTOCOL_IM_H */
