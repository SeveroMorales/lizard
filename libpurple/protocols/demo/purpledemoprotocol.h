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

#ifndef PURPLE_DEMO_PROTOCOL_H
#define PURPLE_DEMO_PROTOCOL_H

#include <glib.h>
#include <glib-object.h>

#include <gplugin-native.h>

#include <purple.h>

G_BEGIN_DECLS

#define PURPLE_DEMO_TYPE_PROTOCOL (purple_demo_protocol_get_type())
G_DECLARE_FINAL_TYPE(PurpleDemoProtocol, purple_demo_protocol, PURPLE_DEMO,
                     PROTOCOL, PurpleProtocol)

G_GNUC_INTERNAL void purple_demo_protocol_register(GPluginNativePlugin *plugin);

G_GNUC_INTERNAL PurpleProtocol *purple_demo_protocol_new(void);

G_END_DECLS

#endif /* PURPLE_DEMO_PROTOCOL_H */
