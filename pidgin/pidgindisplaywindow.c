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

#include <adwaita.h>

#include "pidgindisplaywindow.h"

#include "stdio.h"

#include "gtkconv.h"
#include "gtkdialogs.h"
#include "gtkutils.h"
#include "pidgindisplayitem.h"
#include "pidgininvitedialog.h"
#include "pidginaccountrow.h"

enum {
	SIG_CONVERSATION_SWITCHED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

struct _PidginDisplayWindow {
	GtkApplicationWindow parent;

	GtkWidget *view;
	GtkWidget *bin;

	GListModel *base_model;
	GListModel *selection_model;

	GListStore *conversation_model;
};

G_DEFINE_TYPE(PidginDisplayWindow, pidgin_display_window,
              GTK_TYPE_APPLICATION_WINDOW)

static GtkWidget *default_window = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_display_window_actions_set_enabled(GActionMap *map,
                                          const gchar **actions,
                                          gboolean enabled)
{
	for(int i = 0; actions[i] != NULL; i++) {
		GAction *action = NULL;
		const gchar *name = actions[i];

		action = g_action_map_lookup_action(map, name);
		if(action != NULL) {
			g_simple_action_set_enabled(G_SIMPLE_ACTION(action), enabled);
		}
	}
}

static GListModel *
pidgin_display_window_create_model(GObject *item,
                                   G_GNUC_UNUSED gpointer data)
{
	GListModel *model = NULL;

	model = pidgin_display_item_get_children(PIDGIN_DISPLAY_ITEM(item));
	if(model != NULL) {
		return g_object_ref(model);
	}

	return NULL;
}

static gboolean
pidgin_display_window_find_conversation(gconstpointer a, gconstpointer b) {
	PidginDisplayItem *item_a = PIDGIN_DISPLAY_ITEM((gpointer)a);
	PidginDisplayItem *item_b = PIDGIN_DISPLAY_ITEM((gpointer)b);
	PurpleConversation *conversation_a = NULL;
	PurpleConversation *conversation_b = NULL;

	conversation_a = g_object_get_data(G_OBJECT(item_a), "conversation");
	conversation_b = g_object_get_data(G_OBJECT(item_b), "conversation");

	return (conversation_a == conversation_b);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_display_window_invite_cb(GtkDialog *dialog, gint response_id,
                                G_GNUC_UNUSED gpointer data)
{
	PidginInviteDialog *invite_dialog = PIDGIN_INVITE_DIALOG(dialog);
	PurpleChatConversation *chat = NULL;

	chat = pidgin_invite_dialog_get_conversation(invite_dialog);

	g_object_set_data(G_OBJECT(chat), "pidgin-invite-dialog", NULL);

	if(response_id == GTK_RESPONSE_ACCEPT) {
		const gchar *contact = NULL, *message = NULL;

		contact = pidgin_invite_dialog_get_contact(invite_dialog);
		message = pidgin_invite_dialog_get_message(invite_dialog);

		if(!purple_strequal(contact, "")) {
			PurpleConnection *connection = NULL;

			connection = purple_conversation_get_connection(PURPLE_CONVERSATION(chat));
			purple_serv_chat_invite(connection,
			                        purple_chat_conversation_get_id(chat),
			                        message, contact);
		}
	}

	gtk_window_destroy(GTK_WINDOW(invite_dialog));
}

/******************************************************************************
 * Actions
 *****************************************************************************/
static void
pidgin_display_window_alias(G_GNUC_UNUSED GSimpleAction *simple,
                            G_GNUC_UNUSED GVariant *parameter,
                            gpointer data)
{
	PidginDisplayWindow *window = data;
	PurpleConversation *selected = NULL;

	selected = pidgin_display_window_get_selected(window);
	if(PURPLE_IS_CONVERSATION(selected)) {
		PurpleAccount *account;
		const gchar *name;

		account = purple_conversation_get_account(selected);
		name = purple_conversation_get_name(selected);

		if(PURPLE_IS_IM_CONVERSATION(selected)) {
			PurpleBuddy *buddy = purple_blist_find_buddy(account, name);

			if(PURPLE_IS_BUDDY(buddy)) {
				pidgin_dialogs_alias_buddy(buddy);
			}
		} else if(PURPLE_IS_CHAT_CONVERSATION(selected)) {
			PurpleChat *chat = purple_blist_find_chat(account, name);

			if(PURPLE_IS_CHAT(chat)) {
				pidgin_dialogs_alias_chat(chat);
			}
		}
	}
}

static void
pidgin_display_window_close_conversation(G_GNUC_UNUSED GSimpleAction *simple,
                                         G_GNUC_UNUSED GVariant *parameter,
                                         gpointer data)
{
	PidginDisplayWindow *window = data;
	PurpleConversation *selected = NULL;

	selected = pidgin_display_window_get_selected(window);
	if(PURPLE_IS_CONVERSATION(selected)) {
		pidgin_display_window_remove(window, selected);
		pidgin_conversation_detach(selected);
	}
}

static void
pidgin_display_window_get_info(G_GNUC_UNUSED GSimpleAction *simple,
                               G_GNUC_UNUSED GVariant *parameter,
                               gpointer data)
{
	PidginDisplayWindow *window = data;
	PurpleConversation *selected = NULL;

	selected = pidgin_display_window_get_selected(window);
	if(PURPLE_IS_CONVERSATION(selected)) {
		if(PURPLE_IS_IM_CONVERSATION(selected)) {
			PurpleConnection *connection = NULL;

			connection = purple_conversation_get_connection(selected);
			pidgin_retrieve_user_info(connection,
			                          purple_conversation_get_name(selected));
		}
	}
}

static void
pidgin_display_window_invite(G_GNUC_UNUSED GSimpleAction *simple,
                             G_GNUC_UNUSED GVariant *parameter,
                             gpointer data)
{
	PidginDisplayWindow *window = data;
	PurpleConversation *selected = NULL;

	selected = pidgin_display_window_get_selected(window);
	if(PURPLE_IS_CHAT_CONVERSATION(selected)) {
		GtkWidget *invite_dialog = NULL;

		invite_dialog = g_object_get_data(G_OBJECT(selected),
		                                  "pidgin-invite-dialog");

		if(!GTK_IS_WIDGET(invite_dialog)) {
			invite_dialog = pidgin_invite_dialog_new(PURPLE_CHAT_CONVERSATION(selected));
			g_object_set_data(G_OBJECT(selected), "pidgin-invite-dialog",
			                  invite_dialog);

			gtk_window_set_transient_for(GTK_WINDOW(invite_dialog),
			                             GTK_WINDOW(window));
			gtk_window_set_destroy_with_parent(GTK_WINDOW(invite_dialog), TRUE);

			g_signal_connect(invite_dialog, "response",
			                 G_CALLBACK(pidgin_display_window_invite_cb),
			                 NULL);
		}

		gtk_widget_show(invite_dialog);
	}
}

static void
pidgin_display_window_send_file(G_GNUC_UNUSED GSimpleAction *simple,
                                G_GNUC_UNUSED GVariant *parameter,
                                gpointer data)
{
	PidginDisplayWindow *window = data;
	PurpleConversation *selected = NULL;

	selected = pidgin_display_window_get_selected(window);
	if(PURPLE_IS_IM_CONVERSATION(selected)) {
		PurpleConnection *connection = NULL;

		connection = purple_conversation_get_connection(selected);
		purple_serv_send_file(connection,
		                      purple_conversation_get_name(selected),
		                      NULL);
	}
}

static GActionEntry win_entries[] = {
	{
		.name = "alias",
		.activate = pidgin_display_window_alias
	}, {
		.name = "close",
		.activate = pidgin_display_window_close_conversation
	}, {
		.name = "get-info",
		.activate = pidgin_display_window_get_info
	}, {
		.name = "invite",
		.activate = pidgin_display_window_invite
	}, {
		.name = "send-file",
		.activate = pidgin_display_window_send_file
	}
};

/*<private>
 * pidgin_display_window_conversation_actions:
 *
 * A list of action names that are only valid if a conversation is selected.
 */
static const gchar *pidgin_display_window_conversation_actions[] = {
	"alias",
	"close",
	"get-info",
	NULL
};

static const gchar *pidgin_display_window_im_conversation_actions[] = {
	"send-file",
	NULL
};

static const gchar *pidgin_display_window_chat_conversation_actions[] = {
	"invite",
	NULL
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
pidgin_display_window_key_pressed_cb(G_GNUC_UNUSED GtkEventControllerKey *controller,
                                     guint keyval,
                                     G_GNUC_UNUSED guint keycode,
                                     GdkModifierType state,
                                     gpointer data)
{
	PidginDisplayWindow *window = data;

	if (state & GDK_CONTROL_MASK) {
		switch (keyval) {
			case GDK_KEY_Page_Down:
			case GDK_KEY_KP_Page_Down:
			case ']':
				pidgin_display_window_select_next(window);
				return TRUE;
				break;
			case GDK_KEY_Home:
				pidgin_display_window_select_first(window);
				return TRUE;
				break;
			case GDK_KEY_End:
				pidgin_display_window_select_last(window);
				return TRUE;
				break;
			case GDK_KEY_Page_Up:
			case GDK_KEY_KP_Page_Up:
			case '[':
				pidgin_display_window_select_previous(window);
				return TRUE;
				break;
		}
	} else if (state & GDK_ALT_MASK) {
		if ('1' <= keyval && keyval <= '9') {
			guint switchto = keyval - '1';
			pidgin_display_window_select_nth(window, switchto);

			return TRUE;
		}
	}

	return FALSE;
}

static void
pidgin_display_window_selected_item_changed_cb(GObject *self,
                                               G_GNUC_UNUSED GParamSpec *pspec,
                                               gpointer data)
{
	PidginDisplayItem *item = NULL;
	PidginDisplayWindow *window = data;
	PurpleConversation *conversation = NULL;
	GtkSingleSelection *selection = GTK_SINGLE_SELECTION(self);
	GtkTreeListRow *row = NULL;
	GtkWidget *widget = NULL;
	gboolean is_conversation = FALSE;
	gboolean is_im_conversation = FALSE;
	gboolean is_chat_conversation = FALSE;

	row = gtk_single_selection_get_selected_item(selection);

	item = gtk_tree_list_row_get_item(row);

	/* Toggle whether actions should be enabled or disabled. */
	conversation = g_object_get_data(G_OBJECT(item), "conversation");
	if(PURPLE_IS_CONVERSATION(conversation)) {
		is_conversation = PURPLE_IS_CONVERSATION(conversation);
		is_im_conversation = PURPLE_IS_IM_CONVERSATION(conversation);
		is_chat_conversation = PURPLE_IS_CHAT_CONVERSATION(conversation);
	}

	pidgin_display_window_actions_set_enabled(G_ACTION_MAP(window),
	                                          pidgin_display_window_conversation_actions,
	                                          is_conversation);
	pidgin_display_window_actions_set_enabled(G_ACTION_MAP(window),
	                                          pidgin_display_window_im_conversation_actions,
	                                          is_im_conversation);
	pidgin_display_window_actions_set_enabled(G_ACTION_MAP(window),
	                                          pidgin_display_window_chat_conversation_actions,
	                                          is_chat_conversation);

	widget = pidgin_display_item_get_widget(item);
	if(GTK_IS_WIDGET(widget)) {
		adw_bin_set_child(ADW_BIN(window->bin), widget);
	}
}
static char *
pidgin_display_accounts_icons_cb(void)
{
	const char* icon_name = NULL;

	PurpleAccountManager *manager = NULL;
	GList *enabled = NULL;
	// 

	manager = purple_account_manager_get_default();
	enabled = purple_account_manager_get_enabled(manager);	
	
	if(enabled == NULL)
		return NULL;
	
	if(PURPLE_IS_ACCOUNT(enabled->data)) {
		PurpleProtocol *protocol = purple_account_get_protocol(enabled->data);
		if(PURPLE_IS_PROTOCOL(protocol)) {
			icon_name = purple_protocol_get_icon_name(protocol);
		}
	}
	g_print("Here is the string: %s\n", icon_name);
	return g_strdup(icon_name);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_display_window_dispose(GObject *obj) {
	PidginDisplayWindow *window = PIDGIN_DISPLAY_WINDOW(obj);

	g_clear_object(&window->conversation_model);

	G_OBJECT_CLASS(pidgin_display_window_parent_class)->dispose(obj);
}

static void
pidgin_display_window_init(PidginDisplayWindow *window) {
	GtkEventController *key = NULL;
	GtkTreeListModel *tree_model = NULL;

	gtk_widget_init_template(GTK_WIDGET(window));

	/* Setup the tree list model. */
	tree_model = gtk_tree_list_model_new(window->base_model, FALSE, TRUE,
	                                     (GtkTreeListModelCreateModelFunc)pidgin_display_window_create_model,
	                                     window, NULL);

	/* Set the model of the selection to the tree model. */
	gtk_single_selection_set_model(GTK_SINGLE_SELECTION(window->selection_model),
	                               G_LIST_MODEL(tree_model));
	g_clear_object(&tree_model);

	/* Set the application and add all of our actions. */
	gtk_window_set_application(GTK_WINDOW(window),
	                           GTK_APPLICATION(g_application_get_default()));

	g_action_map_add_action_entries(G_ACTION_MAP(window), win_entries,
	                                G_N_ELEMENTS(win_entries), window);

	/* Add a key controller. */
	key = gtk_event_controller_key_new();
	gtk_event_controller_set_propagation_phase(key, GTK_PHASE_CAPTURE);
	g_signal_connect(G_OBJECT(key), "key-pressed",
	                 G_CALLBACK(pidgin_display_window_key_pressed_cb),
	                 window);
	gtk_widget_add_controller(GTK_WIDGET(window), key);
}

static void
pidgin_display_window_class_init(PidginDisplayWindowClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->dispose = pidgin_display_window_dispose;

	/**
	 * PidginDisplayWindow::conversation-switched:
	 * @window: The conversation window.
	 * @new_conv: The now active conversation.
	 *
	 * Emitted when a window switched from one conversation to another.
	 */
	signals[SIG_CONVERSATION_SWITCHED] = g_signal_new_class_handler(
		"conversation-switched",
		G_OBJECT_CLASS_TYPE(obj_class),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_CONVERSATION
	);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Display/window.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginDisplayWindow,
	                                     view);
	gtk_widget_class_bind_template_child(widget_class, PidginDisplayWindow,
	                                     bin);
	gtk_widget_class_bind_template_child(widget_class, PidginDisplayWindow,
	                                     base_model);
	gtk_widget_class_bind_template_child(widget_class, PidginDisplayWindow,
	                                     selection_model);
	gtk_widget_class_bind_template_child(widget_class, PidginDisplayWindow,
	                                     conversation_model);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_display_window_key_pressed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_display_window_selected_item_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_display_accounts_icons_cb);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_display_window_get_default(void) {
	if(!GTK_IS_WIDGET(default_window)) {
		default_window = pidgin_display_window_new();
		g_object_add_weak_pointer(G_OBJECT(default_window),
		                          (gpointer)&default_window);
	}

	return default_window;
}

GtkWidget *
pidgin_display_window_new(void) {
	return g_object_new(
		PIDGIN_TYPE_DISPLAY_WINDOW,
		"show-menubar", TRUE,
		NULL);
}

void
pidgin_display_window_add(PidginDisplayWindow *window,
                          PurpleConversation *conversation)
{
	PidginConversation *gtkconv = NULL;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));
	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	gtkconv = PIDGIN_CONVERSATION(conversation);
	if(gtkconv != NULL) {
		PidginDisplayItem *item = NULL;
		const char *value = NULL;

		GtkWidget *parent = gtk_widget_get_parent(gtkconv->tab_cont);

		if(GTK_IS_WIDGET(parent)) {
			g_object_ref(gtkconv->tab_cont);
			gtk_widget_unparent(gtkconv->tab_cont);
		}

		value = purple_conversation_get_name(conversation);
		item = pidgin_display_item_new(gtkconv->tab_cont, value);
		g_object_set_data(G_OBJECT(item), "conversation", conversation);

		g_object_bind_property(conversation, "title",
		                       item, "title",
		                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

		g_list_store_append(window->conversation_model, item);
		g_clear_object(&item);

		if(GTK_IS_WIDGET(parent)) {
			g_object_unref(gtkconv->tab_cont);
		}
	}
}

void
pidgin_display_window_remove(PidginDisplayWindow *window,
                             PurpleConversation *conversation)
{
	PidginDisplayItem *item = NULL;
	guint position = 0;
	gboolean found = FALSE;
	gchar *id = NULL;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));
	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	/* Create a wrapper item for our find function. */
	id = g_uuid_string_random();
	item = g_object_new(PIDGIN_TYPE_DISPLAY_ITEM, "id", id, NULL);
	g_free(id);
	g_object_set_data(G_OBJECT(item), "conversation", conversation);

	found = g_list_store_find_with_equal_func(window->conversation_model,
	                                          item,
	                                          pidgin_display_window_find_conversation,
	                                          &position);

	g_clear_object(&item);

	if(found) {
		g_list_store_remove(window->conversation_model, position);
	}
}

guint
pidgin_display_window_get_count(G_GNUC_UNUSED PidginDisplayWindow *window) {
	/* TODO: This is only used by the gestures plugin and that will probably
	 * need some rewriting and different api for a mixed content window list
	 * this is now.
	 */

	return 0;
}

PurpleConversation *
pidgin_display_window_get_selected(PidginDisplayWindow *window) {
	GtkSingleSelection *selection = NULL;
	GtkTreeListRow *tree_row = NULL;
	GObject *selected = NULL;

	g_return_val_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window), NULL);

	selection = GTK_SINGLE_SELECTION(window->selection_model);
	tree_row = gtk_single_selection_get_selected_item(selection);
	selected = gtk_tree_list_row_get_item(tree_row);

	if(PIDGIN_IS_DISPLAY_ITEM(selected)) {
		return g_object_get_data(selected, "conversation");
	}

	return NULL;
}

void
pidgin_display_window_select(PidginDisplayWindow *window,
                             PurpleConversation *conversation)
{
	/* TODO: This is used by the unity and gestures plugins, but I'm really not
	 * sure how to make this work yet without some hard-coding or something, so
	 * I'm opting to stub it out for now.
	 */

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));
	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));
}

void
pidgin_display_window_select_previous(PidginDisplayWindow *window) {
	GtkSingleSelection *selection = NULL;
	guint position = 0;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));

	selection = GTK_SINGLE_SELECTION(window->selection_model);
	position = gtk_single_selection_get_selected(selection);
	if(position == 0) {
		position = g_list_model_get_n_items(G_LIST_MODEL(selection)) - 1;
	} else {
		position = position - 1;
	}

	gtk_single_selection_set_selected(selection, position);
}

void
pidgin_display_window_select_next(PidginDisplayWindow *window) {
	GtkSingleSelection *selection = NULL;
	guint position = 0;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));

	selection = GTK_SINGLE_SELECTION(window->selection_model);
	position = gtk_single_selection_get_selected(selection);
	if(position + 1 >= g_list_model_get_n_items(G_LIST_MODEL(selection))) {
		position = 0;
	} else {
		position = position + 1;
	}

	gtk_single_selection_set_selected(selection, position);
}

void
pidgin_display_window_select_first(PidginDisplayWindow *window) {
	GtkSingleSelection *selection = NULL;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));

	selection = GTK_SINGLE_SELECTION(window->selection_model);

	/* The selection has autoselect set to true, which won't do anything if
	 * this is an invalid value.
	 */
	gtk_single_selection_set_selected(selection, 0);
}

void
pidgin_display_window_select_last(PidginDisplayWindow *window) {
	GtkSingleSelection *selection = NULL;
	guint n_items = 0;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));

	selection = GTK_SINGLE_SELECTION(window->selection_model);
	n_items = g_list_model_get_n_items(G_LIST_MODEL(selection));

	/* The selection has autoselect set to true, which won't do anything if
	 * this is an invalid value.
	 */
	gtk_single_selection_set_selected(selection, n_items - 1);
}

void
pidgin_display_window_select_nth(PidginDisplayWindow *window,
                                 guint nth)
{
	GtkSingleSelection *selection = NULL;
	guint n_items = 0;

	g_return_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window));

	selection = GTK_SINGLE_SELECTION(window->selection_model);

	/* The selection has autoselect set to true, but this isn't bound checking
	 * or something on the children models, so we verify before setting.
	 */
	n_items = g_list_model_get_n_items(G_LIST_MODEL(selection));
	if(nth < n_items) {
		gtk_single_selection_set_selected(selection, nth);
	}
}

gboolean
pidgin_display_window_conversation_is_selected(PidginDisplayWindow *window,
                                               PurpleConversation *conversation)
{
	GtkSingleSelection *selection = NULL;
	GObject *selected = NULL;

	g_return_val_if_fail(PIDGIN_IS_DISPLAY_WINDOW(window), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	selection = GTK_SINGLE_SELECTION(window->selection_model);
	selected = gtk_single_selection_get_selected_item(selection);

	if(PIDGIN_IS_DISPLAY_ITEM(selected)) {
		PurpleConversation *selected_conversation = NULL;

		selected_conversation = g_object_get_data(G_OBJECT(selected),
		                                          "conversation");
		if(selected_conversation != NULL) {
			return (selected_conversation == conversation);
		}
	}

	return FALSE;
}
