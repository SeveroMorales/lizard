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

#include "pidginprotocolchooser.h"

enum {
	PROP_ZERO,
	PROP_PROTOCOL,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _PidginProtocolChooser {
	AdwComboRow parent;

	GtkWidget *sort;
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
dropdown_changed_cb(G_GNUC_UNUSED GObject *obj,
                    G_GNUC_UNUSED GParamSpec *pspec,
                    gpointer data)
{
	PidginProtocolChooser *chooser = PIDGIN_PROTOCOL_CHOOSER(data);

	g_object_notify_by_pspec(G_OBJECT(chooser), properties[PROP_PROTOCOL]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginProtocolChooser, pidgin_protocol_chooser,
              ADW_TYPE_COMBO_ROW)

static void
pidgin_protocol_chooser_get_property(GObject *obj, guint prop_id,
                                     GValue *value, GParamSpec *pspec)
{
	PidginProtocolChooser *chooser = PIDGIN_PROTOCOL_CHOOSER(obj);

	switch(prop_id) {
		case PROP_PROTOCOL:
			g_value_set_object(value,
			                   pidgin_protocol_chooser_get_protocol(chooser));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
pidgin_protocol_chooser_set_property(GObject *obj, guint prop_id,
                                     const GValue *value, GParamSpec *pspec)
{
	PidginProtocolChooser *chooser = PIDGIN_PROTOCOL_CHOOSER(obj);

	switch(prop_id) {
		case PROP_PROTOCOL:
			pidgin_protocol_chooser_set_protocol(chooser,
			                                     g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
pidgin_protocol_chooser_class_init(PidginProtocolChooserClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_protocol_chooser_get_property;
	obj_class->set_property = pidgin_protocol_chooser_set_property;

	/**
	 * PidginProtocolChooser:protocol:
	 *
	 * The protocol which is currently selected.
	 *
	 * Since: 3.0.0
	 **/
	properties[PROP_PROTOCOL] = g_param_spec_object(
		"protocol",
		"protocol",
		"The PurpleProtocol which is currently selected",
		PURPLE_TYPE_PROTOCOL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class,
	                                            "/im/pidgin/Pidgin3/Protocols/chooser.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginProtocolChooser,
	                                     sort);

	gtk_widget_class_bind_template_callback(widget_class, dropdown_changed_cb);
}

static void
pidgin_protocol_chooser_init(PidginProtocolChooser *chooser) {
	PurpleProtocolManager *manager = NULL;

	gtk_widget_init_template(GTK_WIDGET(chooser));

	manager = purple_protocol_manager_get_default();
	gtk_sort_list_model_set_model(GTK_SORT_LIST_MODEL(chooser->sort),
	                              G_LIST_MODEL(manager));
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_protocol_chooser_new(void) {
	return g_object_new(PIDGIN_TYPE_PROTOCOL_CHOOSER, NULL);
}

PurpleProtocol *
pidgin_protocol_chooser_get_protocol(PidginProtocolChooser *chooser) {
	PurpleProtocol *protocol = NULL;

	g_return_val_if_fail(PIDGIN_IS_PROTOCOL_CHOOSER(chooser), NULL);

	protocol = adw_combo_row_get_selected_item(ADW_COMBO_ROW(chooser));

	return protocol;
}

void
pidgin_protocol_chooser_set_protocol(PidginProtocolChooser *chooser,
                                     PurpleProtocol *protocol)
{
	guint position = 0;

	g_return_if_fail(PIDGIN_IS_PROTOCOL_CHOOSER(chooser));

	if(protocol != NULL) {
		GListModel *model = adw_combo_row_get_model(ADW_COMBO_ROW(chooser));
		guint count = g_list_model_get_n_items(model);

		for(guint i = 0; i < count; i++) {
			PurpleProtocol *this_protocol = NULL;

			this_protocol = g_list_model_get_item(model, i);

			if(this_protocol == protocol) {
				position = i;
				break;
			}
		}
	}

	adw_combo_row_set_selected(ADW_COMBO_ROW(chooser), position);
}
