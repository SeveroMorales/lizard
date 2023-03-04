/*
 * Integration with Unity's messaging menu and launcher
 * Copyright (C) 2013 Ankit Vani <a@nevitus.org>
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
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <purple.h>

#include <pidgin.h>

#include <talkatu.h>

#include <unity.h>
#include <messaging-menu.h>

#define UNITY_PLUGIN_ID "gtk-unity-integration"

static MessagingMenuApp *mmapp = NULL;
static UnityLauncherEntry *launcher = NULL;
static guint n_sources = 0;
static gint launcher_count;
static gint messaging_menu_text;
static gboolean alert_chat_nick = TRUE;

enum {
	LAUNCHER_COUNT_DISABLE,
	LAUNCHER_COUNT_MESSAGES,
	LAUNCHER_COUNT_SOURCES,
};

enum {
	MESSAGING_MENU_COUNT,
	MESSAGING_MENU_TIME,
};

static int attach_signals(PurpleConversation *conv);
static void detach_signals(PurpleConversation *conv);

static void
update_launcher(void)
{
	guint count = 0;
	GList *convs = NULL;
	g_return_if_fail(launcher != NULL);
	if (launcher_count == LAUNCHER_COUNT_DISABLE)
		return;

	if (launcher_count == LAUNCHER_COUNT_MESSAGES) {
		PurpleConversationManager *manager = NULL;

		manager = purple_conversation_manager_get_default();
		convs = purple_conversation_manager_get_all(manager);

		while(convs != NULL) {
			PurpleConversation *conv = convs->data;
			count += GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv),
					"unity-message-count"));
			convs = g_list_delete_link(convs, convs);
		}
	} else {
		count = n_sources;
	}

	if (launcher != NULL) {
		if (count > 0)
			unity_launcher_entry_set_count_visible(launcher, TRUE);
		else
			unity_launcher_entry_set_count_visible(launcher, FALSE);
		unity_launcher_entry_set_count(launcher, count);
	}
}

static gchar *
conversation_id(PurpleConversation *conv)
{
	PurpleAccount *account = purple_conversation_get_account(conv);
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	const char *type = "misc";

	if(PURPLE_IS_IM_CONVERSATION(conv)) {
		type = "im";
	} else if(PURPLE_IS_CHAT_CONVERSATION(conv)) {
		type = "chat";
	}

	return g_strdup_printf("%s:%s:%s:%s",
	                       type,
	                       purple_conversation_get_name(conv),
	                       purple_contact_info_get_username(info),
	                       purple_account_get_protocol_id(account));
}

static void
messaging_menu_add_conversation(PurpleConversation *conv, gint count)
{
	gchar *id;
	g_return_if_fail(count > 0);
	id = conversation_id(conv);

	/* GBytesIcon may be useful for messaging menu source icons using buddy
	   icon data for IMs */
	if (!messaging_menu_app_has_source(mmapp, id))
		messaging_menu_app_append_source(mmapp, id, NULL,
				purple_conversation_get_title(conv));

	if (messaging_menu_text == MESSAGING_MENU_TIME)
		messaging_menu_app_set_source_time(mmapp, id, g_get_real_time());
	else if (messaging_menu_text == MESSAGING_MENU_COUNT)
		messaging_menu_app_set_source_count(mmapp, id, count);
	messaging_menu_app_draw_attention(mmapp, id);

	g_free(id);
}

static void
messaging_menu_remove_conversation(PurpleConversation *conv)
{
	gchar *id = conversation_id(conv);
	if (messaging_menu_app_has_source(mmapp, id))
		messaging_menu_app_remove_source(mmapp, id);
	g_free(id);
}

static void
refill_messaging_menu(void)
{
	PurpleConversationManager *manager = NULL;
	GList *convs;

	manager = purple_conversation_manager_get_default();
	convs = purple_conversation_manager_get_all(manager);
	while(convs != NULL) {
		PurpleConversation *conv = convs->data;
		messaging_menu_add_conversation(conv,
			GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv),
			"unity-message-count")));
		convs = g_list_delete_link(convs, convs);
	}
}

static int
alert(PurpleConversation *conv)
{
	gint count;
	PidginConversation *gtkconv = NULL;
	PidginDisplayWindow *displaywin = NULL;
	GtkRoot *root = NULL;
	GtkWidget *win = NULL;

	if (conv == NULL || PIDGIN_CONVERSATION(conv) == NULL)
		return 0;

	gtkconv = PIDGIN_CONVERSATION(conv);
	root = gtk_widget_get_root(gtkconv->tab_cont);
	win = GTK_WIDGET(root);
	displaywin = PIDGIN_DISPLAY_WINDOW(win);

	if (!gtk_widget_has_focus(win) ||
		!pidgin_display_window_conversation_is_selected(displaywin, conv))
	{
		count = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv),
				"unity-message-count"));
		if (!count++)
			++n_sources;

		g_object_set_data(G_OBJECT(conv), "unity-message-count",
				GINT_TO_POINTER(count));
		messaging_menu_add_conversation(conv, count);
		update_launcher();
	}

	return 0;
}

static void
unalert(PurpleConversation *conv)
{
	if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "unity-message-count")) > 0)
		--n_sources;

	g_object_set_data(G_OBJECT(conv), "unity-message-count",
			GINT_TO_POINTER(0));
	messaging_menu_remove_conversation(conv);
	update_launcher();
}

static int
unalert_cb(G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gpointer data,
           PurpleConversation *conv)
{
	unalert(conv);
	return 0;
}

static gboolean
message_displayed_cb(PurpleConversation *conv, PurpleMessage *msg,
                     G_GNUC_UNUSED gpointer data)
{
	PurpleMessageFlags flags = purple_message_get_flags(msg);

	if ((PURPLE_IS_CHAT_CONVERSATION(conv) && alert_chat_nick &&
			!(flags & PURPLE_MESSAGE_NICK)))
		return FALSE;

	if ((flags & PURPLE_MESSAGE_RECV) && !(flags & PURPLE_MESSAGE_DELAYED))
		alert(conv);

	return FALSE;
}

static void
im_sent_im(PurpleAccount *account, PurpleMessage *msg,
           G_GNUC_UNUSED gpointer data)
{
	PurpleConversation *im = NULL;
	PurpleConversationManager *manager = NULL;

	manager = purple_conversation_manager_get_default();

	im = purple_conversation_manager_find_im(manager, account,
	                                         purple_message_get_recipient(msg));

	unalert(im);
}

static void
chat_sent_im(PurpleAccount *account, G_GNUC_UNUSED PurpleMessage *msg, int id)
{
	PurpleConversation *chat = NULL;
	PurpleConversationManager *manager = NULL;

	manager = purple_conversation_manager_get_default();

	chat = purple_conversation_manager_find_chat_by_id(manager, account, id);

	unalert(chat);
}

static void
conv_created(PurpleConversation *conv)
{
	g_object_set_data(G_OBJECT(conv), "unity-message-count",
			GINT_TO_POINTER(0));
	attach_signals(conv);
}

static void
deleting_conv(PurpleConversation *conv)
{
	detach_signals(conv);
	unalert(conv);
}

static void
message_source_activated(G_GNUC_UNUSED MessagingMenuApp *app,
                         const gchar *id,
                         G_GNUC_UNUSED gpointer user_data)
{
	gchar **sections = g_strsplit(id, ":", 0);
	PurpleConversation *conv = NULL;
	PurpleConversationManager *conversation_manager = NULL;
	PurpleAccount *account;
	PurpleAccountManager *account_manager = NULL;

	char *type     = sections[0];
	char *cname    = sections[1];
	char *aname    = sections[2];
	char *protocol = sections[3];

	conversation_manager = purple_conversation_manager_get_default();

	account_manager = purple_account_manager_get_default();
	account = purple_account_manager_find(account_manager, aname, protocol);

	if (g_strcmp0(type, "im") == 0) {
		conv = purple_conversation_manager_find_im(conversation_manager,
		                                           account, cname);
	} else if (g_strcmp0(type, "chat") == 0) {
		conv = purple_conversation_manager_find_chat(conversation_manager,
		                                             account, cname);
	} else {
		conv = purple_conversation_manager_find(conversation_manager,
		                                        account, cname);
	}

	if (conv) {
		GtkRoot *root = NULL;
		GtkWidget *win = NULL;
		PidginDisplayWindow *displaywin = NULL;

		root = gtk_widget_get_root(PIDGIN_CONVERSATION(conv)->tab_cont);
		win = GTK_WIDGET(root);
		displaywin = PIDGIN_DISPLAY_WINDOW(win);

		unalert(conv);

		pidgin_display_window_select(displaywin, conv);

		gtk_root_set_focus(root, PIDGIN_CONVERSATION(conv)->entry);
	}
	g_strfreev (sections);
}

static PurpleSavedStatus *
create_transient_status(PurpleStatusPrimitive primitive,
                        PurpleStatusType *status_type)
{
	PurpleSavedStatus *saved_status = purple_savedstatus_new(NULL, primitive);

	if(status_type != NULL) {
		PurpleAccountManager *manager = NULL;
		GList *active_accts = NULL;

		manager = purple_account_manager_get_default();
		active_accts = purple_account_manager_get_enabled(manager);

		while(active_accts != NULL) {
			PurpleAccount *account = PURPLE_ACCOUNT(active_accts->data);

			purple_savedstatus_set_substatus(saved_status, account,
			                                 status_type, NULL);

			active_accts = g_list_delete_link(active_accts, active_accts);
		}
	}

	return saved_status;
}

static void
status_changed_cb(PurpleSavedStatus *saved_status)
{
	MessagingMenuStatus status = MESSAGING_MENU_STATUS_AVAILABLE;

	switch (purple_savedstatus_get_primitive_type(saved_status)) {
	case PURPLE_STATUS_AVAILABLE:
	case PURPLE_STATUS_MOOD:
	case PURPLE_STATUS_TUNE:
	case PURPLE_STATUS_UNSET:
		status = MESSAGING_MENU_STATUS_AVAILABLE;
		break;

	case PURPLE_STATUS_AWAY:
	case PURPLE_STATUS_EXTENDED_AWAY:
		status = MESSAGING_MENU_STATUS_AWAY;
		break;

	case PURPLE_STATUS_INVISIBLE:
		status = MESSAGING_MENU_STATUS_INVISIBLE;
		break;

	case PURPLE_STATUS_MOBILE:
	case PURPLE_STATUS_OFFLINE:
		status = MESSAGING_MENU_STATUS_OFFLINE;
		break;

	case PURPLE_STATUS_UNAVAILABLE:
		status = MESSAGING_MENU_STATUS_BUSY;
		break;

	default:
		g_assert_not_reached();
	}
	messaging_menu_app_set_status(mmapp, status);
}

static void
messaging_menu_status_changed(G_GNUC_UNUSED MessagingMenuApp *mmapp,
                              MessagingMenuStatus mm_status,
                              G_GNUC_UNUSED gpointer user_data)
{
	PurpleSavedStatus *saved_status;
	PurpleStatusPrimitive primitive = PURPLE_STATUS_UNSET;

	switch (mm_status) {
	case MESSAGING_MENU_STATUS_AVAILABLE:
		primitive = PURPLE_STATUS_AVAILABLE;
		break;

	case MESSAGING_MENU_STATUS_AWAY:
		primitive = PURPLE_STATUS_AWAY;
		break;

	case MESSAGING_MENU_STATUS_BUSY:
		primitive = PURPLE_STATUS_UNAVAILABLE;
		break;

	case MESSAGING_MENU_STATUS_INVISIBLE:
		primitive = PURPLE_STATUS_INVISIBLE;
		break;

	case MESSAGING_MENU_STATUS_OFFLINE:
		primitive = PURPLE_STATUS_OFFLINE;
		break;

	default:
		g_assert_not_reached();
	}

	saved_status = purple_savedstatus_find_transient_by_type_and_message(primitive, NULL);
	if (saved_status == NULL)
		saved_status = create_transient_status(primitive, NULL);
	purple_savedstatus_activate(saved_status);
}

static void
alert_config_cb(GtkWidget *widget, G_GNUC_UNUSED gpointer data)
{
	gboolean on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	purple_prefs_set_bool("/plugins/gtk/unity/alert_chat_nick", on);
	alert_chat_nick = on;
}

static void
launcher_config_cb(GtkWidget *widget, gpointer data)
{
	gint option = GPOINTER_TO_INT(data);
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		return;

	purple_prefs_set_int("/plugins/gtk/unity/launcher_count", option);
	launcher_count = option;
	if (option == LAUNCHER_COUNT_DISABLE)
		unity_launcher_entry_set_count_visible(launcher, FALSE);
	else
		update_launcher();
}

static void
messaging_menu_config_cb(GtkWidget *widget, gpointer data)
{
	gint option = GPOINTER_TO_INT(data);
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		return;

	purple_prefs_set_int("/plugins/gtk/unity/messaging_menu_text", option);
	messaging_menu_text = option;
	refill_messaging_menu();
}

static int
attach_signals(PurpleConversation *conv)
{
	PidginConversation *gtkconv = NULL;
	guint id;

	gtkconv = PIDGIN_CONVERSATION(conv);
	if (!gtkconv)
		return 0;

	id = g_signal_connect(G_OBJECT(gtkconv->entry), "focus-in-event",
			G_CALLBACK(unalert_cb), conv);
	g_object_set_data(G_OBJECT(conv), "unity-entry-signal", GUINT_TO_POINTER(id));

	id = g_signal_connect(G_OBJECT(gtkconv->history), "focus-in-event",
			G_CALLBACK(unalert_cb), conv);
	g_object_set_data(G_OBJECT(conv), "unity-history-signal", GUINT_TO_POINTER(id));

	return 0;
}

static void
detach_signals(PurpleConversation *conv)
{
	PidginConversation *gtkconv = NULL;
	guint id;
	gtkconv = PIDGIN_CONVERSATION(conv);
	if (!gtkconv)
		return;

	id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "unity-history-signal"));
	g_signal_handler_disconnect(gtkconv->history, id);

	id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "unity-entry-signal"));
	g_signal_handler_disconnect(gtkconv->entry, id);

	g_object_set_data(G_OBJECT(conv), "unity-message-count",
			GINT_TO_POINTER(0));
}

static GtkWidget *
get_config_frame(G_GNUC_UNUSED PurplePlugin *plugin)
{
	GtkWidget *ret = NULL, *frame = NULL;
	GtkWidget *vbox = NULL, *toggle = NULL, *group = NULL;

	ret = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);

	/* Alerts */

	frame = pidgin_make_frame(ret, _("Chatroom alerts"));
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_append(GTK_BOX(frame), vbox);

	toggle = gtk_check_button_new_with_mnemonic(_("Chatroom message alerts _only where someone says your username"));
	gtk_box_append(GTK_BOX(vbox), toggle);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(toggle),
			purple_prefs_get_bool("/plugins/gtk/unity/alert_chat_nick"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
			G_CALLBACK(alert_config_cb), NULL);

	/* Launcher integration */

	frame = pidgin_make_frame(ret, _("Launcher Icon"));
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_append(GTK_BOX(frame), vbox);

	toggle = gtk_check_button_new_with_mnemonic(_("_Disable launcher integration"));
	group = toggle;
	gtk_box_append(GTK_BOX(vbox), toggle);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(toggle),
		purple_prefs_get_int("/plugins/gtk/unity/launcher_count") == LAUNCHER_COUNT_DISABLE);
	g_signal_connect(G_OBJECT(toggle), "toggled",
			G_CALLBACK(launcher_config_cb), GUINT_TO_POINTER(LAUNCHER_COUNT_DISABLE));

	toggle = gtk_check_button_new_with_mnemonic(
			_("Show number of unread _messages on launcher icon"));
	gtk_check_button_set_group(GTK_CHECK_BUTTON(toggle),
	                           GTK_CHECK_BUTTON(group));
	gtk_box_append(GTK_BOX(vbox), toggle);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(toggle),
		purple_prefs_get_int("/plugins/gtk/unity/launcher_count") == LAUNCHER_COUNT_MESSAGES);
	g_signal_connect(G_OBJECT(toggle), "toggled",
			G_CALLBACK(launcher_config_cb), GUINT_TO_POINTER(LAUNCHER_COUNT_MESSAGES));

	toggle = gtk_check_button_new_with_mnemonic(
			_("Show number of unread co_nversations on launcher icon"));
	gtk_check_button_set_group(GTK_CHECK_BUTTON(toggle),
	                           GTK_CHECK_BUTTON(group));
	gtk_box_append(GTK_BOX(vbox), toggle);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(toggle),
		purple_prefs_get_int("/plugins/gtk/unity/launcher_count") == LAUNCHER_COUNT_SOURCES);
	g_signal_connect(G_OBJECT(toggle), "toggled",
			G_CALLBACK(launcher_config_cb), GUINT_TO_POINTER(LAUNCHER_COUNT_SOURCES));

	/* Messaging menu integration */

	frame = pidgin_make_frame(ret, _("Messaging Menu"));
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_append(GTK_BOX(frame), vbox);

	toggle = gtk_check_button_new_with_mnemonic(
			_("Show number of _unread messages for conversations in messaging menu"));
	group = toggle;
	gtk_box_append(GTK_BOX(vbox), toggle);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(toggle),
		purple_prefs_get_int("/plugins/gtk/unity/messaging_menu_text") == MESSAGING_MENU_COUNT);
	g_signal_connect(G_OBJECT(toggle), "toggled",
			G_CALLBACK(messaging_menu_config_cb), GUINT_TO_POINTER(MESSAGING_MENU_COUNT));

	toggle = gtk_check_button_new_with_mnemonic(
			_("Show _elapsed time for unread conversations in messaging menu"));
	gtk_check_button_set_group(GTK_CHECK_BUTTON(toggle),
	                           GTK_CHECK_BUTTON(group));
	gtk_box_append(GTK_BOX(vbox), toggle);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(toggle),
		purple_prefs_get_int("/plugins/gtk/unity/messaging_menu_text") == MESSAGING_MENU_TIME);
	g_signal_connect(G_OBJECT(toggle), "toggled",
			G_CALLBACK(messaging_menu_config_cb), GUINT_TO_POINTER(MESSAGING_MENU_TIME));

	return ret;
}

static GPluginPluginInfo *
unity_query(G_GNUC_UNUSED GError **error)
{
	const gchar * const authors[] = {
		"Ankit Vani <a@nevitus.org>",
		NULL
	};

	return pidgin_plugin_info_new(
		"id",                   UNITY_PLUGIN_ID,
		"name",                 N_("Unity Integration"),
		"version",              DISPLAY_VERSION,
		"category",             N_("Notification"),
		"summary",              N_("Provides integration with Unity."),
		"description",          N_("Provides integration with Unity's "
		                           "messaging menu and launcher."),
		"authors",              authors,
		"website",              PURPLE_WEBSITE,
		"abi-version",          PURPLE_ABI_VERSION,
		"gtk-config-frame-cb",  get_config_frame,
		NULL
	);
}

static gboolean
unity_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error) {
	GList *convs = NULL;
	PurpleConversationManager *manager = NULL;
	PurpleSavedStatus *saved_status;
	void *conv_handle = purple_conversations_get_handle();
	void *gtk_conv_handle = pidgin_conversations_get_handle();
	void *savedstat_handle = purple_savedstatuses_get_handle();

	purple_prefs_add_none("/plugins/gtk");
	purple_prefs_add_none("/plugins/gtk/unity");
	purple_prefs_add_int("/plugins/gtk/unity/launcher_count", LAUNCHER_COUNT_SOURCES);
	purple_prefs_add_int("/plugins/gtk/unity/messaging_menu_text", MESSAGING_MENU_COUNT);
	purple_prefs_add_bool("/plugins/gtk/unity/alert_chat_nick", TRUE);

	alert_chat_nick = purple_prefs_get_bool("/plugins/gtk/unity/alert_chat_nick");

	mmapp = messaging_menu_app_new("pidgin.desktop");
	g_object_ref(mmapp);
	messaging_menu_app_register(mmapp);
	messaging_menu_text = purple_prefs_get_int("/plugins/gtk/unity/messaging_menu_text");

	g_signal_connect(mmapp, "activate-source",
			G_CALLBACK(message_source_activated), NULL);
	g_signal_connect(mmapp, "status-changed",
			G_CALLBACK(messaging_menu_status_changed), NULL);

	saved_status = purple_savedstatus_get_current();
	status_changed_cb(saved_status);

	purple_signal_connect(savedstat_handle, "savedstatus-changed", plugin,
			G_CALLBACK(status_changed_cb), NULL);

	launcher = unity_launcher_entry_get_for_desktop_id("pidgin.desktop");
	g_object_ref(launcher);
	launcher_count = purple_prefs_get_int("/plugins/gtk/unity/launcher_count");

	purple_signal_connect(gtk_conv_handle, "displayed-im-msg", plugin,
			G_CALLBACK(message_displayed_cb), NULL);
	purple_signal_connect(gtk_conv_handle, "displayed-chat-msg", plugin,
			G_CALLBACK(message_displayed_cb), NULL);
	purple_signal_connect(conv_handle, "sent-im-msg", plugin,
			G_CALLBACK(im_sent_im), NULL);
	purple_signal_connect(conv_handle, "sent-chat-msg", plugin,
			G_CALLBACK(chat_sent_im), NULL);
	purple_signal_connect(conv_handle, "conversation-created", plugin,
			G_CALLBACK(conv_created), NULL);
	purple_signal_connect(conv_handle, "deleting-conversation", plugin,
			G_CALLBACK(deleting_conv), NULL);

	manager = purple_conversation_manager_get_default();
	convs = purple_conversation_manager_get_all(manager);
	while(convs != NULL) {
		PurpleConversation *conv = PURPLE_CONVERSATION(convs->data);
		attach_signals(conv);
		convs = g_list_delete_link(convs, convs);
	}

	return TRUE;
}

static gboolean
unity_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
             G_GNUC_UNUSED gboolean shutdown,
             G_GNUC_UNUSED GError **error)
{
	GList *convs = NULL;
	PurpleConversationManager *manager = NULL;

	manager = purple_conversation_manager_get_default();
	convs = purple_conversation_manager_get_all(manager);

	while(convs != NULL) {
		PurpleConversation *conv = PURPLE_CONVERSATION(convs->data);
		unalert(conv);
		detach_signals(conv);
		convs = g_list_delete_link(convs, convs);
	}

	unity_launcher_entry_set_count_visible(launcher, FALSE);
	messaging_menu_app_unregister(mmapp);

	g_object_unref(launcher);
	g_object_unref(mmapp);
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(unity)
