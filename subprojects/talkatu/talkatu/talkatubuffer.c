/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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

#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include <talkatu/talkatuactiongroup.h>
#include <talkatu/talkatubuffer.h>
#include <talkatu/talkatuenums.h>
#include <talkatu/talkatutag.h>
#include <talkatu/talkatutagtable.h>

/**
 * TalkatuBuffer:
 *
 * A #GtkTextBuffer subclass that will automatically apply formatting according
 * to the actions in a #GSimpleAction group.
 */

/**
 * TalkatuBufferClass:
 * @insert_markup: The insert_markup vfunc is called to insert already rendered
 *                 text into the #TalkatuBuffer.
 * @create_action_group: The create_action_group vfunc is called to get a
 *                       #GSimpleAction group of actions that the
 *                       #TalkatuBuffer supports.
 *
 * The backing class to #TalkatuBuffer.
 */

/**
 * TALKATU_BUFFER_LINK_TARGET_ATTRIBUTE:
 *
 * The name of the attribute set on #GtkTextTags that contain the URL for
 * links.
 */

/**
 * TalkatuBufferStyle:
 * @TALKATU_BUFFER_STYLE_RICH: Format the buffer character by character.
 * @TALKATU_BUFFER_STYLE_WHOLE: Format the buffer as a whole.
 *
 * The format style of a #TalkatuBuffer.
 */

typedef struct _TalkatuBufferPrivate {
	TalkatuBufferStyle style;
	GSimpleActionGroup *action_group;

	GSList *tags;
} TalkatuBufferPrivate;

enum {
	PROP_0,
	PROP_STYLE,
	PROP_ACTION_GROUP,
	N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE_WITH_PRIVATE(TalkatuBuffer, talkatu_buffer, GTK_TYPE_TEXT_BUFFER);

#define TALKATU_BUFFER_MARK_LAST_INSERT "talkatu-last-insert"

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_buffer_action_toggled_cb(G_GNUC_UNUSED GActionGroup *action_group,
                                 gchar *name,
                                 GVariant *value,
                                 gpointer data)
{
	TalkatuBufferPrivate *priv = talkatu_buffer_get_instance_private(data);
	const gchar *tag_name = NULL;

	tag_name = talkatu_tag_name_for_action_name(name);
	if(tag_name == NULL) {
		return;
	}

	if(g_variant_get_boolean(value)) {
		/* apply the tag */
		priv->tags = g_slist_prepend(priv->tags, (gpointer)tag_name);
	} else {
		/* remove the tag */
		priv->tags = g_slist_remove(priv->tags, (gpointer)tag_name);
	}
}

/******************************************************************************
 * TalkatuBuffer Stuff
 *****************************************************************************/
static void
talkatu_buffer_real_insert_markup(TalkatuBuffer *buffer,
                                  GtkTextIter *pos,
                                  const gchar *new_text,
                                  gint new_text_length)
{
	GTK_TEXT_BUFFER_CLASS(talkatu_buffer_parent_class)->insert_text(
		GTK_TEXT_BUFFER(buffer),
		pos,
		new_text,
		new_text_length
	);
}

/******************************************************************************
 * GtkTextBuffer Stuff
 *****************************************************************************/
static void
talkatu_buffer_toggle_tag(G_GNUC_UNUSED GtkTextBuffer *buffer,
                          GSimpleActionGroup *action_group,
                          GtkTextTag *tag,
                          gboolean state)
{
	GAction *action = NULL;
	const gchar *action_name = NULL;
	gchar *name = NULL;

	/* GtkTextTag doesn't have an accessor for it's name property, so
	 * just grab it with g_object_get.
	 */
	g_object_get(G_OBJECT(tag), "name", &name, NULL);
	if(name == NULL) {
		return;
	}

	action_name = talkatu_action_name_for_tag_name(name);
	if(action_name == NULL) {
		g_warning(_("failed to find action for tag %s"), name);
		g_free(name);

		return;
	}
	g_free(name);

	action = g_action_map_lookup_action(G_ACTION_MAP(action_group), action_name);
	if(action != NULL) {
		g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_boolean(state));
	} else {
		g_warning(_("failed to find action %s"), action_name);
	}
}

static void
talkatu_buffer_insert_text(GtkTextBuffer *buffer,
                           GtkTextIter *pos,
                           const gchar *new_text,
                           gint new_text_length)
{
	TalkatuBufferPrivate *priv = NULL;
	GSList *tag = NULL;
	GtkTextIter *start = NULL;

	/* need to figure out if this is normal/expected... */
	if(new_text_length == 0) {
		g_message("called with empty text, determine if this is expected");
		return;
	}

	priv = talkatu_buffer_get_instance_private(TALKATU_BUFFER(buffer));

	/* chain up to the parent */
	GTK_TEXT_BUFFER_CLASS(talkatu_buffer_parent_class)->insert_text(
		buffer,
		pos,
		new_text,
		new_text_length
	);

	/* now apply the tags as necessary. pos has been revalidated */
	start = gtk_text_iter_copy(pos);
	gtk_text_iter_backward_chars(start, new_text_length);

	/* apply all tags to that text */
	for(tag = priv->tags; tag; tag = tag->next) {
		gtk_text_buffer_apply_tag_by_name(buffer, tag->data, start, pos);
	}

	gtk_text_iter_free(start);
}

static void
talkatu_buffer_mark_set(GtkTextBuffer      *buffer,
                        const GtkTextIter  *location,
                        GtkTextMark        *mark)
{
	TalkatuBufferPrivate *priv = NULL;
	GSList *tags = NULL, *tag = NULL;
	GtkTextMark *mark_last_insert = NULL;

	if(mark != gtk_text_buffer_get_insert(buffer)) {
		return;
	}

	priv = talkatu_buffer_get_instance_private(TALKATU_BUFFER(buffer));

	/* the insertion point has moved, reset all of the tags from the old
	 * insertion point to their defaults, which should be false.
	 */
	mark_last_insert = gtk_text_buffer_get_mark(buffer, TALKATU_BUFFER_MARK_LAST_INSERT);

	/* if the mark doesn't exist, create it */
	if(mark_last_insert == NULL) {
		mark_last_insert = gtk_text_buffer_create_mark(
			buffer,
			TALKATU_BUFFER_MARK_LAST_INSERT,
			location,
			FALSE
		);
	} else {
		/* we have a last insertion point, so we need to get the tags there
		 * and reset their states.
		 */

		GtkTextIter iter;

		gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark_last_insert);

		tags = gtk_text_iter_get_tags(&iter);
		for(tag = tags; tag; tag = tag->next) {
			talkatu_buffer_toggle_tag(buffer, priv->action_group, tag->data, FALSE);
		}
		g_slist_free(tags);
	}

	tags = gtk_text_iter_get_tags(location);
	for(tag = tags; tag; tag = tag->next) {
		talkatu_buffer_toggle_tag(buffer, priv->action_group, tag->data, TRUE);
	}

	gtk_text_buffer_move_mark(buffer, mark_last_insert, location);
}

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
talkatu_buffer_set_style(TalkatuBuffer *buffer, TalkatuBufferStyle style) {
	TalkatuBufferPrivate *priv = talkatu_buffer_get_instance_private(buffer);

	priv->style = style;

	g_object_notify(G_OBJECT(buffer), "style");
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_buffer_get_property(GObject *obj,
                            guint prop_id,
                            GValue *value,
                            GParamSpec *pspec)
{
	TalkatuBuffer *buffer = TALKATU_BUFFER(obj);

	switch(prop_id) {
		case PROP_STYLE:
			g_value_set_enum(value, talkatu_buffer_get_style(buffer));
			break;
		case PROP_ACTION_GROUP:
			g_value_set_object(value, talkatu_buffer_get_action_group(buffer));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_buffer_set_property(GObject *obj,
                            guint prop_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
	TalkatuBuffer *buffer = TALKATU_BUFFER(obj);

	switch(prop_id) {
		case PROP_STYLE:
			talkatu_buffer_set_style(buffer, g_value_get_enum(value));
			break;
		case PROP_ACTION_GROUP:
			talkatu_buffer_set_action_group(buffer, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static GObject *
talkatu_buffer_constructor(GType type,
                           guint n_params,
                           GObjectConstructParam *params)
{
	GObjectConstructParam *param;
	guint i = 0;

	for(i = 0, param = params; i < n_params; i++, param++) {
		const gchar *name = g_param_spec_get_name(param->pspec);
		if(g_ascii_strcasecmp("tag-table", name) == 0) {
			if(g_value_get_object(param->value) == NULL) {
				g_value_set_object(param->value, talkatu_tag_table_new());
			}
		}
	}

	return G_OBJECT_CLASS(talkatu_buffer_parent_class)->constructor(type, n_params, params);
}

static void
talkatu_buffer_finalize(GObject *obj) {
	TalkatuBufferPrivate *priv = talkatu_buffer_get_instance_private(TALKATU_BUFFER(obj));

	g_clear_object(&priv->action_group);

	G_OBJECT_CLASS(talkatu_buffer_parent_class)->finalize(obj);
}

static void
talkatu_buffer_init(G_GNUC_UNUSED TalkatuBuffer *buffer) {
	// gtk_text_buffer_register_serialize_tagset(GTK_TEXT_BUFFER(buffer), NULL);
	// gtk_text_buffer_register_deserialize_tagset(GTK_TEXT_BUFFER(buffer), NULL);
}

static void
talkatu_buffer_class_init(TalkatuBufferClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkTextBufferClass *text_buffer_class = GTK_TEXT_BUFFER_CLASS(klass);

	obj_class->get_property = talkatu_buffer_get_property;
	obj_class->set_property = talkatu_buffer_set_property;
	obj_class->constructor = talkatu_buffer_constructor;
	obj_class->finalize = talkatu_buffer_finalize;

	text_buffer_class->insert_text = talkatu_buffer_insert_text;
	text_buffer_class->mark_set = talkatu_buffer_mark_set;

	klass->insert_markup = talkatu_buffer_real_insert_markup;

	properties[PROP_STYLE] = g_param_spec_enum(
		"style", "style", "The format style of the buffer",
		TALKATU_TYPE_BUFFER_STYLE,
		TALKATU_BUFFER_STYLE_RICH,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);

	properties[PROP_ACTION_GROUP] = g_param_spec_object(
		"action-group", "action-group", "The action group for this buffer",
		G_TYPE_ACTION_GROUP,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_buffer_new:
 * @action_group: An optional #GSimpleAction group to use.
 *
 * This is a simple #GtkTextBuffer subclass that contains the shared behavior
 * for the other Talkatu text buffers.
 *
 * Returns: (transfer full): The #TalkatuBuffer instance.
 */
GtkTextBuffer *
talkatu_buffer_new(GSimpleActionGroup *action_group) {
	return GTK_TEXT_BUFFER(g_object_new(
		TALKATU_TYPE_BUFFER,
		"action-group", action_group,
		NULL
	));
}

/**
 * talkatu_buffer_get_style:
 * @buffer: A #TalkatuBuffer instance.
 *
 * Gets format style of @buffer.
 *
 * Returns: The format style of @buffer.
 */
TalkatuBufferStyle
talkatu_buffer_get_style(TalkatuBuffer *buffer) {
	TalkatuBufferPrivate *priv = NULL;

	g_return_val_if_fail(TALKATU_IS_BUFFER(buffer), TALKATU_BUFFER_STYLE_RICH);

	priv = talkatu_buffer_get_instance_private(buffer);

	return priv->style;
}

/**
 * talkatu_buffer_get_action_group:
 * @buffer: A #TalkatuBuffer instance.
 *
 * A #TalkatuBuffer can support multiple actions, whether it's formatting
 * of text, or being able to insert images, code, etc.  This function is called
 * by #TalkatuView to map them to keybindings as well as the format
 * toolbar.
 *
 * Returns: (transfer none): The #GSimpleActionGroup the @buffer supports.
 */
GSimpleActionGroup *
talkatu_buffer_get_action_group(TalkatuBuffer *buffer) {
	TalkatuBufferPrivate *priv = NULL;

	g_return_val_if_fail(TALKATU_IS_BUFFER(buffer), NULL);

	priv = talkatu_buffer_get_instance_private(buffer);

	return priv->action_group;
}

/**
 * talkatu_buffer_set_action_group:
 * @buffer: The instance.
 * @action_group: The new action group.
 *
 * Sets the [class@Gio.SimpleActionGroup] of @buffer to @action_group.
 * @action_group is used to determine what formatting tags are currently
 * enabled among other things.
 *
 * Since: 0.2.0
 */
void
talkatu_buffer_set_action_group(TalkatuBuffer *buffer,
                                GSimpleActionGroup *action_group)
{
	TalkatuBufferPrivate *priv = NULL;

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));

	priv = talkatu_buffer_get_instance_private(buffer);

	/* clear the tags */
	g_slist_free(priv->tags);
	priv->tags = NULL;

	/* clean up the old one */
	if(priv->action_group) {
		g_signal_handlers_disconnect_by_func(
			priv->action_group,
			talkatu_buffer_action_toggled_cb,
			buffer
		);

		g_object_unref(G_OBJECT(priv->action_group));

		priv->action_group = NULL;
	}

	if(G_IS_SIMPLE_ACTION_GROUP(action_group)) {
		priv->action_group = g_object_ref(action_group);
		g_signal_connect(
			action_group,
			"action-state-changed",
			G_CALLBACK(talkatu_buffer_action_toggled_cb),
			buffer
		);
	}
}

/**
 * talkatu_buffer_insert_markup:
 * @buffer: The #TalkatuBuffer instance.
 * @pos: The #GtkTextIter where the text should be inserted.
 * @new_text: The new text to insert.
 * @new_text_length: The len of @new_text.
 *
 * Inserts text that will be or already is marked up.  Calling this tells
 * @buffer to not apply the currently selected format to the newly inserted
 * text, which is what it does when text is normally inserted.
 */
void
talkatu_buffer_insert_markup(TalkatuBuffer *buffer,
                             GtkTextIter *pos,
                             const gchar *new_text,
                             gint new_text_length)
{
	TalkatuBufferClass *klass = NULL;

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));

	klass = TALKATU_BUFFER_GET_CLASS(buffer);
	if(klass && klass->insert_markup) {
		klass->insert_markup(buffer, pos, new_text, new_text_length);
	}
}

/**
 * talkatu_buffer_insert_markup_with_tags_by_name:
 * @buffer: The #TalkatuBuffer instance.
 * @pos: The #GtkTextIter where the text should be inserted.
 * @new_text: UTF-8 text.
 * @new_text_length: The len of @new_text, or -1.
 * @first_tag_name: The name of the first tag to apply to @new_text.
 * @...: Additional tags to apply to @new_text.
 *
 * Similar to #talkatu_buffer_insert_markup but allows you to specify
 * tags to apply to the newly inserted text.
 */
void
talkatu_buffer_insert_markup_with_tags_by_name(TalkatuBuffer *buffer,
                                               GtkTextIter *pos,
                                               const gchar *new_text,
                                               gint new_text_length,
                                               const gchar *first_tag_name,
                                               ...)
{
	GtkTextIter start;
	gint start_offset;
	va_list vargs;
	const gchar *tag_name = NULL;

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));
	g_return_if_fail(pos != NULL);
	g_return_if_fail(new_text != NULL);
	g_return_if_fail(gtk_text_iter_get_buffer(pos) == GTK_TEXT_BUFFER(buffer));

	start_offset = gtk_text_iter_get_offset(pos);

	talkatu_buffer_insert_markup(buffer, pos, new_text, new_text_length);

	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, start_offset);

	va_start(vargs, first_tag_name);

	tag_name = first_tag_name;
	while(tag_name != NULL) {
		gtk_text_buffer_apply_tag_by_name(
			GTK_TEXT_BUFFER(buffer),
			tag_name,
			&start,
			pos
		);

		tag_name = va_arg(vargs, const gchar *);
	}

	va_end(vargs);
}


/**
 * talkatu_buffer_insert_link:
 * @buffer: The #TalkatuBuffer instance.
 * @pos: The #GtkTextIter where to insert the link.
 * @display_text: The Text to display for the link.
 * @url: The url to link to.
 *
 * Inserts a link into @buffer with the given @url and @display_text.  If
 * @display_text is not given, @url will be used.
 */
void
talkatu_buffer_insert_link(TalkatuBuffer *buffer,
                           GtkTextIter *pos,
                           const gchar *display_text,
                           const gchar *url)
{
	GtkTextTagTable *tag_table = NULL;
	GtkTextTag *link_tag = NULL;
	GtkTextIter start;
	gint start_offset = 0;
	glong len = 0;

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));
	g_return_if_fail(pos != NULL);
	g_return_if_fail(url != NULL);
	g_return_if_fail(gtk_text_iter_get_buffer(pos) == GTK_TEXT_BUFFER(buffer));

	/* figure out what text we're displaying */
	if(display_text != NULL) {
		len = g_utf8_strlen(display_text, -1);
	}

	if(len == 0) {
		display_text = url;
	}

	/* insert the text and get the starting iterator */
	start_offset = gtk_text_iter_get_offset(pos);
	talkatu_buffer_insert_markup(buffer, pos, display_text, -1);
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, start_offset);

	/* create the link_tag */
	tag_table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer));
	link_tag = gtk_text_tag_new(NULL);
	g_object_set_data_full(
		G_OBJECT(link_tag),
		TALKATU_BUFFER_LINK_TARGET_ATTRIBUTE,
		g_strdup(url),
		g_free
	);

	gtk_text_tag_table_add(tag_table, link_tag);

	/* now apply the tags */
	gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(buffer), TALKATU_TAG_ANCHOR, &start, pos);
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), link_tag, &start, pos);
}


/**
 * talkatu_buffer_clear:
 * @buffer: The #TalkatuBuffer instance.
 *
 * Clears all text out of the buffer.
 */
void
talkatu_buffer_clear(TalkatuBuffer *buffer) {
	GtkTextIter start, end;

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(buffer), &start, &end);
}

/**
 * talkatu_buffer_get_plain_text:
 * @buffer: The #TalkatuBuffer instance.
 *
 * Returns the text from the buffer without markup.
 *
 * Returns (transfer full): A copy of the text from @buffer.
 */
gchar *
talkatu_buffer_get_plain_text(TalkatuBuffer *buffer) {
	GtkTextIter start, end;

	g_return_val_if_fail(TALKATU_IS_BUFFER(buffer), NULL);

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);

	return gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);
}

/**
 * talkatu_buffer_get_is_empty:
 * @buffer: The #TalkatuBuffer instance.
 *
 * Returns whether or not @buffer has any text in it.
 *
 * Returns: %TRUE if empty, %FALSE otherwise.
 */
gboolean
talkatu_buffer_get_is_empty(TalkatuBuffer *buffer) {
	GtkTextIter start, end;

	g_return_val_if_fail(TALKATU_IS_BUFFER(buffer), FALSE);

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);

	return gtk_text_iter_equal(&start, &end);
}
