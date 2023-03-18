/*
 * Purple - XMPP debugging tool
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <purple.h>
#include <pidgin.h>

#include "xmppconsole.h"

#define PLUGIN_ID      "gtk-xmpp"
#define PLUGIN_DOMAIN  (g_quark_from_static_string(PLUGIN_ID))

struct _PidginXmppConsole {
	GtkWindow parent;

	PidginAccountChooser *account_chooser;
	PurpleConnection *gc;
	GtkTextBuffer *buffer;
	struct {
		GtkTextTag *info;
		GtkTextTag *incoming;
		GtkTextTag *outgoing;
		GtkTextTag *bracket;
		GtkTextTag *tag;
		GtkTextTag *attr;
		GtkTextTag *value;
		GtkTextTag *xmlns;
	} tags;
	GtkWidget *entry;
	GtkTextBuffer *entry_buffer;
	GtkWidget *sw;

	struct {
		GtkMenuButton *button;
		GtkEntry *to;
		GtkDropDown *type;
	} iq;

	struct {
		GtkMenuButton *button;
		GtkEntry *to;
		GtkDropDown *type;
		GtkDropDown *show;
		GtkEntry *status;
		GtkEntry *priority;
	} presence;

	struct {
		GtkMenuButton *button;
		GtkEntry *to;
		GtkDropDown *type;
		GtkEntry *body;
		GtkEntry *subject;
		GtkEntry *thread;
	} message;
};

G_DEFINE_DYNAMIC_TYPE(PidginXmppConsole, pidgin_xmpp_console, GTK_TYPE_WINDOW)

static PidginXmppConsole *console = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
xmppconsole_append_xmlnode(PidginXmppConsole *console, PurpleXmlNode *node,
                           gint indent_level, GtkTextIter *iter,
                           GtkTextTag *tag)
{
	PurpleXmlNode *c;
	gboolean need_end = FALSE, pretty = TRUE;
	gint i;

	g_return_if_fail(node != NULL);

	for (i = 0; i < indent_level; i++) {
		gtk_text_buffer_insert_with_tags(console->buffer, iter, "\t", 1, tag, NULL);
	}

	gtk_text_buffer_insert_with_tags(console->buffer, iter, "<", 1,
	                                 tag, console->tags.bracket, NULL);
	gtk_text_buffer_insert_with_tags(console->buffer, iter, node->name, -1,
	                                 tag, console->tags.tag, NULL);

	if (node->xmlns) {
		if ((!node->parent ||
		     !node->parent->xmlns ||
		     !purple_strequal(node->xmlns, node->parent->xmlns)) &&
		    !purple_strequal(node->xmlns, "jabber:client"))
		{
			gtk_text_buffer_insert_with_tags(console->buffer, iter, " ", 1,
			                                 tag, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, "xmlns", 5,
			                                 tag, console->tags.attr, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, "='", 2,
			                                 tag, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, node->xmlns, -1,
			                                 tag, console->tags.xmlns, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, "'", 1,
			                                 tag, NULL);
		}
	}
	for (c = node->child; c; c = c->next)
	{
		if (c->type == PURPLE_XMLNODE_TYPE_ATTRIB) {
			gtk_text_buffer_insert_with_tags(console->buffer, iter, " ", 1,
			                                 tag, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, c->name, -1,
			                                 tag, console->tags.attr, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, "='", 2,
			                                 tag, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, c->data, -1,
			                                 tag, console->tags.value, NULL);
			gtk_text_buffer_insert_with_tags(console->buffer, iter, "'", 1,
			                                 tag, NULL);
		} else if (c->type == PURPLE_XMLNODE_TYPE_TAG || c->type == PURPLE_XMLNODE_TYPE_DATA) {
			if (c->type == PURPLE_XMLNODE_TYPE_DATA)
				pretty = FALSE;
			need_end = TRUE;
		}
	}

	if (need_end) {
		gtk_text_buffer_insert_with_tags(console->buffer, iter, ">", 1,
		                                 tag, console->tags.bracket, NULL);
		if (pretty) {
			gtk_text_buffer_insert_with_tags(console->buffer, iter, "\n", 1,
			                                 tag, NULL);
		}

		for (c = node->child; c; c = c->next)
		{
			if (c->type == PURPLE_XMLNODE_TYPE_TAG) {
				xmppconsole_append_xmlnode(console, c, indent_level + 1, iter, tag);
			} else if (c->type == PURPLE_XMLNODE_TYPE_DATA && c->data_sz > 0) {
				gtk_text_buffer_insert_with_tags(console->buffer, iter, c->data, c->data_sz,
				                                 tag, NULL);
			}
		}

		if (pretty) {
			for (i = 0; i < indent_level; i++) {
				gtk_text_buffer_insert_with_tags(console->buffer, iter, "\t", 1, tag, NULL);
			}
		}
		gtk_text_buffer_insert_with_tags(console->buffer, iter, "<", 1,
		                                 tag, console->tags.bracket, NULL);
		gtk_text_buffer_insert_with_tags(console->buffer, iter, "/", 1,
		                                 tag, NULL);
		gtk_text_buffer_insert_with_tags(console->buffer, iter, node->name, -1,
		                                 tag, console->tags.tag, NULL);
		gtk_text_buffer_insert_with_tags(console->buffer, iter, ">", 1,
		                                 tag, console->tags.bracket, NULL);
		gtk_text_buffer_insert_with_tags(console->buffer, iter, "\n", 1,
		                                 tag, NULL);
	} else {
		gtk_text_buffer_insert_with_tags(console->buffer, iter, "/", 1,
		                                 tag, NULL);
		gtk_text_buffer_insert_with_tags(console->buffer, iter, ">", 1,
		                                 tag, console->tags.bracket, NULL);
		gtk_text_buffer_insert_with_tags(console->buffer, iter, "\n", 1,
		                                 tag, NULL);
	}
}

static void
purple_xmlnode_received_cb(PurpleConnection *gc, PurpleXmlNode **packet,
                           G_GNUC_UNUSED gpointer data)
{
	GtkTextIter iter;

	if (console == NULL || console->gc != gc) {
		return;
	}

	gtk_text_buffer_get_end_iter(console->buffer, &iter);
	xmppconsole_append_xmlnode(console, *packet, 0, &iter,
	                           console->tags.incoming);
}

static void
purple_xmlnode_sent_cb(PurpleConnection *gc, char **packet,
                       G_GNUC_UNUSED gpointer data)
{
	GtkTextIter iter;
	PurpleXmlNode *node;

	if (console == NULL || console->gc != gc) {
		return;
	}
	node = purple_xmlnode_from_str(*packet, -1);

	if (!node)
		return;

	gtk_text_buffer_get_end_iter(console->buffer, &iter);
	xmppconsole_append_xmlnode(console, node, 0, &iter,
	                           console->tags.outgoing);
	purple_xmlnode_free(node);
}

static gboolean
message_send_cb(G_GNUC_UNUSED GtkEventControllerKey *event, guint keyval,
                G_GNUC_UNUSED guint keycode,
                G_GNUC_UNUSED GdkModifierType state,
                gpointer data)
{
	PidginXmppConsole *console = data;
	PurpleProtocol *protocol = NULL;
	PurpleConnection *gc;
	gchar *text;
	GtkTextIter start, end;

	if (keyval != GDK_KEY_KP_Enter && keyval != GDK_KEY_Return) {
		return FALSE;
	}

	gc = console->gc;

	if (gc)
		protocol = purple_connection_get_protocol(gc);

	gtk_text_buffer_get_bounds(console->entry_buffer, &start, &end);
	text = gtk_text_buffer_get_text(console->entry_buffer, &start, &end, FALSE);

	if(PURPLE_IS_PROTOCOL_SERVER(protocol)) {
		purple_protocol_server_send_raw(PURPLE_PROTOCOL_SERVER(protocol), gc,
		                                text, strlen(text));
	}

	g_free(text);
	gtk_text_buffer_set_text(console->entry_buffer, "", 0);

	return TRUE;
}

static void
entry_changed_cb(GtkTextBuffer *buffer, gpointer data) {
	PidginXmppConsole *console = data;
	GtkTextIter start, end;
	char *xmlstr, *str;
	GtkTextIter iter;
	int wrapped_lines;
	int lines;
	GdkRectangle oneline;
	int height;
	int pad_top, pad_inside, pad_bottom;
	PurpleXmlNode *node;

	wrapped_lines = 1;
	gtk_text_buffer_get_start_iter(buffer, &iter);
	gtk_text_view_get_iter_location(GTK_TEXT_VIEW(console->entry), &iter, &oneline);
	while (gtk_text_view_forward_display_line(GTK_TEXT_VIEW(console->entry),
	                                          &iter)) {
		wrapped_lines++;
	}

	lines = gtk_text_buffer_get_line_count(buffer);

	/* Show a maximum of 64 lines */
	lines = MIN(lines, 6);
	wrapped_lines = MIN(wrapped_lines, 6);

	pad_top = gtk_text_view_get_pixels_above_lines(GTK_TEXT_VIEW(console->entry));
	pad_bottom = gtk_text_view_get_pixels_below_lines(GTK_TEXT_VIEW(console->entry));
	pad_inside = gtk_text_view_get_pixels_inside_wrap(GTK_TEXT_VIEW(console->entry));

	height = (oneline.height + pad_top + pad_bottom) * lines;
	height += (oneline.height + pad_inside) * (wrapped_lines - lines);

	gtk_widget_set_size_request(console->sw, -1, height + 6);

	gtk_text_buffer_get_bounds(buffer, &start, &end);
	str = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	if (!str) {
		return;
	}

	xmlstr = g_strdup_printf("<xml>%s</xml>", str);
	node = purple_xmlnode_from_str(xmlstr, -1);
	if (node) {
		gtk_text_buffer_remove_tag_by_name(console->entry_buffer, "invalid",
		                                   &start, &end);
	} else {
		gtk_text_buffer_apply_tag_by_name(console->entry_buffer, "invalid",
		                                  &start, &end);
	}
	g_free(str);
	g_free(xmlstr);
	if (node)
		purple_xmlnode_free(node);
}

static void
load_text_and_set_caret(PidginXmppConsole *console, const gchar *pre_text,
                        const gchar *post_text)
{
	GtkTextIter start, end;
	GtkTextIter where;
	GtkTextMark *mark;

	g_signal_handlers_block_by_func(console->entry_buffer, entry_changed_cb,
	                                console);
	gtk_text_buffer_begin_user_action(console->entry_buffer);

	gtk_text_buffer_get_bounds(console->entry_buffer, &start, &end);
	gtk_text_buffer_delete(console->entry_buffer, &start, &end);
	gtk_text_buffer_insert(console->entry_buffer, &end, pre_text, -1);

	gtk_text_buffer_get_end_iter(console->entry_buffer, &where);
	mark = gtk_text_buffer_create_mark(console->entry_buffer, NULL, &where, TRUE);

	gtk_text_buffer_insert(console->entry_buffer, &where, post_text, -1);

	gtk_text_buffer_get_iter_at_mark(console->entry_buffer, &where, mark);
	gtk_text_buffer_place_cursor(console->entry_buffer, &where);
	gtk_text_buffer_delete_mark(console->entry_buffer, mark);

	gtk_text_buffer_end_user_action(console->entry_buffer);
	g_signal_handlers_unblock_by_func(console->entry_buffer, entry_changed_cb,
	                                  console);

	entry_changed_cb(console->entry_buffer, console);
}

static void
iq_clicked_cb(G_GNUC_UNUSED GtkWidget *w, gpointer data)
{
	PidginXmppConsole *console = data;
	GtkStringObject *obj = NULL;
	const gchar *to, *type;
	gchar *stanza;

	to = gtk_editable_get_text(GTK_EDITABLE(console->iq.to));
	obj = gtk_drop_down_get_selected_item(console->iq.type);
	type = gtk_string_object_get_string(obj);

	if(to != NULL && *to != '\0') {
		stanza = g_strdup_printf("<iq to='%s' id='console%x' type='%s'>",
		                         to, g_random_int(), type);
	} else {
		stanza = g_strdup_printf("<iq id='console%x' type='%s'>",
		                         g_random_int(), type);
	}

	load_text_and_set_caret(console, stanza, "</iq>");
	gtk_widget_grab_focus(console->entry);
	g_free(stanza);

	/* Reset everything. */
	gtk_editable_set_text(GTK_EDITABLE(console->iq.to), "");
	gtk_drop_down_set_selected(console->iq.type, 0);
	gtk_menu_button_popdown(console->iq.button);
}

static void
presence_clicked_cb(G_GNUC_UNUSED GtkWidget *w, gpointer data)
{
	PidginXmppConsole *console = data;
	GtkStringObject *obj = NULL;
	const gchar *to, *status, *priority;
	const gchar *type, *show;
	GString *stanza = NULL;

	to = gtk_editable_get_text(GTK_EDITABLE(console->presence.to));
	obj = gtk_drop_down_get_selected_item(console->presence.type);
	type = gtk_string_object_get_string(obj);
	if (purple_strequal(type, "default")) {
		type = "";
	}
	obj = gtk_drop_down_get_selected_item(console->presence.show);
	show = gtk_string_object_get_string(obj);
	if (purple_strequal(show, "default")) {
		show = "";
	}
	status = gtk_editable_get_text(GTK_EDITABLE(console->presence.status));
	priority = gtk_editable_get_text(GTK_EDITABLE(console->presence.priority));
	if (purple_strequal(priority, "0")) {
		priority = "";
	}

	stanza = g_string_new("<presence");
	if(*to != '\0') {
		g_string_append_printf(stanza, " to='%s'", to);
	}
	g_string_append_printf(stanza, " id='console%x'", g_random_int());
	if(*type != '\0') {
		g_string_append_printf(stanza, " type='%s'", type);
	}
	g_string_append_c(stanza, '>');

	if(*show != '\0') {
		g_string_append_printf(stanza, "<show>%s</show>", show);
	}

	if(*status != '\0') {
		g_string_append_printf(stanza, "<status>%s</status>", status);
	}

	if(*priority != '\0') {
		g_string_append_printf(stanza, "<priority>%s</priority>", priority);
	}

	load_text_and_set_caret(console, stanza->str, "</presence>");
	gtk_widget_grab_focus(console->entry);
	g_string_free(stanza, TRUE);

	/* Reset everything. */
	gtk_editable_set_text(GTK_EDITABLE(console->presence.to), "");
	gtk_drop_down_set_selected(console->presence.type, 0);
	gtk_drop_down_set_selected(console->presence.show, 0);
	gtk_editable_set_text(GTK_EDITABLE(console->presence.status), "");
	gtk_editable_set_text(GTK_EDITABLE(console->presence.priority), "0");
	gtk_menu_button_popdown(console->presence.button);
}

static void
message_clicked_cb(G_GNUC_UNUSED GtkWidget *w, gpointer data)
{
	PidginXmppConsole *console = data;
	GtkStringObject *obj = NULL;
	const gchar *to, *body, *thread, *subject, *type;
	GString *stanza = NULL;

	to = gtk_editable_get_text(GTK_EDITABLE(console->message.to));
	body = gtk_editable_get_text(GTK_EDITABLE(console->message.body));
	thread = gtk_editable_get_text(GTK_EDITABLE(console->message.thread));
	subject = gtk_editable_get_text(GTK_EDITABLE(console->message.subject));
	obj = gtk_drop_down_get_selected_item(console->message.type);
	type = gtk_string_object_get_string(obj);

	stanza = g_string_new("<message");
	if(*to != '\0') {
		g_string_append_printf(stanza, " to='%s'", to);
	}
	g_string_append_printf(stanza, " id='console%x' type='%s'>",
	                       g_random_int(), type);

	if(*body != '\0') {
		g_string_append_printf(stanza, "<body>%s</body>", body);
	}

	if(*subject != '\0') {
		g_string_append_printf(stanza, "<subject>%s</subject>", subject);
	}

	if(*thread != '\0') {
		g_string_append_printf(stanza, "<thread>%s</thread>", thread);
	}

	load_text_and_set_caret(console, stanza->str, "</message>");
	gtk_widget_grab_focus(console->entry);
	g_string_free(stanza, TRUE);

	/* Reset everything. */
	gtk_editable_set_text(GTK_EDITABLE(console->message.to), "");
	gtk_drop_down_set_selected(console->message.type, 0);
	gtk_editable_set_text(GTK_EDITABLE(console->message.body), "");
	gtk_editable_set_text(GTK_EDITABLE(console->message.subject), "0");
	gtk_editable_set_text(GTK_EDITABLE(console->message.thread), "0");
	gtk_menu_button_popdown(console->message.button);
}

static void
dropdown_changed_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                    gpointer data)
{
	PidginXmppConsole *console = data;
	PidginAccountChooser *chooser = PIDGIN_ACCOUNT_CHOOSER(obj);
	PurpleAccount *account = NULL;

	account = pidgin_account_chooser_get_selected(chooser);
	if(PURPLE_IS_ACCOUNT(account)) {
		console->gc = purple_account_get_connection(account);
		gtk_text_buffer_set_text(console->buffer, "", 0);
	} else {
		GtkTextIter start, end;
		console->gc = NULL;
		gtk_text_buffer_set_text(console->buffer, _("Not connected to XMPP"), -1);
		gtk_text_buffer_get_bounds(console->buffer, &start, &end);
		gtk_text_buffer_apply_tag(console->buffer, console->tags.info, &start, &end);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_xmpp_console_class_finalize(G_GNUC_UNUSED PidginXmppConsoleClass *klass) {
}

static void
pidgin_xmpp_console_class_init(PidginXmppConsoleClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	        widget_class,
	        "/im/pidgin/Pidgin3/Plugin/XMPPConsole/console.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     account_chooser);
	gtk_widget_class_bind_template_callback(widget_class, dropdown_changed_cb);

	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     buffer);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.info);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.incoming);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.outgoing);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.bracket);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.tag);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.attr);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.value);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     tags.xmlns);

	/* Popover for <iq/> button. */
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     iq.button);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     iq.to);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     iq.type);
	gtk_widget_class_bind_template_callback(widget_class, iq_clicked_cb);

	/* Popover for <presence/> button. */
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     presence.button);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     presence.to);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     presence.type);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     presence.show);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     presence.status);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     presence.priority);
	gtk_widget_class_bind_template_callback(widget_class, presence_clicked_cb);

	/* Popover for <message/> button. */
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     message.button);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     message.to);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     message.type);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     message.body);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     message.subject);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     message.thread);
	gtk_widget_class_bind_template_callback(widget_class, message_clicked_cb);

	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     entry);
	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole,
	                                     entry_buffer);
	gtk_widget_class_bind_template_callback(widget_class, message_send_cb);

	gtk_widget_class_bind_template_child(widget_class, PidginXmppConsole, sw);
	gtk_widget_class_bind_template_callback(widget_class, entry_changed_cb);
}

static void
pidgin_xmpp_console_init(PidginXmppConsole *console) {
	gtk_widget_init_template(GTK_WIDGET(console));

	dropdown_changed_cb(G_OBJECT(console->account_chooser), NULL, console);
	entry_changed_cb(console->entry_buffer, console);

	gtk_widget_show(GTK_WIDGET(console));
}

/******************************************************************************
 * Plugin implementation
 *****************************************************************************/
static void
create_console(G_GNUC_UNUSED GSimpleAction *action,
               G_GNUC_UNUSED GVariant *parameter, G_GNUC_UNUSED gpointer data)
{
	if (console == NULL) {
		console = g_object_new(PIDGIN_TYPE_XMPP_CONSOLE, NULL);
		g_object_add_weak_pointer(G_OBJECT(console), (gpointer)&console);
	}

	gtk_window_present(GTK_WINDOW(console));
}

static GPluginPluginInfo *
xmpp_console_query(G_GNUC_UNUSED GError **error)
{
	GActionEntry entries[] = {
		{
			.name = "console",
			.activate = create_console,
		}
	};
	GMenu *menu = NULL;
	GSimpleActionGroup *group = NULL;
	const gchar * const authors[] = {
		"Sean Egan <seanegan@gmail.com>",
		NULL
	};

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries,
	                                G_N_ELEMENTS(entries), NULL);

	menu = g_menu_new();
	g_menu_append(menu, _("XMPP Console"), "console");

	return pidgin_plugin_info_new(
		"id",           PLUGIN_ID,
		"name",         N_("XMPP Console"),
		"version",      DISPLAY_VERSION,
		"category",     N_("Protocol utility"),
		"summary",      N_("Send and receive raw XMPP stanzas."),
		"description",  N_("This plugin is useful for debugging XMPP servers "
		                   "or clients."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"action-group", group,
		"action-menu",  menu,
		NULL
	);
}

static gboolean
xmpp_console_load(GPluginPlugin *plugin, GError **error)
{
	PurpleProtocolManager *manager = NULL;
	PurpleProtocol *xmpp = NULL;

	pidgin_xmpp_console_register_type(G_TYPE_MODULE(plugin));

	manager = purple_protocol_manager_get_default();
	xmpp = purple_protocol_manager_find(manager, "prpl-jabber");
	if (!PURPLE_IS_PROTOCOL(xmpp)) {
		g_set_error_literal(error, PLUGIN_DOMAIN, 0,
		                    _("No XMPP protocol is loaded."));
		return FALSE;
	}

	purple_signal_connect(xmpp, "jabber-receiving-xmlnode", plugin,
	                      G_CALLBACK(purple_xmlnode_received_cb), NULL);
	purple_signal_connect(xmpp, "jabber-sending-text", plugin,
	                      G_CALLBACK(purple_xmlnode_sent_cb), NULL);

	return TRUE;
}

static gboolean
xmpp_console_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                    G_GNUC_UNUSED gboolean shutdown,
                    G_GNUC_UNUSED GError **error)
{
	if (console) {
		gtk_window_destroy(GTK_WINDOW(console));
	}
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(xmpp_console)
