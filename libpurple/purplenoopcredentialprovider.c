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

#include <glib/gi18n-lib.h>

#include "purplenoopcredentialprovider.h"

#include "purplecredentialmanager.h"

struct _PurpleNoopCredentialProvider {
	PurpleCredentialProvider parent;
};

G_DEFINE_TYPE(PurpleNoopCredentialProvider, purple_noop_credential_provider,
              PURPLE_TYPE_CREDENTIAL_PROVIDER)

/******************************************************************************
 * PurpleCredentialProvider Implementation
 *****************************************************************************/
static void
purple_noop_credential_provider_read_password_async(PurpleCredentialProvider *provider,
                                                    G_GNUC_UNUSED PurpleAccount *account,
                                                    GCancellable *cancellable,
                                                    GAsyncReadyCallback callback,
                                                    gpointer data)
{
	GTask *task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	g_task_return_new_error(task, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
	                        _("provider does not store passwords"));

	g_object_unref(G_OBJECT(task));
}

static gchar *
purple_noop_credential_provider_read_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                                     GAsyncResult *result,
                                                     GError **error)
{
	return g_task_propagate_pointer(G_TASK(result), error);
}

static void
purple_noop_credential_provider_write_password_async(PurpleCredentialProvider *provider,
                                                     G_GNUC_UNUSED PurpleAccount *account,
                                                     G_GNUC_UNUSED const char *password,
                                                     GCancellable *cancellable,
                                                     GAsyncReadyCallback callback,
                                                     gpointer data)
{
	GTask *task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	g_task_return_new_error(task, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
	                        _("provider does not store passwords"));

	g_object_unref(G_OBJECT(task));
}

static gboolean
purple_noop_credential_provider_write_password_finish(G_GNUC_UNUSED PurpleCredentialProvider *provider,
                                                      GAsyncResult *result,
                                                      GError **error)
{
	return g_task_propagate_boolean(G_TASK(result), error);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_noop_credential_provider_init(G_GNUC_UNUSED PurpleNoopCredentialProvider *provider)
{
}

static void
purple_noop_credential_provider_class_init(PurpleNoopCredentialProviderClass *klass)
{
	PurpleCredentialProviderClass *provider_class = NULL;

	provider_class = PURPLE_CREDENTIAL_PROVIDER_CLASS(klass);
	provider_class->read_password_async =
		purple_noop_credential_provider_read_password_async;
	provider_class->read_password_finish =
		purple_noop_credential_provider_read_password_finish;
	provider_class->write_password_async =
		purple_noop_credential_provider_write_password_async;
	provider_class->write_password_finish =
		purple_noop_credential_provider_write_password_finish;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleCredentialProvider *
purple_noop_credential_provider_new(void) {
	return PURPLE_CREDENTIAL_PROVIDER(g_object_new(
		PURPLE_TYPE_NOOP_CREDENTIAL_PROVIDER,
		"id", "noop-provider",
		"name", _("None"),
		"description", _("Passwords will not be saved."),
		NULL
	));
}
