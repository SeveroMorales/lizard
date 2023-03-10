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

#ifndef PURPLE_DEMO_CONNECTION_H
#define PURPLE_DEMO_CONNECTION_H

#include <glib.h>

#include <purple.h>

G_BEGIN_DECLS

#define PURPLE_DEMO_TYPE_CONNECTION (purple_demo_connection_get_type())
G_DECLARE_FINAL_TYPE(PurpleDemoConnection, purple_demo_connection, PURPLE_DEMO,
                     CONNECTION, PurpleConnection)

G_GNUC_INTERNAL void purple_demo_connection_register(GPluginNativePlugin *plugin);

G_BEGIN_DECLS

#endif /* PURPLE_DEMO_CONNECTION_H */
