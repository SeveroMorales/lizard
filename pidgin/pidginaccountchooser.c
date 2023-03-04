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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301 USA
 */
#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include "pidginaccountchooser.h"

struct _PidginAccountChooser {
	AdwBin parent;

	GtkDropDown *chooser;
	GtkFilterListModel *filter;
};

enum
{
	PROP_0,
	PROP_ACCOUNT,
	PROP_FILTER,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = {NULL};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static char *
pidgin_account_chooser_icon_name_cb(G_GNUC_UNUSED GObject *self,
                                    PurpleAccount *account,
                                    G_GNUC_UNUSED gpointer data)
{
	const char *icon_name = NULL;

	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleProtocol *protocol = purple_account_get_protocol(account);
		icon_name = purple_protocol_get_icon_name(protocol);
	}

	return g_strdup(icon_name);
}

static char *
pidgin_account_chooser_label_cb(G_GNUC_UNUSED GObject *self,
                                PurpleAccount *account,
                                G_GNUC_UNUSED gpointer data)
{
	gchar *markup = NULL;
	const char *alias = NULL;
	const char *protocol_name = NULL;
	const char *username = NULL;

	if(!PURPLE_IS_ACCOUNT(account)) {
		return NULL;
	}

	alias = purple_contact_info_get_alias(PURPLE_CONTACT_INFO(account));
	protocol_name = purple_account_get_protocol_name(account);
	username = purple_contact_info_get_username(PURPLE_CONTACT_INFO(account));

	if(alias != NULL) {
		markup = g_strdup_printf(_("%s (%s) (%s)"), username, alias,
		                         protocol_name);
	} else {
		markup = g_strdup_printf(_("%s (%s)"), username, protocol_name);
	}

	return markup;
}

static void
pidgin_account_chooser_changed_cb(G_GNUC_UNUSED GObject *obj,
                                  G_GNUC_UNUSED GParamSpec *pspec,
                                  gpointer data)
{
	g_object_notify_by_pspec(G_OBJECT(data), properties[PROP_ACCOUNT]);
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginAccountChooser, pidgin_account_chooser, ADW_TYPE_BIN)

static void
pidgin_account_chooser_get_property(GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *pspec)
{
	PidginAccountChooser *chooser = PIDGIN_ACCOUNT_CHOOSER(object);

	switch (prop_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value, pidgin_account_chooser_get_selected(chooser));
			break;
		case PROP_FILTER:
			g_value_set_object(value, pidgin_account_chooser_get_filter(chooser));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
pidgin_account_chooser_set_property(GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *pspec)
{
	PidginAccountChooser *chooser = PIDGIN_ACCOUNT_CHOOSER(object);

	switch (prop_id) {
		case PROP_ACCOUNT:
			pidgin_account_chooser_set_selected(chooser,
			                                    g_value_get_object(value));
			break;
		case PROP_FILTER:
			pidgin_account_chooser_set_filter(chooser,
			                                  g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
pidgin_account_chooser_class_init(PidginAccountChooserClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	/* Properties */
	obj_class->get_property = pidgin_account_chooser_get_property;
	obj_class->set_property = pidgin_account_chooser_set_property;

	properties[PROP_ACCOUNT] = g_param_spec_object(
	        "account", "Account", "The account that is currently selected.",
	        PURPLE_TYPE_ACCOUNT,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_FILTER] = g_param_spec_object(
	        "filter", "filter",
	        "The filter to be applied on the list of accounts.",
	        GTK_TYPE_FILTER,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, PROP_LAST, properties);

	/* Widget template */
	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/Accounts/chooser.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginAccountChooser,
	                                     chooser);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountChooser,
	                                     filter);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_chooser_icon_name_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_chooser_label_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_chooser_changed_cb);
}

static void
pidgin_account_chooser_init(PidginAccountChooser *chooser) {
	GListModel *model = NULL;

	gtk_widget_init_template(GTK_WIDGET(chooser));

	model = purple_account_manager_get_default_as_model();
	gtk_filter_list_model_set_model(chooser->filter, model);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_account_chooser_new(void) {
	PidginAccountChooser *chooser = NULL;

	chooser = g_object_new(PIDGIN_TYPE_ACCOUNT_CHOOSER, NULL);

	return GTK_WIDGET(chooser);
}

GtkFilter *
pidgin_account_chooser_get_filter(PidginAccountChooser *chooser) {
	g_return_val_if_fail(PIDGIN_IS_ACCOUNT_CHOOSER(chooser), NULL);

	return gtk_filter_list_model_get_filter(chooser->filter);
}

void
pidgin_account_chooser_set_filter(PidginAccountChooser *chooser,
                                  GtkFilter *filter)
{
	g_return_if_fail(PIDGIN_IS_ACCOUNT_CHOOSER(chooser));

	gtk_filter_list_model_set_filter(chooser->filter, filter);
	g_object_notify_by_pspec(G_OBJECT(chooser), properties[PROP_FILTER]);
}

PurpleAccount *
pidgin_account_chooser_get_selected(PidginAccountChooser *chooser) {
	g_return_val_if_fail(PIDGIN_IS_ACCOUNT_CHOOSER(chooser), NULL);

	return gtk_drop_down_get_selected_item(chooser->chooser);
}

void
pidgin_account_chooser_set_selected(PidginAccountChooser *chooser,
                                    PurpleAccount *account)
{
	GListModel *model = NULL;
	guint n_items = 0;

	g_return_if_fail(PIDGIN_IS_ACCOUNT_CHOOSER(chooser));

	model = gtk_drop_down_get_model(chooser->chooser);
	g_return_if_fail(G_IS_LIST_MODEL(model));

	n_items = g_list_model_get_n_items(model);
	for(guint position = 0; position < n_items; position++) {
		PurpleAccount *acc = g_list_model_get_item(model, position);

		if(acc == account) {
			/* NOTE: Property notification occurs in 'changed' signal
			 * callback.
			 */
			gtk_drop_down_set_selected(chooser->chooser, position);

			g_object_unref(acc);

			return;
		}

		g_object_unref(acc);
	}
}
