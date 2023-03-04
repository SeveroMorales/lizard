/*
 * idle.c - I'dle Mak'er plugin for Purple
 *
 * This file is part of Purple.
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

/* This plugin no longer depends on gtk */
#define IDLE_PLUGIN_ID "core-idle"

static GList *idled_accts = NULL;

static gboolean
unidle_filter(PurpleAccount *acct)
{
	if (g_list_find(idled_accts, acct))
		return TRUE;

	return FALSE;
}

static gboolean
idleable_filter(PurpleAccount *account)
{
	PurpleProtocol *protocol;

	protocol = purple_account_get_protocol(account);
	g_return_val_if_fail(protocol != NULL, FALSE);

	return PURPLE_PROTOCOL_IMPLEMENTS(protocol, SERVER, set_idle);
}

static void
set_idle_time(PurpleAccount *acct, int mins_idle)
{
	PurpleConnection *gc = purple_account_get_connection(acct);
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(acct);
	PurplePresence *presence = purple_account_get_presence(acct);
	GDateTime *idle_since = NULL;
	gboolean idle = FALSE;

	if (!gc)
		return;

	purple_debug_info("idle", "setting idle time for %s to %d\n",
	                  purple_contact_info_get_username(info), mins_idle);

	idle = mins_idle > 0;

	if(idle) {
		GDateTime *now = g_date_time_new_now_local();
		idle_since = g_date_time_add_minutes(now, -1 * mins_idle);
		g_date_time_unref(now);
	}

	purple_presence_set_idle(presence, idle, idle_since);

	g_clear_pointer(&idle_since, g_date_time_unref);
}

static void
idle_action_ok(G_GNUC_UNUSED gpointer data, PurpleRequestFields *fields)
{
	PurpleAccount *acct = purple_request_fields_get_account(fields, "acct");
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(acct);
	int tm = purple_request_fields_get_integer(fields, "mins");

	/* only add the account to the GList if it's not already been idled */
	if(!unidle_filter(acct)) {
		purple_debug_misc("idle", "%s hasn't been idled yet; adding to list.",
		                  purple_contact_info_get_username(info));
		idled_accts = g_list_append(idled_accts, acct);
	}

	set_idle_time(acct, tm);
}

static void
idle_all_action_ok(G_GNUC_UNUSED gpointer data, PurpleRequestFields *fields)
{
	PurpleAccountManager *manager = NULL;
	PurpleAccount *acct = NULL;
	GList *list, *iter;
	int tm = purple_request_fields_get_integer(fields, "mins");

	manager = purple_account_manager_get_default();
	list = purple_account_manager_get_enabled(manager);
	for(iter = list; iter; iter = iter->next) {
		acct = (PurpleAccount *)(iter->data);

		if(acct && idleable_filter(acct)) {
			PurpleContactInfo *info = PURPLE_CONTACT_INFO(acct);

			purple_debug_misc("idle", "Idling %s.\n",
			                  purple_contact_info_get_username(info));

			set_idle_time(acct, tm);

			if(!g_list_find(idled_accts, acct))
				idled_accts = g_list_append(idled_accts, acct);
		}
	}

	g_list_free(list);
}

static void
unidle_action_ok(G_GNUC_UNUSED gpointer data, PurpleRequestFields *fields)
{
	PurpleAccount *acct = purple_request_fields_get_account(fields, "acct");

	set_idle_time(acct, 0); /* unidle the account */

	/* once the account has been unidled it shouldn't be in the list */
	idled_accts = g_list_remove(idled_accts, acct);
}

static void
signing_off_cb(PurpleConnection *gc, G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account;

	account = purple_connection_get_account(gc);
	idled_accts = g_list_remove(idled_accts, account);
}

/******************************************************************************
 * Actions
 *****************************************************************************/
static void
purple_idle_set_account_idle_time(G_GNUC_UNUSED GSimpleAction *action,
                                  G_GNUC_UNUSED GVariant *parameter,
                                  gpointer data)
{
	/* Use the super fancy request API */

	PurpleRequestFields *request;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;

	group = purple_request_field_group_new(NULL);

	field = purple_request_field_account_new("acct", _("Account"), NULL);
	purple_request_field_account_set_filter(field, idleable_filter);
	purple_request_field_account_set_show_all(field, FALSE);
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_int_new("mins", _("Minutes"), 10, 0, 9999);
	purple_request_field_group_add_field(group, field);

	request = purple_request_fields_new();
	purple_request_fields_add_group(request, group);

	purple_request_fields(data,
			N_("I'dle Mak'er"),
			_("Set Account Idle Time"),
			NULL,
			request,
			_("_Set"), G_CALLBACK(idle_action_ok),
			_("_Cancel"), NULL,
			NULL, NULL);
}

static void
purple_idle_unset_account_idle_time(G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data)
{
	PurpleRequestFields *request;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;

	if (idled_accts == NULL)
	{
		purple_notify_info(NULL, NULL, _("None of your accounts are idle."), NULL, NULL);
		return;
	}

	group = purple_request_field_group_new(NULL);

	field = purple_request_field_account_new("acct", _("Account"), NULL);
	purple_request_field_account_set_filter(field, unidle_filter);
	purple_request_field_account_set_show_all(field, FALSE);
	purple_request_field_group_add_field(group, field);

	request = purple_request_fields_new();
	purple_request_fields_add_group(request, group);

	purple_request_fields(data,
			N_("I'dle Mak'er"),
			_("Unset Account Idle Time"),
			NULL,
			request,
			_("_Unset"), G_CALLBACK(unidle_action_ok),
			_("_Cancel"), NULL,
			NULL, NULL);
}

static void
purple_idle_set_all_accounts_idle_time(G_GNUC_UNUSED GSimpleAction *action,
                                       G_GNUC_UNUSED GVariant *parameter,
                                       gpointer data)
{
	PurpleRequestFields *request;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;

	group = purple_request_field_group_new(NULL);

	field = purple_request_field_int_new("mins", _("Minutes"), 10, 0, 9999);
	purple_request_field_group_add_field(group, field);

	request = purple_request_fields_new();
	purple_request_fields_add_group(request, group);

	purple_request_fields(data,
			N_("I'dle Mak'er"),
			_("Set Idle Time for All Accounts"),
			NULL,
			request,
			_("_Set"), G_CALLBACK(idle_all_action_ok),
			_("_Cancel"), NULL,
			NULL, NULL);
}

static void
purple_idle_unset_all_accounts_idle_time(G_GNUC_UNUSED GSimpleAction *action,
                                         G_GNUC_UNUSED GVariant *parameter,
                                         G_GNUC_UNUSED gpointer data)
{
	/* freeing the list here will cause segfaults if the user idles an account
	 * after the list is freed */
	g_list_foreach(idled_accts, (GFunc)set_idle_time, GINT_TO_POINTER(0));
	g_list_free(idled_accts);
	idled_accts = NULL;
}

/******************************************************************************
 * GPlugin Exports
 *****************************************************************************/
static GPluginPluginInfo *
idle_query(G_GNUC_UNUSED GError **error)
{
	GSimpleActionGroup *group = NULL;
	GActionEntry entries[] = {
		{
			.name = "set-account-idle-time",
			.activate = purple_idle_set_account_idle_time,
		}, {
			.name = "unset-account-idle-time",
			.activate = purple_idle_unset_account_idle_time,
		}, {
			.name = "set-all-accounts-idle-time",
			.activate = purple_idle_set_all_accounts_idle_time,
		}, {
			.name = "unset-all-accounts-idle-time",
			.activate = purple_idle_unset_all_accounts_idle_time,
		}
	};
	GMenu *menu = NULL;
	const gchar * const authors[] = {
		"Eric Warmenhoven <eric@warmenhoven.org>",
		NULL
	};

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries,
	                                G_N_ELEMENTS(entries), NULL);

	menu = g_menu_new();
	g_menu_append(menu, _("Set Account Idle Time"), "set-account-idle-time");
	g_menu_append(menu, _("Unset Account Idle Time"),
	              "unset-account-idle-time");
	g_menu_append(menu, _("Set Idle Time for All Accounts"),
	              "set-all-accounts-idle-time");
	g_menu_append(menu, _("Unset Idle Time for All Idled Accounts"),
	              "unset-all-accounts-idle-time");

	return purple_plugin_info_new(
		"id",           IDLE_PLUGIN_ID,
		/* This is a cultural reference.  Dy'er Mak'er is a song by Led Zeppelin.
		   If that doesn't translate well into your language, drop the 's before translating. */
		"name",         N_("I'dle Mak'er"),
		"version",      DISPLAY_VERSION,
		"category",     N_("Utility"),
		"summary",      N_("Allows you to hand-configure how long you've been idle"),
		"description",  N_("Allows you to hand-configure how long you've been idle"),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"action-group", group,
		"action-menu",  menu,
		NULL
	);
}

static gboolean
idle_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{
	purple_signal_connect(purple_connections_get_handle(), "signing-off",
						plugin,
						G_CALLBACK(signing_off_cb), NULL);

	return TRUE;
}

static gboolean
idle_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
            G_GNUC_UNUSED gboolean shutdown,
            G_GNUC_UNUSED GError **error)
{
	purple_idle_unset_all_accounts_idle_time(NULL, NULL, NULL);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(idle);
