/*
 * Purple buddy notification plugin.
 *
 * Copyright (C) 2000-2001, Eric Warmenhoven (original code)
 * Copyright (C) 2002, Etan Reisner <deryni@eden.rutgers.edu> (rewritten code)
 * Copyright (C) 2003, Christian Hammond (update for changed API)
 * Copyright (C) 2003, Brian Tarricone <bjt23@cornell.edu> (mostly rewritten)
 * Copyright (C) 2003, Mark Doliner (minor cleanup)
 * Copyright (C) 2003, Etan Reisner (largely rewritten again)
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
 *
 */

/* TODO
 * 22:22:17 <seanegan> deryni: speaking of notify.c... you know what else
 * might be a neat feature?
 * 22:22:30 <seanegan> Changing the window icon.
 * 22:23:25 <deryni> seanegan: To what?
 * 22:23:42 <seanegan> deryni: I dunno. Flash it between the regular icon and
 * blank or something.
 * 22:23:53 <deryni> Also I think purple might re-set that sort of frequently,
 * but I'd have to look.
 * 22:25:16 <seanegan> deryni: I keep my conversations in one workspace and am
 * frequently in an another, and the icon flashing in the pager would be a
 * neat visual clue.
 */

/*
 * From Etan, 2002:
 *  -Added config dialog
 *  -Added control over notification method
 *  -Added control over when to release notification
 *
 *  -Added option to get notification for chats also
 *  -Cleaned up code
 *  -Added option to notify on click as it's own option
 *   rather then as what happens when on focus isn't clicked
 *  -Added apply button to change the denotification methods for
 *   open conversation windows
 *  -Fixed apply to conversations, count now keeps count across applies
 *  -Fixed(?) memory leak, and in the process fixed some stupidities
 *  -Hit enter when done editing the title string entry box to save it
 *
 * Thanks to Carles Pina i Estany <carles@pinux.info>
 *   for count of new messages option
 *
 * From Brian, 20 July 2003:
 *  -Use new xml prefs
 *  -Better handling of notification states tracking
 *  -Better pref change handling
 *  -Fixed a possible memleak and possible crash (rare)
 *  -Use gtk_window_get_title() rather than gtkwin->title
 *  -Other random fixes and cleanups
 *
 * Etan again, 12 August 2003:
 *  -Better use of the new xml prefs
 *  -Removed all bitmask stuff
 *  -Even better pref change handling
 *  -Removed unnecessary functions
 *  -Reworking of notification/unnotification stuff
 *  -Header file include cleanup
 *  -General code cleanup
 *
 * Etan yet again, 04 April 2004:
 *  -Re-added Urgent option
 *  -Re-added unnotify on focus option (still needs work, as it will only
 *  react to focus-in events when the entry or history widgets are focused)
 *
 * Sean, 08 January, 2005:
 *  -Added Raise option, formally in Purple proper
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <pidgin.h>

#define NOTIFY_PLUGIN_ID "gtk-x11-notify"

static PurplePlugin *my_plugin = NULL;
#ifdef HAVE_X11
static GdkAtom _Cardinal = GDK_NONE;
static GdkAtom _PurpleUnseenCount = GDK_NONE;
#endif

/* notification set/unset */
static int notify(PurpleConversation *conv, gboolean increment);
static void notify_win(PidginConvWindow *purplewin, PurpleConversation *conv);
static void unnotify(PurpleConversation *conv, gboolean reset);
static int unnotify_cb(GtkWidget *widget, gpointer data,
                       PurpleConversation *conv);

/* gtk widget callbacks for prefs panel */
static void type_toggle_cb(GtkWidget *widget, gpointer data);
static void method_toggle_cb(GtkWidget *widget, gpointer data);
static void notify_toggle_cb(GtkWidget *widget, gpointer data);
static gboolean options_entry_cb(GtkWidget *widget, GdkEventFocus *event,
                                 gpointer data);
static void apply_method(void);
static void apply_notify(void);

/* string function */
static void handle_string(PidginConvWindow *purplewin);

/* count_title function */
static void handle_count_title(PidginConvWindow *purplewin);

/* count_xprop function */
static void handle_count_xprop(PidginConvWindow *purplewin);

/* urgent function */
static void handle_urgent(PidginConvWindow *purplewin, gboolean set);

/* present function */
static void handle_present(PurpleConversation *conv);

/****************************************/
/* Begin doing stuff below this line... */
/****************************************/
static guint
count_messages(PidginConvWindow *purplewin)
{
	guint count = 0;
	GList *convs = NULL, *l;

	for (convs = purplewin->gtkconvs; convs != NULL; convs = convs->next) {
		PidginConversation *conv = convs->data;
		for (l = conv->convs; l != NULL; l = l->next) {
			count += GPOINTER_TO_INT(g_object_get_data(G_OBJECT(l->data), "notify-message-count"));
		}
	}

	return count;
}

static int
notify(PurpleConversation *conv, gboolean increment)
{
	gint count;
	gboolean has_focus;
	PidginConvWindow *purplewin = NULL;
	GSettings *settings = NULL;

	if (conv == NULL || PIDGIN_CONVERSATION(conv) == NULL)
		return 0;

	/* We want to remove the notifications, but not reset the counter */
	unnotify(conv, FALSE);

	purplewin = PIDGIN_CONVERSATION(conv)->win;

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	/* If we aren't doing notifications for this type of conversation, return */
	if ((PURPLE_IS_IM_CONVERSATION(conv) &&
	     !g_settings_get_boolean(settings, "type-im")) ||
	    (PURPLE_IS_CHAT_CONVERSATION(conv) &&
	     !g_settings_get_boolean(settings, "type-chat")))
	{
		g_object_unref(settings);
		return 0;
	}

	g_object_get(G_OBJECT(purplewin->window),
	             "has-toplevel-focus", &has_focus, NULL);

	if(g_settings_get_boolean(settings, "type-focused") || !has_focus) {
		if (increment) {
			count = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "notify-message-count"));
			count++;
			g_object_set_data(G_OBJECT(conv), "notify-message-count", GINT_TO_POINTER(count));
		}

		notify_win(purplewin, conv);
	}

	g_object_unref(settings);

	return 0;
}

static void
notify_win(PidginConvWindow *purplewin, PurpleConversation *conv)
{
	GSettings *settings = NULL;

	if (count_messages(purplewin) <= 0)
		return;

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	if(g_settings_get_boolean(settings, "method-count")) {
		handle_count_title(purplewin);
	}
	if(g_settings_get_boolean(settings, "method-count-xprop")) {
		handle_count_xprop(purplewin);
	}
	if(g_settings_get_boolean(settings, "method-string")) {
		handle_string(purplewin);
	}
	if(g_settings_get_boolean(settings, "method-urgent")) {
		handle_urgent(purplewin, TRUE);
	}
	if(g_settings_get_boolean(settings, "method-present")) {
		handle_present(conv);
	}

	g_object_unref(settings);
}

static void
unnotify(PurpleConversation *conv, gboolean reset)
{
	PurpleConversation *active_conv = NULL;
	PidginDisplayWindow *displaywin = NULL;
	GtkWidget *win;

	g_return_if_fail(conv != NULL);
	if (PIDGIN_CONVERSATION(conv) == NULL)
		return;

	win = gtk_widget_get_toplevel(PIDGIN_CONVERSATION(conv)->tab_cont);
	displaywin = PIDGIN_DISPLAY_WINDOW(win);

	activate_conv = pidgin_display_window_get_selected(purplewin);

	/* reset the conversation window title */
	purple_conversation_autoset_title(active_conv);

	if (reset) {
		/* Only need to actually remove the urgent hinting here, since
		 * removing it just to have it re-added in re-notify is an
		 * unnecessary couple extra RTs to the server */
		handle_urgent(purplewin, FALSE);
		g_object_set_data(G_OBJECT(conv), "notify-message-count", GINT_TO_POINTER(0));
		/* Same logic as for the urgent hint, xprops are also a RT.
		 * This needs to go here so that it gets the updated message
		 * count. */
		handle_count_xprop(purplewin);
	}

	return;
}

static int
unnotify_cb(GtkWidget *widget, gpointer data, PurpleConversation *conv)
{
	if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "notify-message-count")) != 0)
		unnotify(conv, TRUE);

	return 0;
}

static gboolean
message_displayed_cb(PurpleConversation *conv, PurpleMessage *msg, gpointer _unused)
{
	PurpleMessageFlags flags = purple_message_get_flags(msg);
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	/* Ignore anything that's not a received message or a system message */
	if(!(flags & (PURPLE_MESSAGE_RECV|PURPLE_MESSAGE_SYSTEM))) {
		g_object_unref(settings);
		return FALSE;
	}
	/* Don't highlight for delayed messages */
	if((flags & PURPLE_MESSAGE_RECV) && (flags & PURPLE_MESSAGE_DELAYED)) {
		g_object_unref(settings);
		return FALSE;
	}
	/* Check whether to highlight for system message for either chat or IM */
	if (flags & PURPLE_MESSAGE_SYSTEM) {
		if (PURPLE_IS_CHAT_CONVERSATION(conv)) {
			if (!g_settings_get_boolean(settings, "type-chat-sys")) {
				g_object_unref(settings);
				return FALSE;
			}
		} else if (PURPLE_IS_IM_CONVERSATION(conv)) {
			if (!g_settings_get_boolean(settings, "type-im-sys")) {
				g_object_unref(settings);
				return FALSE;
			}
		} else {
			/* System message not from chat or IM, ignore */
			g_object_unref(settings);
			return FALSE;
		}
	}
	
	/* If it's a chat, check if we should only highlight when nick is mentioned */
	if ((PURPLE_IS_CHAT_CONVERSATION(conv) &&
	     g_settings_get_boolean(settings, "type-chat-nick") &&
	     !(flags & PURPLE_MESSAGE_NICK)))
	{
		g_object_unref(settings);
		return FALSE;
	}

	/* Nothing speaks against notifying, do so */
	notify(conv, TRUE);

	g_object_unref(settings);
	return FALSE;
}

static void
im_sent_im(PurpleAccount *account, PurpleMessage *msg, gpointer _unused)
{
	PurpleIMConversation *im = NULL;
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	if(g_settings_get_boolean(settings, "notify-send")) {
		im = purple_conversations_find_im_with_account(
			purple_message_get_recipient(msg), account);
		unnotify(PURPLE_CONVERSATION(im), TRUE);
	}

	g_object_unref(settings);
}

static void
chat_sent_im(PurpleAccount *account, PurpleMessage *msg, int id)
{
	PurpleChatConversation *chat = NULL;
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	if(g_settings_get_boolean(settings, "notify-send")) {
		chat = purple_conversations_find_chat(purple_account_get_connection(account), id);
		unnotify(PURPLE_CONVERSATION(chat), TRUE);
	}

	g_object_unref(settings);
}

static int
attach_signals(PurpleConversation *conv)
{
	PidginConversation *gtkconv = NULL;
	GSList *webview_ids = NULL, *entry_ids = NULL;
	guint id;
	GSettings *settings = NULL;

	gtkconv = PIDGIN_CONVERSATION(conv);
	if (!gtkconv) {
		purple_debug_misc("notify", "Failed to find gtkconv\n");
		return 0;
	}

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	if(g_settings_get_boolean(settings, "notify_focus")) {
		/* TODO should really find a way to make this work no matter
		 * where the focus is inside the conv window, without having
		 * to bind to focus-in-event on the g(d|t)kwindow */
		/* try setting the signal on the focus-in-event for
		 * gtkwin->notebook->container? */
		id = g_signal_connect(G_OBJECT(gtkconv->entry), "focus-in-event",
		                      G_CALLBACK(unnotify_cb), conv);
		entry_ids = g_slist_append(entry_ids, GUINT_TO_POINTER(id));

		id = g_signal_connect(G_OBJECT(gtkconv->webview), "focus-in-event",
		                      G_CALLBACK(unnotify_cb), conv);
		webview_ids = g_slist_append(webview_ids, GUINT_TO_POINTER(id));
	}

	if(g_settings_get_boolean(settings, "notify-click")) {
		/* TODO similarly should really find a way to allow for
		 * clicking in other places of the window */
		id = g_signal_connect(G_OBJECT(gtkconv->entry), "button-press-event",
		                      G_CALLBACK(unnotify_cb), conv);
		entry_ids = g_slist_append(entry_ids, GUINT_TO_POINTER(id));

		id = g_signal_connect(G_OBJECT(gtkconv->webview), "button-press-event",
		                      G_CALLBACK(unnotify_cb), conv);
		webview_ids = g_slist_append(webview_ids, GUINT_TO_POINTER(id));
	}

	if(g_settings_get_boolean(settings, "notify-type")) {
		id = g_signal_connect(G_OBJECT(gtkconv->entry), "key-press-event",
		                      G_CALLBACK(unnotify_cb), conv);
		entry_ids = g_slist_append(entry_ids, GUINT_TO_POINTER(id));
	}

	g_object_set_data(G_OBJECT(conv), "notify-webview-signals", webview_ids);
	g_object_set_data(G_OBJECT(conv), "notify-entry-signals", entry_ids);

	g_object_unref(settings);
	return 0;
}

static void
detach_signals(PurpleConversation *conv)
{
	PidginConversation *gtkconv = NULL;
	GSList *ids = NULL, *l;

	gtkconv = PIDGIN_CONVERSATION(conv);
	if (!gtkconv)
		return;

	ids = g_object_get_data(G_OBJECT(conv), "notify-webview-signals");
	for (l = ids; l != NULL; l = l->next)
		g_signal_handler_disconnect(gtkconv->webview, GPOINTER_TO_INT(l->data));
	g_slist_free(ids);

	ids = g_object_get_data(G_OBJECT(conv), "notify-entry-signals");
	for (l = ids; l != NULL; l = l->next)
		g_signal_handler_disconnect(gtkconv->entry, GPOINTER_TO_INT(l->data));
	g_slist_free(ids);

	g_object_set_data(G_OBJECT(conv), "notify-message-count", GINT_TO_POINTER(0));

	g_object_set_data(G_OBJECT(conv), "notify-webview-signals", NULL);
	g_object_set_data(G_OBJECT(conv), "notify-entry-signals", NULL);
}

static void
conv_created(PurpleConversation *conv)
{
	g_object_set_data(G_OBJECT(conv), "notify-message-count",
	                           GINT_TO_POINTER(0));

	/* always attach the signals, notify() will take care of conversation
	 * type checking */
	attach_signals(conv);
}

static void
conv_switched(PurpleConversation *conv)
{
#if 0
	PidginConvWindow *purplewin = purple_conversation_get_window(new_conv);
	GSettings *settings = NULL;
#endif

	/*
	 * If the conversation was switched, then make sure we re-notify
	 * because Purple will have overwritten our custom window title.
	 */
	notify(conv, FALSE);

#if 0
	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	printf("conv_switched - %p - %p\n", old_conv, new_conv);
	printf("count - %d\n", count_messages(purplewin));
	if(g_settings_get_boolean(settings, "notify-switch")) {
		unnotify(new_conv, FALSE);
	} else {
		/* if we don't have notification on the window then we don't want to
		 * re-notify it */
		if (count_messages(purplewin))
			notify_win(purplewin);
	}

	g_object_unref(settings);
#endif
}

static void
deleting_conv(PurpleConversation *conv)
{
	PidginConvWindow *purplewin = NULL;
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

	if (gtkconv == NULL)
		return;

	detach_signals(conv);

	purplewin = gtkconv->win;

	handle_urgent(purplewin, FALSE);
	g_object_set_data(G_OBJECT(conv), "notify-message-count", GINT_TO_POINTER(0));

	return;

#if 0
	/* i think this line crashes */
	if (count_messages(purplewin))
		notify_win(purplewin);
#endif
}

static void
handle_string(PidginConvWindow *purplewin)
{
	GtkWindow *window = NULL;
	gchar *prefix = NULL;
	gchar *newtitle = NULL;
	GSettings *settings = NULL;

	g_return_if_fail(purplewin != NULL);

	window = GTK_WINDOW(purplewin->window);
	g_return_if_fail(window != NULL);

	settings = g_settings_new_with_backend("im.pidgin.Pidgin.plugin.Notify",
	                                       purple_core_get_settings_backend());

	prefix = g_settings_get_string("title-string");
	newtitle = g_strconcat(prefix, gtk_window_get_title(window), NULL);
	gtk_window_set_title(window, newtitle);

	g_free(prefix);
	g_free(newtitle);
	g_object_unref(settings);
}

static void
handle_count_title(PidginConvWindow *purplewin)
{
	GtkWindow *window;
	char newtitle[256];

	g_return_if_fail(purplewin != NULL);

	window = GTK_WINDOW(purplewin->window);
	g_return_if_fail(window != NULL);

	g_snprintf(newtitle, sizeof(newtitle), "[%d] %s",
	           count_messages(purplewin), gtk_window_get_title(window));
	gtk_window_set_title(window, newtitle);
}

static void
handle_count_xprop(PidginConvWindow *purplewin)
{
#ifdef HAVE_X11
	guint count;
	GtkWidget *window;
	GdkWindow *gdkwin;

	window = purplewin->window;
	g_return_if_fail(window != NULL);

	if (_PurpleUnseenCount == GDK_NONE) {
		_PurpleUnseenCount = gdk_atom_intern("_PIDGIN_UNSEEN_COUNT", FALSE);
	}

	if (_Cardinal == GDK_NONE) {
		_Cardinal = gdk_atom_intern("CARDINAL", FALSE);
	}

	count = count_messages(purplewin);
	gdkwin = gtk_widget_get_window(window);

	gdk_property_change(gdkwin, _PurpleUnseenCount, _Cardinal, 32,
	                    GDK_PROP_MODE_REPLACE, (guchar *) &count, 1);
#endif
}

static void
handle_urgent(PidginConvWindow *purplewin, gboolean set)
{
	g_return_if_fail(purplewin != NULL);
	g_return_if_fail(purplewin->window != NULL);

	gtk_window_set_urgency_hint(GTK_WINDOW(purplewin->window), set);
}

static void
handle_present(PurpleConversation *conv)
{
	if (pidgin_conv_is_hidden(PIDGIN_CONVERSATION(conv)))
		return;

	purple_conversation_present(conv);
}

static void
type_toggle_cb(GtkWidget *widget, gpointer data)
{
	gboolean on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gchar pref[256];

	g_snprintf(pref, sizeof(pref), "/plugins/gtk/X11/notify/%s",
	           (char *)data);

	purple_prefs_set_bool(pref, on);
}

static void
method_toggle_cb(GtkWidget *widget, gpointer data)
{
	gboolean on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gchar pref[256];

	g_snprintf(pref, sizeof(pref), "/plugins/gtk/X11/notify/%s",
	           (char *)data);

	purple_prefs_set_bool(pref, on);

	if (purple_strequal(data, "method_string")) {
		GtkWidget *entry = g_object_get_data(G_OBJECT(widget), "title-entry");
		gtk_widget_set_sensitive(entry, on);

		purple_prefs_set_string("/plugins/gtk/X11/notify/title_string",
		                        gtk_editable_get_text(GTK_EDITABLE(entry)));
	}

	apply_method();
}

static void
notify_toggle_cb(GtkWidget *widget, gpointer data)
{
	gboolean on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gchar pref[256];

	g_snprintf(pref, sizeof(pref), "/plugins/gtk/X11/notify/%s",
	           (char *)data);

	purple_prefs_set_bool(pref, on);

	apply_notify();
}

static gboolean
options_entry_cb(GtkWidget *widget, GdkEventFocus *evt, gpointer data)
{
	if (data == NULL)
		return FALSE;

	if (purple_strequal(data, "method_string")) {
		purple_prefs_set_string("/plugins/gtk/X11/notify/title_string",
		                        gtk_editable_get_text(GTK_EDITABLE(widget)));
	}

	apply_method();

	return FALSE;
}

static void
apply_method(void)
{
	GList *convs;

	for (convs = purple_conversations_get_all(); convs != NULL;
	     convs = convs->next) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;

		/* remove notifications */
		unnotify(conv, FALSE);

		if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "notify-message-count")) != 0)
			/* reattach appropriate notifications */
			notify(conv, FALSE);
	}
}

static void
apply_notify(void)
{
	GList *convs = purple_conversations_get_all();

	while (convs) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;

		/* detach signals */
		detach_signals(conv);
		/* reattach appropriate signals */
		attach_signals(conv);

		convs = convs->next;
	}
}

static GtkWidget *
get_config_frame(PurplePlugin *plugin)
{
	GtkWidget *ret = NULL, *frame = NULL;
	GtkWidget *vbox = NULL, *hbox = NULL;
	GtkWidget *toggle = NULL, *entry = NULL, *ref;

	ret = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
	gtk_container_set_border_width(GTK_CONTAINER (ret), 12);

	/*---------- "Notify For" ----------*/
	frame = pidgin_make_frame(ret, _("Notify For"));
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	toggle = gtk_check_button_new_with_mnemonic(_("_IM windows"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/type_im"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(type_toggle_cb), "type_im");

	ref = toggle;
	toggle = gtk_check_button_new_with_mnemonic(_("\tS_ystem messages"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                            purple_prefs_get_bool("/plugins/gtk/X11/notify/type_im_sys"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(type_toggle_cb), "type_im_sys");
	g_object_bind_property(ref, "active", toggle, "sensitive",
			G_BINDING_SYNC_CREATE);

	toggle = gtk_check_button_new_with_mnemonic(_("C_hat windows"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/type_chat"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(type_toggle_cb), "type_chat");

	ref = toggle;
	toggle = gtk_check_button_new_with_mnemonic(_("\t_Only when someone says your username"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                            purple_prefs_get_bool("/plugins/gtk/X11/notify/type_chat_nick"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(type_toggle_cb), "type_chat_nick");
	g_object_bind_property(ref, "active", toggle, "sensitive",
			G_BINDING_SYNC_CREATE);

	toggle = gtk_check_button_new_with_mnemonic(_("\tS_ystem messages"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                            purple_prefs_get_bool("/plugins/gtk/X11/notify/type_chat_sys"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(type_toggle_cb), "type_chat_sys");
	g_object_bind_property(ref, "active", toggle, "sensitive",
			G_BINDING_SYNC_CREATE);

	toggle = gtk_check_button_new_with_mnemonic(_("_Focused windows"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/type_focused"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(type_toggle_cb), "type_focused");

	/*---------- "Notification Methods" ----------*/
	frame = pidgin_make_frame(ret, _("Notification Methods"));
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* String method button */
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	toggle = gtk_check_button_new_with_mnemonic(_("Prepend _string into window title:"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/method_string"));
	gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, 0);

	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
	gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),
	                         purple_prefs_get_bool("/plugins/gtk/X11/notify/method_string"));
	gtk_editable_set_text(GTK_EDITABLE(entry),
	                      purple_prefs_get_string("/plugins/gtk/X11/notify/title_string"));
	g_object_set_data(G_OBJECT(toggle), "title-entry", entry);
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(method_toggle_cb), "method_string");
	g_signal_connect(G_OBJECT(entry), "focus-out-event",
	                 G_CALLBACK(options_entry_cb), "method_string");

	/* Count method button */
	toggle = gtk_check_button_new_with_mnemonic(_("Insert c_ount of new messages into window title"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/method_count"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(method_toggle_cb), "method_count");

#ifdef HAVE_X11
	/* Count xprop method button */
	toggle = gtk_check_button_new_with_mnemonic(_("Insert count of new message into _X property"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/method_count_xprop"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(method_toggle_cb), "method_count_xprop");

	/* Urgent method button */
	toggle = gtk_check_button_new_with_mnemonic(_("Set window manager \"_URGENT\" hint"));
#else
	toggle = gtk_check_button_new_with_mnemonic(_("_Flash window"));
#endif
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/method_urgent"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(method_toggle_cb), "method_urgent");

	/* Raise window method button */
	toggle = gtk_check_button_new_with_mnemonic(_("R_aise conversation window"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/method_raise"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(method_toggle_cb), "method_raise");

	/* Present conversation method button */
	/* Translators: "Present" as used here is a verb. The plugin presents
	 * the window to the user. */
	toggle = gtk_check_button_new_with_mnemonic(_("_Present conversation window"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/method_present"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(method_toggle_cb), "method_present");

	/*---------- "Notification Removals" ----------*/
	frame = pidgin_make_frame(ret, _("Notification Removal"));
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Remove on focus button */
	toggle = gtk_check_button_new_with_mnemonic(_("Remove when conversation window _gains focus"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/notify_focus"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(notify_toggle_cb), "notify_focus");

	/* Remove on click button */
	toggle = gtk_check_button_new_with_mnemonic(_("Remove when conversation window _receives click"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/notify_click"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(notify_toggle_cb), "notify_click");

	/* Remove on type button */
	toggle = gtk_check_button_new_with_mnemonic(_("Remove when _typing in conversation window"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/notify_type"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(notify_toggle_cb), "notify_type");

	/* Remove on message send button */
	toggle = gtk_check_button_new_with_mnemonic(_("Remove when a _message gets sent"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/notify_send"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(notify_toggle_cb), "notify_send");

#if 0
	/* Remove on conversation switch button */
	toggle = gtk_check_button_new_with_mnemonic(_("Remove on switch to conversation ta_b"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
	                             purple_prefs_get_bool("/plugins/gtk/X11/notify/notify_switch"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
	                 G_CALLBACK(notify_toggle_cb), "notify_switch");
#endif

	gtk_widget_show_all(ret);
	return ret;
}

static GPluginPluginInfo *
notify_query(GError **error)
{
	const gchar * const authors[] = {
		"Etan Reisner <deryni@eden.rutgers.edu>",
		"Brian Tarricone <bjt23@cornell.edu>",
		NULL
	};

	return pidgin_plugin_info_new(
		"id",                   NOTIFY_PLUGIN_ID,
		"name",                 N_("Message Notification"),
		"version",              DISPLAY_VERSION,
		"category",             N_("Notification"),
		"summary",              N_("Provides a variety of ways of notifying "
		                           "you of unread messages."),
		"description",          N_("Provides a variety of ways of notifying "
		                           "you of unread messages."),
		"authors",              authors,
		"website",              PURPLE_WEBSITE,
		"abi-version",          PURPLE_ABI_VERSION,
		"gtk-config-frame-cb",  get_config_frame,
		NULL
	);
}

static gboolean
notify_load(GPluginPlugin *plugin, GError **error)
{
	GList *convs = purple_conversations_get_all();
	void *conv_handle = purple_conversations_get_handle();
	void *gtk_conv_handle = pidgin_conversations_get_handle();

	my_plugin = plugin;

	purple_signal_connect(gtk_conv_handle, "displayed-im-msg", plugin,
	                    G_CALLBACK(message_displayed_cb), NULL);
	purple_signal_connect(gtk_conv_handle, "displayed-chat-msg", plugin,
	                    G_CALLBACK(message_displayed_cb), NULL);
	purple_signal_connect(gtk_conv_handle, "conversation-switched", plugin,
	                    G_CALLBACK(conv_switched), NULL);
	purple_signal_connect(conv_handle, "sent-im-msg", plugin,
	                    G_CALLBACK(im_sent_im), NULL);
	purple_signal_connect(conv_handle, "sent-chat-msg", plugin,
	                    G_CALLBACK(chat_sent_im), NULL);
	purple_signal_connect(conv_handle, "conversation-created", plugin,
	                    G_CALLBACK(conv_created), NULL);
	purple_signal_connect(conv_handle, "deleting-conversation", plugin,
	                    G_CALLBACK(deleting_conv), NULL);

	while (convs) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;

		/* attach signals */
		attach_signals(conv);

		convs = convs->next;
	}

	return TRUE;
}

static gboolean
notify_unload(GPluginPlugin *plugin, gboolean shutdown, GError **error)
{
	GList *convs = purple_conversations_get_all();

	while (convs) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;

		/* kill signals */
		detach_signals(conv);

		convs = convs->next;
	}

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(notify)
