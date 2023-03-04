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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_NOOP_CREDENTIAL_PROVIDER_H
#define PURPLE_NOOP_CREDENTIAL_PROVIDER_H

#include <glib.h>
#include <glib-object.h>

#include <purplecredentialprovider.h>

G_BEGIN_DECLS

/**
 * PurpleNoopCredentialProvider:
 *
 * #PurpleNoopCredentialProvider is a #PurpleCredentialProvider that does not
 * actually provider credentials.  It is used to implement the default behavior
 * of requiring users to input passwords.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_NOOP_CREDENTIAL_PROVIDER (purple_noop_credential_provider_get_type())
G_DECLARE_FINAL_TYPE(PurpleNoopCredentialProvider,
                     purple_noop_credential_provider,
                     PURPLE, NOOP_CREDENTIAL_PROVIDER, PurpleCredentialProvider)

/**
 * purple_noop_credential_provider_new:
 *
 * Creates a new #PurpleNoopCredentialProvider instance.  You typically will
 * not need to call this directly as #PurpleCredentialManager will create one
 * for itself.
 *
 * Returns: (transfer full): The new #PurpleNoopCredentialProvider instance.
 */
PurpleCredentialProvider *purple_noop_credential_provider_new(void);

G_END_DECLS

#endif /* PURPLE_NOOP_CREDENTIAL_PROVIDER_H */
