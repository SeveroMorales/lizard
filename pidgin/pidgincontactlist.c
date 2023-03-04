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

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "pidgin/pidgincontactlist.h"

struct _PidginContactList {
	GtkBox parent;

	GtkFilterListModel *filter_model;
	GtkCustomFilter *search_filter;

	GtkWidget *search_entry;
	GtkWidget *view;
};

G_DEFINE_TYPE(PidginContactList, pidgin_contact_list, GTK_TYPE_BOX)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
pidgin_contact_list_search_filter(GObject *item, gpointer data) {
	PidginContactList *list = data;
	PurplePerson *person = PURPLE_PERSON(item);
	const char *needle = NULL;

	needle = gtk_editable_get_text(GTK_EDITABLE(list->search_entry));

	return purple_person_matches(person, needle);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_contact_list_search_changed_cb(G_GNUC_UNUSED GtkSearchEntry *self,
                                      gpointer data)
{
	PidginContactList *list = data;

	gtk_filter_changed(GTK_FILTER(list->search_filter),
	                   GTK_FILTER_CHANGE_DIFFERENT);
}

static GdkTexture *
pidgin_contact_list_avatar_cb(G_GNUC_UNUSED GObject *self,
                              PurplePerson *person,
                              G_GNUC_UNUSED gpointer data)
{
	PurpleContactInfo *info = NULL;
	PurpleContact *contact = NULL;
	PurpleBuddyIcon *icon = NULL;
	GdkTexture *texture = NULL;
	GdkPixbuf *pixbuf = NULL;

	/* When filtering we get called for rows that have been filtered out. We
	 * also get called during finalization. I'm not sure why either of these
	 * cases happen, but they do.
	 */
	if(!PURPLE_IS_PERSON(person)) {
		return NULL;
	}

	pixbuf = purple_person_get_avatar_for_display(person);
	if(GDK_IS_PIXBUF(pixbuf)) {
		return gdk_texture_new_for_pixbuf(pixbuf);
	}

	info = purple_person_get_priority_contact_info(person);

	/* All of the contact info in the manager are PurpleContact's so this cast
	 * is fine.
	 */
	contact = PURPLE_CONTACT(info);

	icon = purple_buddy_icons_find(purple_contact_get_account(contact),
	                               purple_contact_info_get_username(info));

	if(icon != NULL) {
		GBytes *bytes = NULL;
		GError *error = NULL;
		gsize size;
		gconstpointer data = NULL;

		data = purple_buddy_icon_get_data(icon, &size);
		bytes = g_bytes_new(data, size);

		texture = gdk_texture_new_from_bytes(bytes, &error);
		g_bytes_unref(bytes);

		if(error != NULL) {
			g_warning("Failed to create texture: %s", error->message);

			g_clear_error(&error);
		}
	}

	return texture;
}

static void
pidgin_contact_list_activate_cb(GtkListView *self, guint position,
                                G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = NULL;
	PurpleContactInfo *info = NULL;
	PurpleConversation *conversation = NULL;
	PurpleConversationManager *manager = NULL;
	PurplePerson *person = NULL;
	GtkSelectionModel *model = NULL;
	const char *name = NULL;

	model = gtk_list_view_get_model(self);

	person = g_list_model_get_item(G_LIST_MODEL(model), position);
	if(!PURPLE_IS_PERSON(person)) {
		g_warning("we seem to have activated a zombie.. RUN!!!!!!");

		return;
	}

	info = purple_person_get_priority_contact_info(person);
	account = purple_contact_get_account(PURPLE_CONTACT(info));
	name = purple_contact_info_get_username(info);

	manager = purple_conversation_manager_get_default();
	conversation = purple_conversation_manager_find_im(manager, account, name);

	if(!PURPLE_IS_CONVERSATION(conversation)) {
		conversation = purple_im_conversation_new(account, name);
		purple_conversation_manager_register(manager, conversation);
	}
}

static gboolean
pidgin_contact_list_message_visible_cb(G_GNUC_UNUSED GtkListItem *item,
                                       const char *message)
{
	/* If we have a message, return TRUE because this is bound to the label's
	 * visibility.
	 */
	return (message != NULL);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_contact_list_init(PidginContactList *list) {
	PurpleContactManager *manager = NULL;

	gtk_widget_init_template(GTK_WIDGET(list));

	manager = purple_contact_manager_get_default();
	gtk_filter_list_model_set_model(list->filter_model, G_LIST_MODEL(manager));

	gtk_custom_filter_set_filter_func(list->search_filter,
	                                  (GtkCustomFilterFunc)pidgin_contact_list_search_filter,
	                                  list, NULL);
}

static void
pidgin_contact_list_class_init(PidginContactListClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/ContactList/widget.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginContactList,
	                                     filter_model);
	gtk_widget_class_bind_template_child(widget_class, PidginContactList,
	                                     search_filter);

	gtk_widget_class_bind_template_child(widget_class, PidginContactList,
	                                     search_entry);
	gtk_widget_class_bind_template_child(widget_class, PidginContactList,
	                                     view);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_contact_list_search_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_contact_list_avatar_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_contact_list_activate_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_contact_list_message_visible_cb);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_contact_list_new(void) {
	return g_object_new(PIDGIN_TYPE_CONTACT_LIST, NULL);
}
