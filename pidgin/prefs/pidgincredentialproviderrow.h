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

#ifndef PIDGIN_CREDENTIAL_PROVIDER_ROW_H
#define PIDGIN_CREDENTIAL_PROVIDER_ROW_H

#include <glib.h>

#include <gtk/gtk.h>

#include <adwaita.h>

G_BEGIN_DECLS

/**
 * PIDGIN_TYPE_CREDENTIAL_PROVIDER_ROW:
 *
 * The standard _get_type macro for #PidginCredentialProviderRow.
 *
 * Since: 3.0.0
 */
#define PIDGIN_TYPE_CREDENTIAL_PROVIDER_ROW (pidgin_credential_provider_row_get_type())
G_DECLARE_FINAL_TYPE(PidginCredentialProviderRow,
                     pidgin_credential_provider_row,
                     PIDGIN, CREDENTIAL_PROVIDER_ROW, AdwActionRow)

/**
 * PidginCredentialProviderRow:
 *
 * #PidginCredentialProviderRow is a widget for the preferences window to let
 * users configure their credential provider.
 *
 * Since: 3.0.0
 */

/**
 * pidgin_credential_provider_row_new:
 * @provider: The credential provider to bind.
 *
 * Creates a new #PidginCredentialProviderRow instance.
 *
 * Returns: (transfer full): The new #PidginCredentialProviderRow instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_credential_provider_row_new(PurpleCredentialProvider *provider);

/**
 * pidgin_credential_provider_row_get_provider:
 * @row: The row instance.
 *
 * Gets the #PurpleCredentialProvider displayed by this widget.
 *
 * Returns: (transfer none): The displayed #PurpleCredentialProvider.
 *
 * Since: 3.0.0
 */
PurpleCredentialProvider *pidgin_credential_provider_row_get_provider(PidginCredentialProviderRow *row);

/**
 * pidgin_credential_provider_row_get_active:
 * @row: The row instance.
 *
 * Gets whether the row is displayed as active.
 *
 * Returns: Whether the row is active.
 *
 * Since: 3.0.0
 */
gboolean pidgin_credential_provider_row_get_active(PidginCredentialProviderRow *row);

/**
 * pidgin_credential_provider_row_set_active:
 * @row: The row instance.
 * @active: Whether to display as active.
 *
 * Sets whether the row is displayed as active.
 *
 * Since: 3.0.0
 */
void pidgin_credential_provider_row_set_active(PidginCredentialProviderRow *row, gboolean active);

G_END_DECLS

#endif /* PIDGIN_CREDENTIAL_PROVIDER_ROW_H */
