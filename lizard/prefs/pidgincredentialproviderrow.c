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

#include <purple.h>

#include <adwaita.h>

#include "pidgincredentialproviderrow.h"

struct _PidginCredentialProviderRow {
	AdwActionRow parent;

	PurpleCredentialProvider *provider;

	GtkWidget *active;
	GtkWidget *configure;
};

enum {
	PROP_0,
	PROP_PROVIDER,
	PROP_ACTIVE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PidginCredentialProviderRow, pidgin_credential_provider_row,
              ADW_TYPE_ACTION_ROW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_credential_provider_row_set_provider(PidginCredentialProviderRow *row,
                                            PurpleCredentialProvider *provider)
{
	if(!g_set_object(&row->provider, provider)) {
		return;
	}

	if(PURPLE_IS_CREDENTIAL_PROVIDER(provider)) {
		adw_preferences_row_set_title(
		    ADW_PREFERENCES_ROW(row),
		    purple_credential_provider_get_name(provider));
		adw_action_row_set_subtitle(
		    ADW_ACTION_ROW(row),
		    purple_credential_provider_get_description(provider));
		/* Not implemented yet, so always hide the configure button. */
		gtk_widget_set_visible(row->configure, FALSE);
	}

	/* Notify that we changed. */
	g_object_notify_by_pspec(G_OBJECT(row), properties[PROP_PROVIDER]);
}


/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_credential_provider_row_get_property(GObject *obj, guint param_id,
                                            GValue *value, GParamSpec *pspec)
{
	PidginCredentialProviderRow *row = PIDGIN_CREDENTIAL_PROVIDER_ROW(obj);

	switch(param_id) {
		case PROP_PROVIDER:
			g_value_set_object(value,
			                   pidgin_credential_provider_row_get_provider(row));
			break;
		case PROP_ACTIVE:
			g_value_set_boolean(value,
			                    pidgin_credential_provider_row_get_active(row));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_credential_provider_row_set_property(GObject *obj, guint param_id,
                                            const GValue *value,
                                            GParamSpec *pspec)
{
	PidginCredentialProviderRow *row = PIDGIN_CREDENTIAL_PROVIDER_ROW(obj);

	switch(param_id) {
		case PROP_PROVIDER:
			pidgin_credential_provider_row_set_provider(row,
			                                            g_value_get_object(value));
			break;
		case PROP_ACTIVE:
			pidgin_credential_provider_row_set_active(row,
			                                          g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_credential_provider_row_finalize(GObject *obj)
{
	PidginCredentialProviderRow *row = PIDGIN_CREDENTIAL_PROVIDER_ROW(obj);

	g_clear_object(&row->provider);
}

static void
pidgin_credential_provider_row_init(PidginCredentialProviderRow *row)
{
	gtk_widget_init_template(GTK_WIDGET(row));
}

static void
pidgin_credential_provider_row_class_init(PidginCredentialProviderRowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_credential_provider_row_get_property;
	obj_class->set_property = pidgin_credential_provider_row_set_property;
	obj_class->finalize = pidgin_credential_provider_row_finalize;

	/**
	 * PidginCredentialProviderRow:provider
	 *
	 * The #PurpleCredentialProvider whose information will be displayed.
	 */
	properties[PROP_PROVIDER] = g_param_spec_object(
		"provider", "provider",
		"The PurpleCredentialProvider instance",
		PURPLE_TYPE_CREDENTIAL_PROVIDER,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginCredentialProviderRow:active
	 *
	 * Whether the #PurpleCredentialProvider is currently active.
	 */
	properties[PROP_ACTIVE] = g_param_spec_boolean(
		"active", "active",
		"Whether the PurpleCredentialProvider is active",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Prefs/credentialprovider.ui"
	);

	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginCredentialProviderRow,
	                                     active);
	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginCredentialProviderRow,
	                                     configure);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_credential_provider_row_new(PurpleCredentialProvider *provider) {
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	return GTK_WIDGET(g_object_new(PIDGIN_TYPE_CREDENTIAL_PROVIDER_ROW,
	                               "provider", provider,
	                               NULL));
}

PurpleCredentialProvider *
pidgin_credential_provider_row_get_provider(PidginCredentialProviderRow *row) {
	g_return_val_if_fail(PIDGIN_IS_CREDENTIAL_PROVIDER_ROW(row), NULL);

	return row->provider;
}

gboolean
pidgin_credential_provider_row_get_active(PidginCredentialProviderRow *row) {
	g_return_val_if_fail(PIDGIN_IS_CREDENTIAL_PROVIDER_ROW(row), FALSE);

	return gtk_check_button_get_active(GTK_CHECK_BUTTON(row->active));
}

void
pidgin_credential_provider_row_set_active(PidginCredentialProviderRow *row,
                                          gboolean active)
{
	g_return_if_fail(PIDGIN_IS_CREDENTIAL_PROVIDER_ROW(row));

	gtk_check_button_set_active(GTK_CHECK_BUTTON(row->active), active);

	g_object_notify_by_pspec(G_OBJECT(row), properties[PROP_ACTIVE]);
}
