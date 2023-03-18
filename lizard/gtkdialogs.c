/* pidgin
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <gst/gst.h>

#include <talkatu.h>

#include "package_revision.h"
#ifdef HAVE_MESON_CONFIG
#include "meson-config.h"
#endif

#include <purple.h>

#include "gtkblist.h"
#include "gtkdialogs.h"
#include "gtkutils.h"
#include "pidgincore.h"

struct _PidginGroupMergeObject {
	PurpleGroup* parent;
	char *new_name;
};

static void
pidgin_dialogs_im_cb(G_GNUC_UNUSED gpointer data, PurpleRequestFields *fields)
{
	PurpleAccount *account;
	const char *username;

	account  = purple_request_fields_get_account(fields, "account");
	username = purple_request_fields_get_string(fields,  "screenname");

	pidgin_dialogs_im_with_user(account, username);
}

static gboolean
pidgin_dialogs_im_name_validator(G_GNUC_UNUSED PurpleRequestField *field,
                                 char **errmsg, gpointer _fields)
{
	PurpleRequestFields *fields = _fields;
	PurpleAccount *account;
	PurpleProtocol *protocol;
	const char *username;
	gboolean valid = FALSE;

	account = purple_request_fields_get_account(fields, "account");
	protocol = purple_account_get_protocol(account);
	username = purple_request_fields_get_string(fields, "screenname");

	if (username) {
		valid = purple_validate(protocol, username);
	}

	if (errmsg && !valid)
		*errmsg = g_strdup(_("Invalid username"));

	return valid;
}

void
pidgin_dialogs_im(void)
{
	PurpleRequestFields *fields;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;

	fields = purple_request_fields_new();

	group = purple_request_field_group_new(NULL);
	purple_request_fields_add_group(fields, group);

	field = purple_request_field_string_new("screenname", _("_Name"), NULL, FALSE);
	purple_request_field_set_type_hint(field, "screenname");
	purple_request_field_set_required(field, TRUE);
	purple_request_field_set_validator(field, pidgin_dialogs_im_name_validator, fields);
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_account_new("account", _("_Account"), NULL);
	purple_request_field_set_type_hint(field, "account");
	purple_request_field_set_visible(field,
		(purple_connections_get_all() != NULL &&
		 purple_connections_get_all()->next != NULL));
	purple_request_field_set_required(field, TRUE);
	purple_request_field_group_add_field(group, field);

	purple_request_fields(
	        purple_blist_get_default(), _("New Instant Message"), NULL,
	        _("Please enter the username or alias of the person "
	          "you would like to IM."),
	        fields, _("OK"), G_CALLBACK(pidgin_dialogs_im_cb), _("Cancel"),
	        NULL, NULL, NULL);
}

void
pidgin_dialogs_im_with_user(PurpleAccount *account, const char *username)
{
	PurpleConversation *im;
	PurpleConversationManager *manager;

	g_return_if_fail(account != NULL);
	g_return_if_fail(username != NULL);

	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, account, username);

	if(!PURPLE_IS_IM_CONVERSATION(im)) {
		im = purple_im_conversation_new(account, username);
	}

	pidgin_conv_attach_to_conversation(im);
	purple_conversation_present(im);
}

static void
pidgin_dialogs_info_cb(G_GNUC_UNUSED gpointer data,
                       PurpleRequestFields *fields)
{
	char *username;
	PurpleAccount *account;
	const gchar *screenname = NULL;

	account = purple_request_fields_get_account(fields, "account");

	screenname = purple_request_fields_get_string(fields,  "screenname");
	username = g_strdup(purple_normalize(account, screenname));

	if(username != NULL && *username != '\0' && account != NULL) {
		pidgin_retrieve_user_info(purple_account_get_connection(account),
		                          username);
	}

	g_free(username);
}

void
pidgin_dialogs_info(void)
{
	PurpleRequestFields *fields;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;

	fields = purple_request_fields_new();

	group = purple_request_field_group_new(NULL);
	purple_request_fields_add_group(fields, group);

	field = purple_request_field_string_new("screenname", _("_Name"), NULL, FALSE);
	purple_request_field_set_type_hint(field, "screenname");
	purple_request_field_set_required(field, TRUE);
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_account_new("account", _("_Account"), NULL);
	purple_request_field_set_type_hint(field, "account");
	purple_request_field_set_visible(field,
		(purple_connections_get_all() != NULL &&
		 purple_connections_get_all()->next != NULL));
	purple_request_field_set_required(field, TRUE);
	purple_request_field_group_add_field(group, field);

	purple_request_fields(
	        purple_blist_get_default(), _("Get User Info"), NULL,
	        _("Please enter the username or alias of the person "
	          "whose info you would like to view."),
	        fields, _("OK"), G_CALLBACK(pidgin_dialogs_info_cb),
	        _("Cancel"), NULL, NULL, NULL);
}

static void
pidgin_dialogs_alias_buddy_cb(PurpleBuddy *buddy, const char *new_alias)
{
	purple_buddy_set_local_alias(buddy, new_alias);
	purple_serv_alias_buddy(buddy);
}

void
pidgin_dialogs_alias_buddy(PurpleBuddy *buddy)
{
	gchar *secondary;

	g_return_if_fail(buddy != NULL);

	secondary = g_strdup_printf(_("Enter an alias for %s."), purple_buddy_get_name(buddy));

	purple_request_input(NULL, _("Alias Buddy"), NULL,
					   secondary, purple_buddy_get_local_alias(buddy), FALSE, FALSE, NULL,
					   _("Alias"), G_CALLBACK(pidgin_dialogs_alias_buddy_cb),
					   _("Cancel"), NULL,
					   purple_request_cpar_from_account(purple_buddy_get_account(buddy)),
					   buddy);

	g_free(secondary);
}

static void
pidgin_dialogs_alias_chat_cb(PurpleChat *chat, const char *new_alias)
{
	purple_chat_set_alias(chat, new_alias);
}

void
pidgin_dialogs_alias_chat(PurpleChat *chat)
{
	gchar *alias;

	g_return_if_fail(chat != NULL);

	g_object_get(chat, "alias", &alias, NULL);

	purple_request_input(NULL, _("Alias Chat"), NULL,
					   _("Enter an alias for this chat."),
					   alias, FALSE, FALSE, NULL,
					   _("Alias"), G_CALLBACK(pidgin_dialogs_alias_chat_cb),
					   _("Cancel"), NULL,
					   purple_request_cpar_from_account(purple_chat_get_account(chat)),
					   chat);

	g_free(alias);
}

static void
pidgin_dialogs_remove_contact_cb(PurpleMetaContact *contact)
{
	PurpleBlistNode *bnode, *cnode;
	PurpleGroup *group;

	cnode = (PurpleBlistNode *)contact;
	group = (PurpleGroup*)cnode->parent;
	for (bnode = cnode->child; bnode; bnode = bnode->next) {
		PurpleBuddy *buddy = (PurpleBuddy*)bnode;
		if (purple_account_is_connected(purple_buddy_get_account(buddy)))
			purple_account_remove_buddy(purple_buddy_get_account(buddy), buddy, group);
	}
	purple_blist_remove_contact(contact);
}

void
pidgin_dialogs_remove_contact(PurpleMetaContact *contact)
{
	PurpleBuddy *buddy = purple_meta_contact_get_priority_buddy(contact);

	g_return_if_fail(contact != NULL);
	g_return_if_fail(buddy != NULL);

	if (PURPLE_BLIST_NODE(contact)->child == PURPLE_BLIST_NODE(buddy) &&
	    PURPLE_BLIST_NODE(buddy)->next == NULL) {
		pidgin_dialogs_remove_buddy(buddy);
	} else {
		gchar *text;
		int contact_size = purple_counting_node_get_total_size(PURPLE_COUNTING_NODE(contact));
		text = g_strdup_printf(
					ngettext(
						"You are about to remove the contact containing %s "
						"and %d other buddy from your buddy list.  Do you "
						"want to continue?",
						"You are about to remove the contact containing %s "
						"and %d other buddies from your buddy list.  Do you "
						"want to continue?", contact_size - 1),
					purple_buddy_get_name(buddy), contact_size - 1);

		purple_request_action(contact, NULL, _("Remove Contact"), text, 0,
				NULL,
				contact, 2,
				_("_Remove Contact"), G_CALLBACK(pidgin_dialogs_remove_contact_cb),
				_("Cancel"),
				NULL);

		g_free(text);
	}
}

static void free_ggmo(struct _PidginGroupMergeObject *ggp)
{
	g_free(ggp->new_name);
	g_free(ggp);
}

static void
pidgin_dialogs_merge_groups_cb(struct _PidginGroupMergeObject *GGP)
{
	purple_group_set_name(GGP->parent, GGP->new_name);
	free_ggmo(GGP);
}

void
pidgin_dialogs_merge_groups(PurpleGroup *source, const char *new_name)
{
	gchar *text;
	struct _PidginGroupMergeObject *ggp;

	g_return_if_fail(source != NULL);
	g_return_if_fail(new_name != NULL);

	text = g_strdup_printf(
				_("You are about to merge the group called %s into the group "
				"called %s. Do you want to continue?"), purple_group_get_name(source), new_name);

	ggp = g_new(struct _PidginGroupMergeObject, 1);
	ggp->parent = source;
	ggp->new_name = g_strdup(new_name);

	purple_request_action(source, NULL, _("Merge Groups"), text, 0,
			NULL,
			ggp, 2,
			_("_Merge Groups"), G_CALLBACK(pidgin_dialogs_merge_groups_cb),
			_("Cancel"), G_CALLBACK(free_ggmo));

	g_free(text);
}

static void
pidgin_dialogs_remove_group_cb(PurpleGroup *group)
{
	PurpleBlistNode *cnode, *bnode;

	cnode = ((PurpleBlistNode*)group)->child;

	while (cnode) {
		if (PURPLE_IS_META_CONTACT(cnode)) {
			bnode = cnode->child;
			cnode = cnode->next;
			while (bnode) {
				PurpleBuddy *buddy;
				if (PURPLE_IS_BUDDY(bnode)) {
					buddy = (PurpleBuddy*)bnode;
					bnode = bnode->next;
					if (purple_account_is_connected(purple_buddy_get_account(buddy))) {
						purple_account_remove_buddy(purple_buddy_get_account(buddy), buddy, group);
						purple_blist_remove_buddy(buddy);
					}
				} else {
					bnode = bnode->next;
				}
			}
		} else if (PURPLE_IS_CHAT(cnode)) {
			PurpleChat *chat = (PurpleChat *)cnode;
			cnode = cnode->next;
			if (purple_account_is_connected(purple_chat_get_account(chat)))
				purple_blist_remove_chat(chat);
		} else {
			cnode = cnode->next;
		}
	}

	purple_blist_remove_group(group);
}

void
pidgin_dialogs_remove_group(PurpleGroup *group)
{
	gchar *text;

	g_return_if_fail(group != NULL);

	text = g_strdup_printf(_("You are about to remove the group %s and all its members from your buddy list.  Do you want to continue?"),
						   purple_group_get_name(group));

	purple_request_action(group, NULL, _("Remove Group"), text, 0,
						NULL,
						group, 2,
						_("_Remove Group"), G_CALLBACK(pidgin_dialogs_remove_group_cb),
						_("Cancel"), NULL);

	g_free(text);
}

/* XXX - Some of this should be moved into the core, methinks. */
static void
pidgin_dialogs_remove_buddy_cb(PurpleBuddy *buddy)
{
	PurpleGroup *group;
	gchar *name;
	PurpleAccount *account;

	group = purple_buddy_get_group(buddy);
	name = g_strdup(purple_buddy_get_name(buddy)); /* purple_buddy_get_name() is a crasher after remove_buddy */
	account = purple_buddy_get_account(buddy);

	purple_debug_info("blist", "Removing '%s' from buddy list.\n", purple_buddy_get_name(buddy));
	/* TODO - Should remove from blist first... then call purple_account_remove_buddy()? */
	purple_account_remove_buddy(account, buddy, group);
	purple_blist_remove_buddy(buddy);

	g_free(name);
}

void
pidgin_dialogs_remove_buddy(PurpleBuddy *buddy)
{
	gchar *text;

	g_return_if_fail(buddy != NULL);

	text = g_strdup_printf(_("You are about to remove %s from your buddy list.  Do you want to continue?"),
						   purple_buddy_get_name(buddy));

	purple_request_action(buddy, NULL, _("Remove Buddy"), text, 0,
		purple_request_cpar_from_account(
			purple_buddy_get_account(buddy)),
		buddy, 2, _("_Remove Buddy"),
		G_CALLBACK(pidgin_dialogs_remove_buddy_cb), _("Cancel"), NULL);

	g_free(text);
}

static void
pidgin_dialogs_remove_chat_cb(PurpleChat *chat)
{
	purple_blist_remove_chat(chat);
}

void
pidgin_dialogs_remove_chat(PurpleChat *chat)
{
	const gchar *name;
	gchar *text;

	g_return_if_fail(chat != NULL);

	name = purple_chat_get_name(chat);
	text = g_strdup_printf(_("You are about to remove the chat %s from your buddy list.  Do you want to continue?"),
			name ? name : "");

	purple_request_action(chat, NULL, _("Remove Chat"), text, 0,
		purple_request_cpar_from_account(purple_chat_get_account(chat)),
		chat, 2, _("_Remove Chat"),
		G_CALLBACK(pidgin_dialogs_remove_chat_cb), _("Cancel"), NULL);

	g_free(text);
}
