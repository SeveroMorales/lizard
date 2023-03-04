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

#include "purpledemoprotocol.h"
#include "purpledemoprotocolactions.h"
#include "purpledemoresource.h"

/******************************************************************************
 * Connection failure action implementations
 *****************************************************************************/
#define REAPER_BUDDY_NAME ("Gary")
#define DEFAULT_REAP_TIME (5)  /* seconds */
#define FATAL_TICK_STR N_("Reaping connection in %d second...")
#define FATAL_TICK_PLURAL_STR N_("Reaping connection in %d seconds...")
#define FATAL_DISCONNECT_STR N_("%s reaped the connection")
#define TEMPORARY_TICK_STR N_("Pruning connection in %d second...")
#define TEMPORARY_TICK_PLURAL_STR N_("Pruning connection in %d seconds...")
#define TEMPORARY_DISCONNECT_STR N_("%s pruned the connection")

static gboolean
purple_demo_protocol_failure_tick(gpointer data,
                                  PurpleConnectionError error_code,
                                  const gchar *tick_str,
                                  const gchar *tick_plural_str,
                                  const gchar *disconnect_str)
{
	PurpleConnection *connection = PURPLE_CONNECTION(data);
	PurpleAccount *account = purple_connection_get_account(connection);
	gchar *message = NULL;
	gint timeout = 0;

	timeout = GPOINTER_TO_INT(g_object_steal_data(G_OBJECT(connection),
	                                              "reaping-time"));
	timeout--;
	if(timeout > 0) {
		g_object_set_data(G_OBJECT(connection), "reaping-time",
		                  GINT_TO_POINTER(timeout));

		message = g_strdup_printf(ngettext(tick_str, tick_plural_str, timeout),
		                          timeout);
		purple_protocol_got_user_status(account, REAPER_BUDDY_NAME,
		                                "available", "message", message, NULL);
		g_free(message);

		return G_SOURCE_CONTINUE;
	}

	message = g_strdup_printf(_(disconnect_str), REAPER_BUDDY_NAME);
	purple_connection_error(connection, error_code, message);
	g_free(message);

	return G_SOURCE_REMOVE;
}

static gboolean
purple_demo_protocol_fatal_failure_cb(gpointer data) {
	return purple_demo_protocol_failure_tick(data,
	                                         PURPLE_CONNECTION_ERROR_CUSTOM_FATAL,
	                                         FATAL_TICK_STR,
	                                         FATAL_TICK_PLURAL_STR,
	                                         FATAL_DISCONNECT_STR);
}

static gboolean
purple_demo_protocol_temporary_failure_cb(gpointer data) {
	return purple_demo_protocol_failure_tick(data,
	                                         PURPLE_CONNECTION_ERROR_CUSTOM_TEMPORARY,
	                                         TEMPORARY_TICK_STR,
	                                         TEMPORARY_TICK_PLURAL_STR,
	                                         TEMPORARY_DISCONNECT_STR);
}

static void
purple_demo_protocol_failure_action_activate(G_GNUC_UNUSED GSimpleAction *action,
                                             GVariant *parameter,
                                             const gchar *tick_str,
                                             const gchar *tick_plural_str,
                                             GSourceFunc cb)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;
	gchar *status = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	/* Do nothing if disconnected, or already in process of reaping. */
	if(!PURPLE_IS_CONNECTION(connection)) {
		return;
	}
	if(g_object_get_data(G_OBJECT(connection), "reaping-time")) {
		return;
	}

	purple_protocol_got_user_idle(account, REAPER_BUDDY_NAME, FALSE, 0);
	status = g_strdup_printf(
	        ngettext(tick_str, tick_plural_str, DEFAULT_REAP_TIME),
	        DEFAULT_REAP_TIME);
	purple_protocol_got_user_status(account, REAPER_BUDDY_NAME, "available",
	                                "message", status, NULL);
	g_free(status);

	g_object_set_data(G_OBJECT(connection), "reaping-time",
	                  GINT_TO_POINTER(DEFAULT_REAP_TIME));
	g_timeout_add_seconds(1, cb, connection);
}

static void
purple_demo_protocol_temporary_failure_action_activate(GSimpleAction *action,
                                                       GVariant *parameter,
                                                       G_GNUC_UNUSED gpointer data)
{
	purple_demo_protocol_failure_action_activate(action, parameter,
	                                             TEMPORARY_TICK_STR,
	                                             TEMPORARY_TICK_PLURAL_STR,
	                                             purple_demo_protocol_temporary_failure_cb);
}

static void
purple_demo_protocol_fatal_failure_action_activate(GSimpleAction *action,
                                                   GVariant *parameter,
                                                   G_GNUC_UNUSED gpointer data)
{
	purple_demo_protocol_failure_action_activate(action, parameter,
	                                             FATAL_TICK_STR,
	                                             FATAL_TICK_PLURAL_STR,
	                                             purple_demo_protocol_fatal_failure_cb);
}

/******************************************************************************
 * Request API action implementations
 *****************************************************************************/

static void
purple_demo_protocol_request_input_ok_cb(G_GNUC_UNUSED gpointer data,
                                         const char *value)
{
	g_message(_("Successfully requested input from UI: %s"), value);
}

static void
purple_demo_protocol_request_input_cancel_cb(G_GNUC_UNUSED gpointer data,
                                             G_GNUC_UNUSED const char *value)
{
	g_message(_("UI cancelled input request"));
}

static void
purple_demo_protocol_request_input_activate(G_GNUC_UNUSED GSimpleAction *action,
                                            GVariant *parameter,
                                            G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;
	static int form = 0;
	gboolean multiline = FALSE, masked = FALSE;
	char *secondary = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	/* Alternate through all four combinations of {masked, multiline}. */
	masked = form % 2 == 1;
	multiline = (form / 2) % 2 == 1;
	form++;
	secondary = g_strdup_printf(_("The input will be %s %s."),
	                            masked ? "masked" : "unmasked",
	                            multiline ? "multiple lines" : "single line");

	purple_request_input(connection, _("Request Input Demo"),
	                     _("Please input some text…"), secondary, _("default"),
	                     multiline, masked, NULL,
	                     _("OK"),
	                     G_CALLBACK(purple_demo_protocol_request_input_ok_cb),
	                     _("Cancel"),
	                     G_CALLBACK(purple_demo_protocol_request_input_cancel_cb),
	                     purple_request_cpar_from_connection(connection), NULL);

	g_free(secondary);
}

static void
purple_demo_protocol_request_choice_ok_cb(G_GNUC_UNUSED gpointer data,
                                          gpointer value)
{
	const char *text = value;

	g_message(_("Successfully requested a choice from UI: %s"), text);
}

static void
purple_demo_protocol_request_choice_cancel_cb(G_GNUC_UNUSED gpointer data,
                                              G_GNUC_UNUSED gpointer value)
{
	g_message(_("UI cancelled choice request"));
}

static void
purple_demo_protocol_request_choice_activate(G_GNUC_UNUSED GSimpleAction *action,
                                             GVariant *parameter,
                                             G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	purple_request_choice(connection, _("Request Choice Demo"),
	                      _("Please pick an option…"), NULL, _("foo"),
	                      _("OK"),
	                      G_CALLBACK(purple_demo_protocol_request_choice_ok_cb),
	                      _("Cancel"),
	                      G_CALLBACK(purple_demo_protocol_request_choice_cancel_cb),
	                      purple_request_cpar_from_connection(connection),
	                      NULL, _("foo"), "foo", _("bar"), "bar",
	                      _("baz"), "baz", NULL);
}

static void
purple_demo_protocol_request_action_cb(G_GNUC_UNUSED gpointer data, int action)
{
	g_message(_("Successfully requested an action from the UI: %d"), action);
}

static void
purple_demo_protocol_request_action_activate(G_GNUC_UNUSED GSimpleAction *action,
                                             GVariant *parameter,
                                             G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	purple_request_action(connection, _("Request Action Demo"),
	                      _("Please choose an action…"), NULL, 1,
	                      purple_request_cpar_from_connection(connection),
	                      NULL, 3,
	                      _("foo"), purple_demo_protocol_request_action_cb,
	                      _("bar"), purple_demo_protocol_request_action_cb,
	                      _("baz"), purple_demo_protocol_request_action_cb);
}

typedef struct {
	gint id;
	gpointer ui_handle;
} PurpleDemoProtocolWaitData;

static gboolean
purple_demo_protocol_request_wait_pulse_cb(gpointer data) {
	PurpleDemoProtocolWaitData *wait = data;

	purple_request_wait_pulse(wait->ui_handle);

	return G_SOURCE_CONTINUE;
}

static void
purple_demo_protocol_request_wait_cancel_cb(G_GNUC_UNUSED gpointer data) {
	g_message(_("UI cancelled wait request"));
}

static void
purple_demo_protocol_request_wait_close_cb(gpointer data) {
	PurpleDemoProtocolWaitData *wait = data;

	g_source_remove(wait->id);
	g_free(wait);
}

static void
purple_demo_protocol_request_wait_activate(G_GNUC_UNUSED GSimpleAction *action,
                                           GVariant *parameter,
                                           G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;
	PurpleDemoProtocolWaitData *wait = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	wait = g_new0(PurpleDemoProtocolWaitData, 1);

	wait->ui_handle = purple_request_wait(connection, _("Request Wait Demo"),
	                                      _("Please wait…"), NULL, TRUE,
	                                      purple_demo_protocol_request_wait_cancel_cb,
	                                      purple_request_cpar_from_connection(connection),
	                                      wait);

	wait->id = g_timeout_add(250, purple_demo_protocol_request_wait_pulse_cb,
	                         wait);

	purple_request_add_close_notify(wait->ui_handle,
	                                purple_demo_protocol_request_wait_close_cb,
	                                wait);
}

static void
purple_demo_protocol_request_fields_ok_cb(G_GNUC_UNUSED gpointer data,
                                          PurpleRequestFields *fields)
{
	PurpleAccount *account = NULL;
	PurpleRequestField *field = NULL;
	GList *list = NULL;
	const char *tmp = NULL;
	GString *info = NULL;

	info = g_string_new(_("Basic group:\n"));

	g_string_append_printf(info, _("\tString: %s\n"),
	                       purple_request_fields_get_string(fields, "string"));
	g_string_append_printf(info, _("\tMultiline string: %s\n"),
	                       purple_request_fields_get_string(fields,
	                                                        "multiline-string"));
	g_string_append_printf(info, _("\tMasked string: %s\n"),
	                       purple_request_fields_get_string(fields,
	                                                        "masked-string"));
	g_string_append_printf(info, _("\tAlphanumeric string: %s\n"),
	                       purple_request_fields_get_string(fields,
	                                                        "alphanumeric"));
	g_string_append_printf(info, _("\tEmail string: %s\n"),
	                       purple_request_fields_get_string(fields, "email"));
	g_string_append_printf(info, _("\tInteger: %d\n"),
	                       purple_request_fields_get_integer(fields, "int"));
	g_string_append_printf(info, _("\tBoolean: %s\n"),
	                       purple_request_fields_get_bool(fields, "bool") ?
	                       _("TRUE") : _("FALSE"));

	g_string_append(info, _("Multiple-choice group:\n"));

	tmp = (const char *)purple_request_fields_get_choice(fields, "choice");
	g_string_append_printf(info, _("\tChoice: %s\n"), tmp);

	field = purple_request_fields_get_field(fields, "list");
	list = purple_request_field_list_get_selected(field);
	if(list != NULL) {
		tmp = (const char *)list->data;
	} else {
		tmp = _("(unset)");
	}
	g_string_append_printf(info, _("\tList: %s\n"), tmp);

	field = purple_request_fields_get_field(fields, "multilist");
	list = purple_request_field_list_get_selected(field);
	g_string_append(info, _("\tMulti-list: ["));
	while(list != NULL) {
		tmp = (const char *)list->data;
		g_string_append_printf(info, "%s%s", tmp,
		                       list->next != NULL ? ", " : "");
		list = list->next;
	}
	g_string_append(info, _("]\n"));

	g_string_append(info, _("Special group:\n"));

	account = purple_request_fields_get_account(fields, "account");
	if(PURPLE_IS_ACCOUNT(account)) {
		tmp = purple_contact_info_get_name_for_display(PURPLE_CONTACT_INFO(account));
	} else {
		tmp = _("(unset)");
	}
	g_string_append_printf(info, _("\tAccount: %s\n"), tmp);

	g_message(_("Successfully requested fields:\n%s"), info->str);

	g_string_free(info, TRUE);
}

static void
purple_demo_protocol_request_fields_cancel_cb(G_GNUC_UNUSED gpointer data,
                                              G_GNUC_UNUSED PurpleRequestFields *fields)
{
	g_message(_("UI cancelled field request"));
}

static void
purple_demo_protocol_request_fields_activate(G_GNUC_UNUSED GSimpleAction *action,
                                             GVariant *parameter,
                                             G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;
	PurpleRequestFields *fields = NULL;
	PurpleRequestFieldGroup *group = NULL;
	PurpleRequestField *field = NULL;
	GBytes *icon = NULL;
	gconstpointer icon_data = NULL;
	gsize icon_len = 0;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	fields = purple_request_fields_new();

	/* This group will contain basic fields. */
	group = purple_request_field_group_new(_("Basic"));
	purple_request_fields_add_group(fields, group);

	field = purple_request_field_label_new("basic-label",
	                                       _("This group contains basic fields"));
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_string_new("string", _("A string"),
	                                        _("default"), FALSE);
	purple_request_field_group_add_field(group, field);
	field = purple_request_field_string_new("multiline-string",
	                                        _("A multiline string"),
	                                        _("default"), TRUE);
	purple_request_field_group_add_field(group, field);
	field = purple_request_field_string_new("masked-string",
	                                        _("A masked string"), _("default"),
	                                        FALSE);
	purple_request_field_string_set_masked(field, TRUE);
	purple_request_field_group_add_field(group, field);
	field = purple_request_field_string_new("alphanumeric",
	                                        _("An alphanumeric string"),
	                                        _("default"), FALSE);
	purple_request_field_set_validator(field,
	                                   purple_request_field_alphanumeric_validator,
	                                   NULL);
	purple_request_field_group_add_field(group, field);
	field = purple_request_field_string_new("email", _("An email"),
	                                        _("me@example.com"), FALSE);
	purple_request_field_set_validator(field,
	                                   purple_request_field_email_validator,
	                                   NULL);
	purple_request_field_group_add_field(group, field);
	field = purple_request_field_int_new("int", _("An integer"), 123, -42, 1337);
	purple_request_field_group_add_field(group, field);
	field = purple_request_field_bool_new("bool", _("A boolean"), FALSE);
	purple_request_field_group_add_field(group, field);

	/* This group will contain fields with multiple options. */
	group = purple_request_field_group_new(_("Multiple"));
	purple_request_fields_add_group(fields, group);

	field = purple_request_field_label_new("multiple-label",
	                                       _("This group contains fields with multiple options"));
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_choice_new("choice", _("A choice"), "foo");
	purple_request_field_choice_add(field, _("foo"), "foo");
	purple_request_field_choice_add(field, _("bar"), "bar");
	purple_request_field_choice_add(field, _("baz"), "baz");
	purple_request_field_choice_add(field, _("quux"), "quux");
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_list_new("list", _("A list"));
	purple_request_field_list_add_icon(field, _("foo"), NULL, "foo");
	purple_request_field_list_add_icon(field, _("bar"), NULL, "bar");
	purple_request_field_list_add_icon(field, _("baz"), NULL, "baz");
	purple_request_field_list_add_icon(field, _("quux"), NULL, "quux");
	purple_request_field_group_add_field(group, field);

	field = purple_request_field_list_new("multilist", _("A multi-select list"));
	purple_request_field_list_set_multi_select(field, TRUE);
	purple_request_field_list_add_icon(field, _("foo"), NULL, "foo");
	purple_request_field_list_add_icon(field, _("bar"), NULL, "bar");
	purple_request_field_list_add_icon(field, _("baz"), NULL, "baz");
	purple_request_field_list_add_icon(field, _("quux"), NULL, "quux");
	purple_request_field_group_add_field(group, field);

	/* This group will contain specialized fields. */
	group = purple_request_field_group_new(_("Special"));
	purple_request_fields_add_group(fields, group);

	field = purple_request_field_label_new("special-label",
	                                       _("This group contains specialized fields"));
	purple_request_field_group_add_field(group, field);

	icon = g_resource_lookup_data(purple_demo_get_resource(),
	                              "/im/pidgin/purple/demo/icons/scalable/apps/im-purple-demo.svg",
	                              G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
	icon_data = g_bytes_get_data(icon, &icon_len);
	field = purple_request_field_image_new("image", _("An image"),
	                                       icon_data, icon_len);
	purple_request_field_group_add_field(group, field);
	g_bytes_unref(icon);

	field = purple_request_field_account_new("account", _("An account"),
	                                         account);
	purple_request_field_group_add_field(group, field);

	purple_request_fields(connection, _("Request Fields Demo"),
	                      _("Please fill out these fields…"), NULL, fields,
	                      _("OK"),
	                      G_CALLBACK(purple_demo_protocol_request_fields_ok_cb),
	                      _("Cancel"),
	                      G_CALLBACK(purple_demo_protocol_request_fields_cancel_cb),
	                      purple_request_cpar_from_connection(connection),
	                      NULL);
}

static void
purple_demo_protocol_request_path_ok_cb(gpointer data, const char *filename) {
	const char *type = data;

	g_message(_("Successfully requested %s from UI: %s"), type, filename);
}

static void
purple_demo_protocol_request_path_cancel_cb(gpointer data) {
	const char *type = data;

	g_message(_("UI cancelled %s request"), type);
}

static void
purple_demo_protocol_request_file_activate(G_GNUC_UNUSED GSimpleAction *action,
                                           GVariant *parameter,
                                           G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	purple_request_file(connection, _("Request File Demo"),
	                    "example.txt", FALSE,
	                    G_CALLBACK(purple_demo_protocol_request_path_ok_cb),
	                    G_CALLBACK(purple_demo_protocol_request_path_cancel_cb),
	                    purple_request_cpar_from_connection(connection),
	                    "file");
}

static void
purple_demo_protocol_request_folder_activate(G_GNUC_UNUSED GSimpleAction *action,
                                             GVariant *parameter,
                                             G_GNUC_UNUSED gpointer data)
{
	PurpleConnection *connection = NULL;
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("Demo failure action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);

	purple_request_folder(connection, _("Request Folder Demo"), NULL,
	                      G_CALLBACK(purple_demo_protocol_request_path_ok_cb),
	                      G_CALLBACK(purple_demo_protocol_request_path_cancel_cb),
	                      purple_request_cpar_from_connection(connection),
	                      "folder");
}

/******************************************************************************
 * Contact action implementations
 *****************************************************************************/
static const gchar *contacts[] = {"Alice", "Bob", "Carlos", "Erin" };

static void
purple_demo_protocol_remote_add(G_GNUC_UNUSED GSimpleAction *action,
                                GVariant *parameter,
                                G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = NULL;
	PurpleAccountManager *account_manager = NULL;
	PurpleAddContactRequest *request = NULL;
	PurpleNotification *notification = NULL;
	PurpleNotificationManager *notification_manager = NULL;
	const gchar *account_id = NULL;
	static guint counter = 0;

	account_id = g_variant_get_string(parameter, NULL);
	account_manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(account_manager, account_id);

	request = purple_add_contact_request_new(account, contacts[counter]);
	notification = purple_notification_new_from_add_contact_request(request);

	notification_manager = purple_notification_manager_get_default();
	purple_notification_manager_add(notification_manager, notification);

	counter++;
	if(counter >= G_N_ELEMENTS(contacts)) {
		counter = 0;
	}
}

/******************************************************************************
 * PurpleProtocolActions Implementation
 *****************************************************************************/
static const gchar *
purple_demo_protocol_get_prefix(G_GNUC_UNUSED PurpleProtocolActions *actions) {
	return "prpl-demo";
}

static GActionGroup *
purple_demo_protocol_get_action_group(G_GNUC_UNUSED PurpleProtocolActions *actions,
                                      G_GNUC_UNUSED PurpleConnection *connection)
{
	GSimpleActionGroup *group = NULL;
	GActionEntry entries[] = {
		{
			.name = "temporary-failure",
			.activate = purple_demo_protocol_temporary_failure_action_activate,
			.parameter_type = "s",
		}, {
			.name = "fatal-failure",
			.activate = purple_demo_protocol_fatal_failure_action_activate,
			.parameter_type = "s",
		}, {
			.name = "remote-add",
			.activate = purple_demo_protocol_remote_add,
			.parameter_type = "s",
		}, {
			.name = "request-input",
			.activate = purple_demo_protocol_request_input_activate,
			.parameter_type = "s",
		}, {
			.name = "request-choice",
			.activate = purple_demo_protocol_request_choice_activate,
			.parameter_type = "s",
		}, {
			.name = "request-action",
			.activate = purple_demo_protocol_request_action_activate,
			.parameter_type = "s",
		}, {
			.name = "request-wait",
			.activate = purple_demo_protocol_request_wait_activate,
			.parameter_type = "s",
		}, {
			.name = "request-fields",
			.activate = purple_demo_protocol_request_fields_activate,
			.parameter_type = "s",
		}, {
			.name = "request-file",
			.activate = purple_demo_protocol_request_file_activate,
			.parameter_type = "s",
		}, {
			.name = "request-folder",
			.activate = purple_demo_protocol_request_folder_activate,
			.parameter_type = "s",
		}
	};
	gsize nentries = G_N_ELEMENTS(entries);

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries, nentries,
	                                NULL);

	return G_ACTION_GROUP(group);
}

static GMenu *
purple_demo_protocol_get_menu(G_GNUC_UNUSED PurpleProtocolActions *actions,
                              G_GNUC_UNUSED PurpleConnection *connection)
{
	GMenu *menu = NULL;
	GMenu *submenu = NULL;
	GMenuItem *item = NULL;

	menu = g_menu_new();

	item = g_menu_item_new(_("Trigger temporary connection failure..."),
	                       "prpl-demo.temporary-failure");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(menu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Trigger fatal connection failure..."),
	                       "prpl-demo.fatal-failure");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(menu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Trigger a contact adding you"),
	                       "prpl-demo.remote-add");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(menu, item);
	g_object_unref(item);

	submenu = g_menu_new();

	item = g_menu_item_new(_("Input"), "prpl-demo.request-input");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Choice"), "prpl-demo.request-choice");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Action"), "prpl-demo.request-action");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Wait"), "prpl-demo.request-wait");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Fields"), "prpl-demo.request-fields");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("File"), "prpl-demo.request-file");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	item = g_menu_item_new(_("Folder"), "prpl-demo.request-folder");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(submenu, item);
	g_object_unref(item);

	g_menu_append_submenu(menu, _("Trigger requests"), G_MENU_MODEL(submenu));
	g_object_unref(submenu);

	return menu;
}

void
purple_demo_protocol_actions_init(PurpleProtocolActionsInterface *iface) {
	iface->get_prefix = purple_demo_protocol_get_prefix;
	iface->get_action_group = purple_demo_protocol_get_action_group;
	iface->get_menu = purple_demo_protocol_get_menu;
}
