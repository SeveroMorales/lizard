/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2023 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatuattachment.h"
#include "talkatu/talkatuattachmentdialog.h"
#include "talkatu/talkatubuffer.h"
#include "talkatu/talkatuenums.h"
#include "talkatu/talkatulinkdialog.h"
#include "talkatu/talkatumarkup.h"
#include "talkatu/talkatumessage.h"
#include "talkatu/talkatusimpleattachment.h"
#include "talkatu/talkatutag.h"

/**
 * TalkatuActionGroup:
 *
 * A #GSimpleActionGroup subclass that contains all of the actions for
 * formatting text in Talkatu.  They are typically created by
 * #TalkatuBuffer subclasses.
 */

/**
 * TalkatuActionGroupClass:
 * @action_activated: The class handler for the
 *                    #TalkatuActionGroup::action-activated signal.
 *
 * The backing class of a #TalkatuActionGroup.
 */

/**
 * TALKATU_ACTION_FORMAT_BOLD:
 *
 * A constant that represents the bold font style action.
 */

/**
 * TALKATU_ACTION_FORMAT_ITALIC:
 *
 * A constant that represents the italic font style action.
 */

/**
 * TALKATU_ACTION_FORMAT_UNDERLINE:
 *
 * A constant that represents the underline font style action.
 */

/**
 * TALKATU_ACTION_FORMAT_STRIKETHROUGH:
 *
 * A constant that represents the strike through font style action.
 */

/**
 * TALKATU_ACTION_FORMAT_GROW:
 *
 * A constant that represents the increase font size action.
 */

/**
 * TALKATU_ACTION_FORMAT_SHRINK:
 *
 * A constant that represents the decrease font size action.
 */

/**
 * TALKATU_ACTION_FORMAT_RESET:
 *
 * A constant that represents the reset all formatting action.
 */

/**
 * TALKATU_ACTION_ATTACH_FILE:
 *
 * A constant that represents the attach file action.
 */

/**
 * TALKATU_ACTION_INSERT_LINK:
 *
 * A constant that presents the action to activate when the user wants to
 * insert a link.
 */

typedef struct {
	GtkTextBuffer *buffer;
	TalkatuFormat format;
	TalkatuInput *input;
} TalkatuActionGroupPrivate;

enum {
	PROP_0,
	PROP_BUFFER,
	PROP_FORMAT,
	PROP_INPUT,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL,};

enum {
	SIG_ACTION_ACTIVATED,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = {0, };

G_DEFINE_TYPE_WITH_PRIVATE(TalkatuActionGroup, talkatu_action_group, G_TYPE_SIMPLE_ACTION_GROUP)

/******************************************************************************
 * Helpers
 *****************************************************************************/
/*
 * talkatu_action_group_set_action_enabled:
 * @ag: The TalkatuActionGroup
 * @name: The name of the action.
 * @enabled: Whether or not to enable the action.
 *
 * Looks up the action with @name in @ag and sets its enabled property to
 * @enabled.
 */
static inline void
talkatu_action_group_set_action_enabled(TalkatuActionGroup *ag,
                                        const char *name,
                                        gboolean enabled)
{
	GAction *action = NULL;

	g_return_if_fail(TALKATU_IS_ACTION_GROUP(ag));
	g_return_if_fail(name != NULL);

	action = g_action_map_lookup_action(G_ACTION_MAP(ag), name);
	if(G_IS_SIMPLE_ACTION(action)) {
		g_simple_action_set_enabled(G_SIMPLE_ACTION(action), enabled);
	}
}

static void
talkatu_action_group_emit_action_activated(TalkatuActionGroup *ag, GAction *action) {
	gchar *name = NULL;

	g_object_get(G_OBJECT(action), "name", &name, NULL);
	g_signal_emit(ag, signals[SIG_ACTION_ACTIVATED], 0, action, name);
	g_free(name);
}

static void
talkatu_action_group_action_activated(GSimpleAction *action,
                                      G_GNUC_UNUSED GVariant *parameter,
                                      gpointer data)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);

	talkatu_action_group_emit_action_activated(ag, G_ACTION(action));
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_action_group_action_added(GActionGroup *ag,
                                  gchar *name,
                                  G_GNUC_UNUSED gpointer data)
{
	GAction *action = g_action_map_lookup_action(G_ACTION_MAP(ag), name);

	if(action != NULL) {
		guint sig_id = g_signal_connect(
			G_OBJECT(action),
			"activate",
			G_CALLBACK(talkatu_action_group_action_activated),
			ag
		);

		g_object_set_data(
			G_OBJECT(action),
			"talkatu-action-group-activate-signal-id",
			GUINT_TO_POINTER(sig_id)
		);
	}
}

static void
talkatu_action_group_action_removed(GActionGroup *ag,
                                    gchar *name,
                                    G_GNUC_UNUSED gpointer data)
{
	GAction *action = g_action_map_lookup_action(G_ACTION_MAP(ag), name);

	if(action != NULL) {
		guint sig_id = GPOINTER_TO_UINT(g_object_get_data(
			G_OBJECT(action),
			"talkatu-action-group-activate-signal-id"
		));
		g_signal_handler_disconnect(G_OBJECT(action), sig_id);
	}
}

static void
talkatu_action_activate(GSimpleAction *action,
                        G_GNUC_UNUSED GVariant *parameter,
                        G_GNUC_UNUSED gpointer user_data)
{
	g_message(_("activating action %s"), g_action_get_name(G_ACTION(action)));
}

static void
talkatu_action_toggle(GSimpleAction *action,
                      G_GNUC_UNUSED GVariant *parameter,
                      G_GNUC_UNUSED gpointer user_data)
{
	GVariant *state = NULL;

	g_message(_("toggling action %s"), g_action_get_name(G_ACTION(action)));

	state = g_action_get_state(G_ACTION(action));
	g_action_change_state(
		G_ACTION(action),
		g_variant_new_boolean(!g_variant_get_boolean(state))
	);
	g_variant_unref(state);
}

static void
talkatu_action_format_toggle(GSimpleAction *action, GVariant *state,
                             gpointer data)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);
	TalkatuActionGroupPrivate *priv = NULL;
	GtkTextIter start, end;
	TalkatuBufferStyle style;
	gboolean apply = FALSE;

	priv = talkatu_action_group_get_instance_private(ag);
	style = talkatu_buffer_get_style(TALKATU_BUFFER(priv->buffer));

	if(style == TALKATU_BUFFER_STYLE_WHOLE) {
		gtk_text_buffer_get_bounds(priv->buffer, &start, &end);
		apply = TRUE;
	} else {
		if(gtk_text_buffer_get_selection_bounds(priv->buffer, &start, &end)) {
			apply = TRUE;
		}
	}

	if(apply) {
		const gchar *tag_name = NULL, *name = NULL;

		name = g_action_get_name(G_ACTION(action));
		tag_name = talkatu_tag_name_for_action_name(name);

		if(tag_name == NULL) {
			return;
		}

		gtk_text_buffer_begin_user_action(priv->buffer);

		if(g_variant_get_boolean(state)) {
			gtk_text_buffer_apply_tag_by_name(priv->buffer, tag_name, &start, &end);
		} else {
			gtk_text_buffer_remove_tag_by_name(priv->buffer, tag_name, &start, &end);
		}

		gtk_text_buffer_end_user_action(priv->buffer);
	}

	g_simple_action_set_state(action, state);
}

static void
talkatu_action_reset_activate(G_GNUC_UNUSED GSimpleAction *act,
                              G_GNUC_UNUSED GVariant *parameter,
                              gpointer data)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);
	TalkatuActionGroupPrivate *priv = NULL;
	GtkTextIter start, end;
	TalkatuBufferStyle style;
	gchar **actions = NULL, **action = NULL;

	priv = talkatu_action_group_get_instance_private(ag);
	style = talkatu_buffer_get_style(TALKATU_BUFFER(priv->buffer));

	/* if there's no selection or this is a whole buffer format, select the
	 * whole buffer.
	 */
	if(!gtk_text_buffer_get_selection_bounds(priv->buffer, &start, &end) || style == TALKATU_BUFFER_STYLE_WHOLE) {
		gtk_text_buffer_get_bounds(priv->buffer, &start, &end);
		gtk_text_buffer_select_range(priv->buffer, &start, &end);
	}

	/* run through all of the talkatu tags and remove them from the
	 * selection.
	 */
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_BOLD, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_ITALIC, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_UNDERLINE, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_STRIKETHROUGH, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_SUBSCRIPT, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_SUPERSCRIPT, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_PRE, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_CODE, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_SEARCH, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_H1, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_H2, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_H3, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_H4, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_H5, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_H6, &start, &end);
	gtk_text_buffer_remove_tag_by_name(priv->buffer, TALKATU_TAG_ANCHOR, &start, &end);

	/* Now run through all the actions and deactivate any that are active by
	 * activating them.
	 */
	actions = g_action_group_list_actions(G_ACTION_GROUP(ag));
	for(action = actions; *action != NULL; action++) {
		GVariant *state = NULL;
		const GVariantType *state_type = NULL;
		gboolean enabled = FALSE;

		g_action_group_query_action(
			G_ACTION_GROUP(ag),
			*action,
			&enabled,
			NULL,
			&state_type,
			NULL,
			&state
		);

		if(state_type == NULL) {
			continue;
		}

		if(g_variant_type_equal(state_type, G_VARIANT_TYPE_BOOLEAN)) {
			if(g_variant_get_boolean(state)) {
				GAction *real_action = g_action_map_lookup_action(G_ACTION_MAP(ag), *action);
				g_action_activate(real_action, NULL);
			}
		}
	}
	g_strfreev(actions);
}

static void
talkatu_action_attach_file_attach_response_cb(GtkDialog *dialog, gint response,
                                              gpointer data)
{
	if(response == GTK_RESPONSE_CANCEL) {
		/* we call this separately for GTK_RESPONSE_CANCEL because
		 * GTK_RESPONSE_DELETE_EVENT already destroys the dialog.
		 */
		gtk_window_destroy(GTK_WINDOW(dialog));
	} else if(response == GTK_RESPONSE_ACCEPT) {
		TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);
		TalkatuActionGroupPrivate *priv = NULL;
		TalkatuAttachment *attachment = NULL;
		TalkatuAttachmentDialog *adialog = TALKATU_ATTACHMENT_DIALOG(dialog);
		const gchar *comment = NULL;

		priv = talkatu_action_group_get_instance_private(ag);

		/* Set the message to the comment from the dialog. */
		comment = talkatu_attachment_dialog_get_comment(adialog);
		talkatu_markup_set_html(TALKATU_BUFFER(priv->buffer), comment, -1);

		/* Add the attachment to the message */
		attachment = talkatu_attachment_dialog_get_attachment(adialog);
		talkatu_message_add_attachment(TALKATU_MESSAGE(priv->input), attachment);
		g_object_unref(G_OBJECT(attachment));

		/* Send the message! */
		talkatu_input_send_message(priv->input);

		gtk_window_destroy(GTK_WINDOW(dialog));
	}

	g_object_unref(dialog);
}

static void
talkatu_action_attach_file_response_cb(GtkDialog *dialog, gint response,
                                       gpointer data)
{
	if(response == GTK_RESPONSE_ACCEPT) {
		TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);
		TalkatuActionGroupPrivate *priv = NULL;
		TalkatuAttachment *attachment = NULL;
		GtkWidget *attach_dialog = NULL;
		GFile *file = NULL;
		gchar *filename = NULL, *content_type = NULL, *comment = NULL;

		priv = talkatu_action_group_get_instance_private(ag);

		file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		filename = g_file_get_path(file);
		content_type = g_content_type_guess(filename, NULL, 0, NULL);

		attachment = talkatu_simple_attachment_new(G_GUINT64_CONSTANT(0),
		                                           content_type);
		g_free(content_type);

		talkatu_attachment_set_local_uri(attachment, filename);
		g_free(filename);
		g_object_unref(file);

		comment = talkatu_markup_get_html(priv->buffer, NULL);
		attach_dialog = talkatu_attachment_dialog_new(attachment, comment);
		g_free(comment);

		g_signal_connect(G_OBJECT(attach_dialog), "response",
		                 G_CALLBACK(talkatu_action_attach_file_attach_response_cb),
		                 data);
		gtk_widget_show(attach_dialog);
	}

	gtk_native_dialog_destroy(GTK_NATIVE_DIALOG(dialog));
	g_object_unref(dialog);
}

static void
talkatu_action_attach_file(G_GNUC_UNUSED GSimpleAction *action,
                           G_GNUC_UNUSED GVariant *parameter,
                           gpointer data)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);
	TalkatuActionGroupPrivate *priv = NULL;
	GtkFileChooserNative *dialog = NULL;

	priv = talkatu_action_group_get_instance_private(ag);
	g_return_if_fail(TALKATU_IS_INPUT(priv->input));

	dialog = gtk_file_chooser_native_new(_("Attach file..."),
	                                     NULL,
	                                     GTK_FILE_CHOOSER_ACTION_OPEN,
	                                     _("Open"),
	                                     _("Cancel"));
	g_signal_connect(G_OBJECT(dialog), "response",
	                 G_CALLBACK(talkatu_action_attach_file_response_cb),
	                 ag);
	gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

static void
talkatu_action_insert_link_response_cb(GtkDialog *dialog, gint response,
                                       gpointer data)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(data);
	TalkatuActionGroupPrivate *priv = NULL;

	priv = talkatu_action_group_get_instance_private(ag);

	if(response == GTK_RESPONSE_ACCEPT) {
		GtkTextTagTable *table = gtk_text_buffer_get_tag_table(priv->buffer);
		GtkTextTag *anchor, *anchor_data;
		GtkTextMark *insert_mark = NULL;
		GtkTextIter insert;
		gchar *label = NULL, *url = NULL;

		/* if any text is selected, delete it */
		if(gtk_text_buffer_get_has_selection(priv->buffer)) {
			gtk_text_buffer_delete_selection(priv->buffer, TRUE, TRUE);
		}

		/* grab our inputs from the dialog */
		url = talkatu_link_dialog_get_url(TALKATU_LINK_DIALOG(dialog));
		label = talkatu_link_dialog_get_display_text(TALKATU_LINK_DIALOG(dialog));

		/* find the anchor tag from the table */
		anchor = gtk_text_tag_table_lookup(table, TALKATU_TAG_ANCHOR);
		g_return_if_fail(GTK_IS_TEXT_TAG(anchor));

		/* now create an anonymous tag that will hold the url.
		 * This should probably be dedupped at some point, but pidgin 2 didn't
		 * bother with that.
		 */
		anchor_data = gtk_text_tag_new(NULL);
		g_object_set_data_full(G_OBJECT(anchor_data), "talkatu-anchor-url",
		                       url, g_free);
		gtk_text_tag_table_add(table, anchor_data);

		insert_mark = gtk_text_buffer_get_insert(priv->buffer);
		gtk_text_buffer_get_iter_at_mark(priv->buffer, &insert, insert_mark);

		gtk_text_buffer_insert_with_tags(
			priv->buffer,
			&insert,
			label,
			-1,
			anchor,
			anchor_data,
			NULL
		);

		g_free(label);
	}

	gtk_window_destroy(GTK_WINDOW(dialog));
	g_object_unref(dialog);
}

static void
talkatu_action_insert_link(G_GNUC_UNUSED GSimpleAction *action,
                           G_GNUC_UNUSED GVariant *parameter,
                           gpointer data)
{
	GtkWidget *link_dialog = talkatu_link_dialog_new();

	g_signal_connect(G_OBJECT(link_dialog), "response",
			 G_CALLBACK(talkatu_action_insert_link_response_cb),
			 data);
	gtk_window_set_modal(GTK_WINDOW(link_dialog), TRUE);
	gtk_widget_show(link_dialog);
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_action_group_get_property(GObject *obj,
                                  guint prop_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(obj);

	switch(prop_id) {
		case PROP_BUFFER:
			g_value_set_object(value, talkatu_action_group_get_buffer(ag));
			break;
		case PROP_FORMAT:
			g_value_set_flags(value, talkatu_action_group_get_format(ag));
			break;
		case PROP_INPUT:
			g_value_set_object(value, talkatu_action_group_get_input(ag));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_action_group_set_property(GObject *obj,
                                  guint prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(obj);

	switch(prop_id) {
		case PROP_BUFFER:
			talkatu_action_group_set_buffer(ag, g_value_get_object(value));
			break;
		case PROP_FORMAT:
			talkatu_action_group_set_format(ag, g_value_get_flags(value));
			break;
		case PROP_INPUT:
			talkatu_action_group_set_input(ag, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_action_group_finalize(GObject *obj) {
	TalkatuActionGroupPrivate *priv = talkatu_action_group_get_instance_private(TALKATU_ACTION_GROUP(obj));

	g_clear_object(&priv->buffer);
	g_clear_object(&priv->input);

	G_OBJECT_CLASS(talkatu_action_group_parent_class)->finalize(obj);
}

static void
talkatu_action_group_constructed(GObject *object) {
	TalkatuActionGroup *ag = TALKATU_ACTION_GROUP(object);
	gsize i;
	GActionEntry entries[] = {
		{
			.name = TALKATU_ACTION_FORMAT_BOLD,
			.activate = talkatu_action_toggle,
			.state = "false",
			.change_state = talkatu_action_format_toggle
		}, {
			.name = TALKATU_ACTION_FORMAT_ITALIC,
			.activate = talkatu_action_toggle,
			.state = "false",
			.change_state = talkatu_action_format_toggle
		}, {
			.name = TALKATU_ACTION_FORMAT_UNDERLINE,
			.activate = talkatu_action_toggle,
			.state = "false",
			.change_state = talkatu_action_format_toggle
		}, {
			.name = TALKATU_ACTION_FORMAT_STRIKETHROUGH,
			.activate = talkatu_action_toggle,
			.state = "false",
			.change_state = talkatu_action_format_toggle
		}, {
			.name = TALKATU_ACTION_FORMAT_GROW,
			.activate = talkatu_action_activate,
		}, {
			.name = TALKATU_ACTION_FORMAT_SHRINK,
			.activate = talkatu_action_activate,
		}, {
			.name = TALKATU_ACTION_FORMAT_RESET,
			.activate = talkatu_action_reset_activate,
		}, {
			.name = TALKATU_ACTION_ATTACH_FILE,
			.activate = talkatu_action_attach_file,
		}, {
			.name = TALKATU_ACTION_INSERT_LINK,
			.activate = talkatu_action_insert_link,
			.state = "false",
			.change_state = talkatu_action_format_toggle,
		},
	};

	g_signal_connect(
		object,
		"action-added",
		G_CALLBACK(talkatu_action_group_action_added),
		NULL
	);

	g_signal_connect(
		object,
		"action-removed",
		G_CALLBACK(talkatu_action_group_action_removed),
		NULL
	);

	g_action_map_add_action_entries(G_ACTION_MAP(ag), entries,
	                                G_N_ELEMENTS(entries), ag);

	/* disable all of the actions by default */
	for(i = 0; i < G_N_ELEMENTS(entries); i++) {
		GAction *action = NULL;

		action = g_action_map_lookup_action(G_ACTION_MAP(ag), entries[i].name);
		g_simple_action_set_enabled(G_SIMPLE_ACTION(action), FALSE);
	}
}

static void
talkatu_action_group_init(G_GNUC_UNUSED TalkatuActionGroup *group) {
}

static void
talkatu_action_group_class_init(TalkatuActionGroupClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = talkatu_action_group_constructed;
	obj_class->get_property = talkatu_action_group_get_property;
	obj_class->set_property = talkatu_action_group_set_property;
	obj_class->finalize = talkatu_action_group_finalize;

	/* setup our properties */

	/**
	 * TalkatuActionGroup::buffer:
	 *
	 * The #TalkatuBuffer that this action group is tied to.
	 */
	properties[PROP_BUFFER] = g_param_spec_object(
		"buffer", "buffer", "The buffer to work on",
		TALKATU_TYPE_BUFFER,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
	);

	/**
	 * TalkatuActionGroup::format:
	 *
	 * The [flags@Talkatu.Format] to use to set which actions are enabled.
	 *
	 * Since: 0.2.0
	 */
	properties[PROP_FORMAT] = g_param_spec_flags(
		"format", "format",
		"The format options to enable",
		TALKATU_TYPE_FORMAT,
		TALKATU_FORMAT_NONE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * TalkatuActionGroup::input:
	 *
	 * The #TalkatuInput that this action group is tied to.
	 */
	properties[PROP_INPUT] = g_param_spec_object(
		"input", "input", "The input to work on",
		TALKATU_TYPE_INPUT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* setup our signals */

	/**
	 * TalkatuActionGroup::action-activated
	 * @talkatuactiongroup: The #TalkatuActionGroup instance.
	 * @arg1: The #GAction that was activated.
	 * @user_data: User supplied data.
	 *
	 * Emitted when one of the actions in @talkatuactiongroup are activated.
	 * This is a convenience signal so people don't have to connect to every
	 * action themselves.
	 */
	signals[SIG_ACTION_ACTIVATED] = g_signal_new(
		"action-activated",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(TalkatuActionGroupClass, action_activated),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		G_TYPE_ACTION,
		G_TYPE_STRING
	);
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * talkatu_action_group_new:
 * @buffer: The #GtkTextBuffer to bind this action group to.
 *
 * Creates a new #TalkatuActionGroup bound to @buffer.
 *
 * Returns: (transfer full): The new #TalkatuActionGroup instance.
 */
GSimpleActionGroup *
talkatu_action_group_new(TalkatuFormat format) {
	return g_object_new(
		TALKATU_TYPE_ACTION_GROUP,
		"format", format,
		NULL
	);
}

GSimpleActionGroup *
talkatu_action_group_new_with_buffer(TalkatuFormat format,
                                     GtkTextBuffer *buffer)
{
	return g_object_new(
		TALKATU_TYPE_ACTION_GROUP,
		"buffer", buffer,
		"format", format,
		NULL
	);
}

/**
 * talkatu_action_group_get_buffer:
 * @ag: The #TalkatuActionGroup instance.
 *
 * #TalkatuActionGroup's are bound to a specific #GtkTextBuffer.  This function
 * return the one that @ag is bound to.
 *
 * Returns: (transfer none): The #GtkTextBuffer that @ag is bound to.
 */
GtkTextBuffer *
talkatu_action_group_get_buffer(TalkatuActionGroup *ag) {
	TalkatuActionGroupPrivate *priv = NULL;

	g_return_val_if_fail(TALKATU_IS_ACTION_GROUP(ag), NULL);

	priv = talkatu_action_group_get_instance_private(ag);

	return priv->buffer;
}

/**
 * talkatu_action_group_set_buffer:
 * @ag: The instance.
 * @buffer: The buffer to use.
 *
 * Sets the [class@Gtk.TextBuffer] that @ag should use.
 *
 * Since: 0.2.0
 */
void
talkatu_action_group_set_buffer(TalkatuActionGroup *ag, GtkTextBuffer *buffer) {
	TalkatuActionGroupPrivate *priv = talkatu_action_group_get_instance_private(ag);

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));

	if(g_set_object(&priv->buffer, buffer)) {
		g_object_notify_by_pspec(G_OBJECT(ag), properties[PROP_BUFFER]);
	}
}

/**
 * talkatu_action_group_set_input:
 * @ag: The #TalkatuActionGroup instance.
 * @input: A #TalkatuInput instance or %NULL.
 *
 * Binds @ag to @input for actions that need to work against the input widget.
 */
void
talkatu_action_group_set_input(TalkatuActionGroup *ag, TalkatuInput *input) {
	TalkatuActionGroupPrivate *priv = NULL;

	g_return_if_fail(TALKATU_IS_ACTION_GROUP(ag));

	priv = talkatu_action_group_get_instance_private(ag);
	if(g_set_object(&priv->input, input)) {
		g_object_notify_by_pspec(G_OBJECT(ag), properties[PROP_INPUT]);
	}
}

/**
 * talkatu_action_group_get_input:
 * @ag: The #TalkatuActionGroup instance.
 *
 * Gets the #TalkatuInput bound to @ag.
 *
 * Returns: (transfer none): The #TalkatuInput that @ag is bound to.
 */
TalkatuInput *
talkatu_action_group_get_input(TalkatuActionGroup *ag) {
	TalkatuActionGroupPrivate *priv = NULL;

	g_return_val_if_fail(TALKATU_IS_ACTION_GROUP(ag), NULL);

	priv = talkatu_action_group_get_instance_private(ag);

	return priv->input;
}

/**
 * talkatu_action_name_for_tag_name:
 * @tag_name: The name of the tag to lookup.
 *
 * Looks up a #GAction for the the tag named @tag_name.
 *
 * Returns: The #GAction if one is found, otherwise %NULL.
 */
const gchar *
talkatu_action_name_for_tag_name(const gchar *tag_name) {
	if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_BOLD) == 0) {
		return TALKATU_ACTION_FORMAT_BOLD;
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_ITALIC) == 0) {
		return TALKATU_ACTION_FORMAT_ITALIC;
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_UNDERLINE) == 0) {
		return TALKATU_ACTION_FORMAT_UNDERLINE;
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_STRIKETHROUGH) == 0) {
		return TALKATU_ACTION_FORMAT_STRIKETHROUGH;
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_ANCHOR) == 0) {
		return TALKATU_ACTION_INSERT_LINK;
	}

	return NULL;
}

/**
 * talkatu_action_group_activate_format:
 * @ag: The #TalkatuActionGroup instance.
 * @format_name: The name of the format to activate.
 *
 * Activates action named @format_name.  This will apply it to a selection if
 * there is one.
 */
void
talkatu_action_group_activate_format(TalkatuActionGroup *ag, const gchar *format_name) {
	GAction *action = NULL;

	g_return_if_fail(TALKATU_IS_ACTION_GROUP(ag));

	action = g_action_map_lookup_action(G_ACTION_MAP(ag), format_name);
	if(action != NULL) {
		g_action_activate(action, NULL);
	}
}

/**
 * talkatu_action_group_get_action_activated:
 * @ag: The #TalkatuActionGroup instance.
 * @name: The name of the action to check.
 *
 * Returns that state of the action named @name.  If @name doesn't exist or is
 * not a toggle action, FALSE is returned.
 *
 * Returns: Whether or not @name is toggled.
 */
gboolean
talkatu_action_group_get_action_activated(TalkatuActionGroup *ag, const gchar *name) {
	GVariant *state = NULL;
	const GVariantType *state_type = NULL;
	gboolean enabled = FALSE;

	g_return_val_if_fail(TALKATU_IS_ACTION_GROUP(ag), FALSE);
	g_return_val_if_fail(name != NULL, FALSE);

	g_action_group_query_action(
		G_ACTION_GROUP(ag),
		name,
		&enabled,
		NULL,
		&state_type,
		NULL,
		&state
	);

	if(state_type == NULL) {
		return FALSE;
	}

	if(g_variant_type_equal(state_type, G_VARIANT_TYPE_BOOLEAN)) {
		return g_variant_get_boolean(state);
	}

	return FALSE;
}

/**
 * talkatu_action_group_get_activated_formats:
 * @ag: The #TalkatuActionGroup instance.
 *
 * Returns an array of all actions that are activated.  This values must be
 * free'd with g_strfreev.
 *
 * Returns: (transfer full): A list of actions that are activated.
 */
gchar **
talkatu_action_group_get_activated_formats(TalkatuActionGroup *ag) {
	GSList *activated = NULL, *l = NULL;
	gchar **actions = NULL, **action = NULL;
	gchar **ret = NULL;
	gint idx = 0;

	actions = g_action_group_list_actions(G_ACTION_GROUP(ag));
	for(action = actions; *action != NULL; action++) {
		if(talkatu_action_group_get_action_activated(ag, *action)) {
			activated = g_slist_prepend(activated, *action);
		} else {
			g_free(*action);
		}
	}
	g_free(actions);

	ret = g_new(gchar *, g_slist_length(activated) + 1);
	for(l = activated, idx = 0; l != NULL; l = l->next, idx++) {
		ret[idx] = (gchar *)l->data;
	}
	ret[idx] = NULL;
	g_slist_free(activated);

	return ret;
}

/**
 * talkatu_action_group_get_format:
 * @ag: The instance.
 *
 * Gets the [flags@Talkatu.Format] that @ag currently has enabled.
 *
 * Returns: The format options that are enabled.
 *
 * Since: 0.2.0
 */
TalkatuFormat
talkatu_action_group_get_format(TalkatuActionGroup *ag) {
	TalkatuActionGroupPrivate *priv = NULL;

	g_return_val_if_fail(TALKATU_IS_ACTION_GROUP(ag), TALKATU_FORMAT_NONE);

	priv = talkatu_action_group_get_instance_private(ag);

	return priv->format;
}

/**
 * talkatu_action_group_set_format:
 * @ag: The instance.
 * @format: The [flags@Talkatu.Format] to set as enabled.
 *
 * Sets the enabled actions of @ag to @format.
 *
 * Since: 0.2.0
 */
void
talkatu_action_group_set_format(TalkatuActionGroup *ag, TalkatuFormat format) {
	gboolean enabled = FALSE;

	g_return_if_fail(TALKATU_IS_ACTION_GROUP(ag));

	enabled = ((format & TALKATU_FORMAT_BOLD) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_FORMAT_BOLD,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_ITALIC) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_FORMAT_ITALIC,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_UNDERLINE) != 0);
	talkatu_action_group_set_action_enabled(ag,
	                                        TALKATU_ACTION_FORMAT_UNDERLINE,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_STRIKETHROUGH) != 0);
	talkatu_action_group_set_action_enabled(ag,
	                                        TALKATU_ACTION_FORMAT_STRIKETHROUGH,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_GROW) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_FORMAT_GROW,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_SHRINK) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_FORMAT_SHRINK,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_RESET) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_FORMAT_RESET,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_ATTACH_FILE) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_ATTACH_FILE,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_ATTACH_IMAGE) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_ATTACH_IMAGE,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_INSERT_LINK) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_INSERT_LINK,
	                                        enabled);

	enabled = ((format & TALKATU_FORMAT_INSERT_CODE) != 0);
	talkatu_action_group_set_action_enabled(ag, TALKATU_ACTION_INSERT_CODE,
	                                        enabled);
}
