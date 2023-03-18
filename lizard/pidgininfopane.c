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

#include "pidgin/pidgininfopane.h"

#include "pidgin/gtkblist.h"
#include "pidgin/gtkutils.h"
#include "pidgin/pidginavatar.h"
#include "pidgin/pidgincore.h"
#include "pidgin/pidginpresenceicon.h"

struct _PidginInfoPane {
	GtkBox parent;

	PurpleConversation *conversation;

	GtkWidget *hbox;
	GtkWidget *presence_icon;
	GtkWidget *name;
	GtkWidget *topic;
	GtkWidget *avatar;
};

enum {
	PROP_0,
	PROP_CONVERSATION,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PidginInfoPane, pidgin_info_pane, GTK_TYPE_BOX)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_info_pane_set_conversation(PidginInfoPane *pane,
                                  PurpleConversation *conversation)
{
	if(!g_set_object(&pane->conversation, conversation)) {
		return;
	}

	/* Tell the avatar about the new conversation. */
	pidgin_avatar_set_conversation(PIDGIN_AVATAR(pane->avatar),
	                               pane->conversation);

	if(PURPLE_IS_CONVERSATION(conversation)) {
		PidginPresenceIcon *presence_icon = NULL;
		PurplePresence *presence = NULL;
		const gchar *fallback = "person";

		g_object_bind_property(G_OBJECT(conversation), "name",
		                       G_OBJECT(pane->name), "label",
		                       G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

		if(PURPLE_IS_CHAT_CONVERSATION(conversation)) {
			g_object_bind_property(G_OBJECT(conversation), "topic",
			                       G_OBJECT(pane->topic), "label",
			                       G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
			fallback = "chat";
		} else if(PURPLE_IS_IM_CONVERSATION(conversation)){
			PurpleAccount *account = NULL;
			PurpleBuddy *buddy = NULL;
			const gchar *name = NULL;

			account = purple_conversation_get_account(conversation);
			name = purple_conversation_get_name(conversation);
			buddy = purple_blist_find_buddy(account, name);

			if(PURPLE_IS_BUDDY(buddy)) {
				presence = purple_buddy_get_presence(buddy);
			}

			fallback = "person";
		}

		presence_icon = PIDGIN_PRESENCE_ICON(pane->presence_icon);
		pidgin_presence_icon_set_fallback(presence_icon, fallback);
		pidgin_presence_icon_set_presence(presence_icon, presence);
	}

	/* Notify that we changed. */
	g_object_notify_by_pspec(G_OBJECT(pane), properties[PROP_CONVERSATION]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_info_pane_get_property(GObject *obj, guint param_id, GValue *value,
                              GParamSpec *pspec)
{
	PidginInfoPane *pane = PIDGIN_INFO_PANE(obj);

	switch(param_id) {
		case PROP_CONVERSATION:
			g_value_set_object(value, pidgin_info_pane_get_conversation(pane));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_info_pane_set_property(GObject *obj, guint param_id, const GValue *value,
                              GParamSpec *pspec)
{
	PidginInfoPane *pane = PIDGIN_INFO_PANE(obj);

	switch(param_id) {
		case PROP_CONVERSATION:
			pidgin_info_pane_set_conversation(pane, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_info_pane_dispose(GObject *obj) {
	PidginInfoPane *pane = PIDGIN_INFO_PANE(obj);

	g_clear_object(&pane->conversation);

	G_OBJECT_CLASS(pidgin_info_pane_parent_class)->dispose(obj);
}

static void
pidgin_info_pane_init(PidginInfoPane *pane) {
	gtk_widget_init_template(GTK_WIDGET(pane));
}

static void
pidgin_info_pane_class_init(PidginInfoPaneClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_info_pane_get_property;
	obj_class->set_property = pidgin_info_pane_set_property;
	obj_class->dispose = pidgin_info_pane_dispose;

	/**
	 * PidginInfoPane:conversation
	 *
	 * The #PurpleConversation whose information will be displayed.
	 */
	properties[PROP_CONVERSATION] = g_param_spec_object(
		"conversation", "conversation",
		"The PurpleConversation instance",
		PURPLE_TYPE_CONVERSATION,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Conversations/infopane.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginInfoPane, hbox);
	gtk_widget_class_bind_template_child(widget_class, PidginInfoPane, avatar);
	gtk_widget_class_bind_template_child(widget_class, PidginInfoPane, presence_icon);
	gtk_widget_class_bind_template_child(widget_class, PidginInfoPane, name);
	gtk_widget_class_bind_template_child(widget_class, PidginInfoPane, topic);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_info_pane_new(PurpleConversation *conversation) {
	return GTK_WIDGET(g_object_new(
		PIDGIN_TYPE_INFO_PANE,
		"conversation", conversation,
		NULL));
}

PurpleConversation *
pidgin_info_pane_get_conversation(PidginInfoPane *pane) {
	g_return_val_if_fail(PIDGIN_IS_INFO_PANE(pane), NULL);

	return pane->conversation;
}
