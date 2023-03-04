/* Purple is the legal property of its developers, whose names are too numerous
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "pidgininvitedialog.h"

struct _PidginInviteDialog {
	GtkDialog parent;

	GtkWidget *contact;
	GtkWidget *message;

	PurpleChatConversation *conversation;
};

enum {
	PROP_ZERO,
	PROP_CONTACT,
	PROP_MESSAGE,
	PROP_CONVERSATION,
	N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES] = {0, };

G_DEFINE_TYPE(PidginInviteDialog, pidgin_invite_dialog, GTK_TYPE_DIALOG);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_invite_dialog_set_conversation(PidginInviteDialog *dialog,
                                      PurpleChatConversation *conversation)
{
	g_return_if_fail(PIDGIN_IS_INVITE_DIALOG(dialog));

	dialog->conversation = g_object_ref(conversation);

	g_object_notify_by_pspec(G_OBJECT(dialog), properties[PROP_CONVERSATION]);
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
pidgin_invite_dialog_get_property(GObject *obj,
                                  guint param_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
	PidginInviteDialog *dialog = PIDGIN_INVITE_DIALOG(obj);

	switch(param_id) {
		case PROP_CONTACT:
			g_value_set_string(value,
			                   pidgin_invite_dialog_get_contact(dialog));
			break;
		case PROP_MESSAGE:
			g_value_set_string(value,
			                   pidgin_invite_dialog_get_message(dialog));
			break;
		case PROP_CONVERSATION:
			g_value_set_object(value,
			                   pidgin_invite_dialog_get_conversation(dialog));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_invite_dialog_set_property(GObject *obj,
                                  guint param_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
	PidginInviteDialog *dialog = PIDGIN_INVITE_DIALOG(obj);

	switch(param_id) {
		case PROP_CONTACT:
			pidgin_invite_dialog_set_contact(dialog,
			                                 g_value_get_string(value));
			break;
		case PROP_MESSAGE:
			pidgin_invite_dialog_set_message(dialog,
			                                 g_value_get_string(value));
			break;
		case PROP_CONVERSATION:
			pidgin_invite_dialog_set_conversation(dialog,
			                                      g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_invite_dialog_finalize(GObject *obj) {
	PidginInviteDialog *dialog = PIDGIN_INVITE_DIALOG(obj);

	g_clear_pointer(&dialog->contact, g_free);
	g_clear_pointer(&dialog->message, g_free);
	g_clear_object(&dialog->conversation);

	G_OBJECT_CLASS(pidgin_invite_dialog_parent_class)->finalize(obj);
}

static void
pidgin_invite_dialog_init(PidginInviteDialog *dialog) {
	gtk_widget_init_template(GTK_WIDGET(dialog));
}

static void
pidgin_invite_dialog_class_init(PidginInviteDialogClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_invite_dialog_get_property;
	obj_class->set_property = pidgin_invite_dialog_set_property;
	obj_class->finalize = pidgin_invite_dialog_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Conversations/invite_dialog.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginInviteDialog,
	                                     contact);
	gtk_widget_class_bind_template_child(widget_class, PidginInviteDialog,
	                                     message);

	properties[PROP_CONTACT] = g_param_spec_string(
		"contact",
		"contact",
		"The person that is being invited",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	properties[PROP_MESSAGE] = g_param_spec_string(
		"message",
		"message",
		"The invite message to send",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	properties[PROP_CONVERSATION] = g_param_spec_object(
		"conversation",
		"conversation",
		"The conversation that someone is being invited to",
		PURPLE_TYPE_CHAT_CONVERSATION,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_invite_dialog_new(PurpleChatConversation *conversation) {
	return GTK_WIDGET(g_object_new(
		PIDGIN_TYPE_INVITE_DIALOG,
		"conversation", conversation,
		NULL));
}

const gchar *
pidgin_invite_dialog_get_contact(PidginInviteDialog *dialog) {
	g_return_val_if_fail(PIDGIN_IS_INVITE_DIALOG(dialog), NULL);

	return gtk_editable_get_text(GTK_EDITABLE(dialog->contact));
}

void
pidgin_invite_dialog_set_contact(PidginInviteDialog *dialog,
                                 const gchar *contact)
{
	g_return_if_fail(PIDGIN_IS_INVITE_DIALOG(dialog));

	if(contact != NULL) {
		gtk_editable_set_text(GTK_EDITABLE(dialog->contact), contact);

		g_object_notify_by_pspec(G_OBJECT(dialog), properties[PROP_CONTACT]);
	}
}

const gchar *
pidgin_invite_dialog_get_message(PidginInviteDialog *dialog) {
	g_return_val_if_fail(PIDGIN_IS_INVITE_DIALOG(dialog), NULL);

	return gtk_editable_get_text(GTK_EDITABLE(dialog->message));
}

void
pidgin_invite_dialog_set_message(PidginInviteDialog *dialog,
                                 const gchar *message)
{
	g_return_if_fail(PIDGIN_IS_INVITE_DIALOG(dialog));

	if(message != NULL) {
		gtk_editable_set_text(GTK_EDITABLE(dialog->message), message);

		g_object_notify_by_pspec(G_OBJECT(dialog), properties[PROP_MESSAGE]);
	}
}

PurpleChatConversation *
pidgin_invite_dialog_get_conversation(PidginInviteDialog *dialog) {
	g_return_val_if_fail(PIDGIN_IS_INVITE_DIALOG(dialog), NULL);

	return dialog->conversation;
}
