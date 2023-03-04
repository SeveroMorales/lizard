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

#include <gumbo.h>

#include <talkatu/talkatucodeset.h>
#include <talkatu/talkatumarkup.h>
#include <talkatu/talkatutag.h>

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gchar *
talkatu_text_tag_get_name(GtkTextTag *tag) {
	gchar *name = NULL;

	g_object_get(G_OBJECT(tag), "name", &name, NULL);

	return name;
}

/******************************************************************************
 * HTML
 *****************************************************************************/
static gint
talkatu_markup_deserialize_html_helper(GtkTextBuffer *buffer,
                                       GtkTextIter *pos,
                                       GumboNode *node,
                                       GString *str)
{
	TalkatuTagDisplay display = TALKATU_TAG_DISPLAY_INLINE;
	GtkTextTagTable *table  = gtk_text_buffer_get_tag_table(buffer);
	GtkTextTag *tag = NULL;
	GumboVector *children = NULL;
	const gchar *tag_name = NULL;
	gsize i = 0;
	gint start_pos, length = 0;

	start_pos = gtk_text_iter_get_offset(pos);

	if(node->type == GUMBO_NODE_ELEMENT) {
		switch(node->v.element.tag) {
			case GUMBO_TAG_B:
				tag_name = TALKATU_TAG_BOLD;
				break;
			case GUMBO_TAG_CODE:
				tag_name = TALKATU_TAG_CODE;
				break;
			case GUMBO_TAG_I:
				tag_name = TALKATU_TAG_ITALIC;
				break;
			case GUMBO_TAG_U:
				tag_name = TALKATU_TAG_UNDERLINE;
				break;
			case GUMBO_TAG_EM:
				break;
			case GUMBO_TAG_STRIKE:
				tag_name = TALKATU_TAG_STRIKETHROUGH;
				break;
			case GUMBO_TAG_SUB:
				tag_name = TALKATU_TAG_SUBSCRIPT;
				break;
			case GUMBO_TAG_SUP:
				tag_name = TALKATU_TAG_SUPERSCRIPT;
				break;
			case GUMBO_TAG_PRE:
				tag_name = TALKATU_TAG_PRE;
				break;
			case GUMBO_TAG_H1:
				tag_name = TALKATU_TAG_H1;
				break;
			case GUMBO_TAG_H2:
				tag_name = TALKATU_TAG_H2;
				break;
			case GUMBO_TAG_H3:
				tag_name = TALKATU_TAG_H3;
				break;
			case GUMBO_TAG_H4:
				tag_name = TALKATU_TAG_H4;
				break;
			case GUMBO_TAG_H5:
				tag_name = TALKATU_TAG_H5;
				break;
			case GUMBO_TAG_H6:
				tag_name = TALKATU_TAG_H6;
				break;
			case GUMBO_TAG_A:
				tag_name = TALKATU_TAG_ANCHOR;
				break;
			case GUMBO_TAG_DL:
				tag_name = TALKATU_TAG_DL;
				break;
			case GUMBO_TAG_DT:
				tag_name = TALKATU_TAG_DT;
				break;
			case GUMBO_TAG_DD:
				tag_name = TALKATU_TAG_DD;
				break;
			case GUMBO_TAG_BR:
				talkatu_buffer_insert_markup(TALKATU_BUFFER(buffer), pos, "\n", -1);
				break;
			default:
				break;
		}
	}

	if(tag_name != NULL) {
		tag = gtk_text_tag_table_lookup(table, tag_name);

		if(TALKATU_IS_TAG(tag)) {
			GtkTextTag *prefix_tag = NULL;
			gchar *prefix_name = NULL;

			g_object_get(G_OBJECT(tag), "display", &display, NULL);

			prefix_name = g_strdup_printf("%s%s", tag_name, TALKATU_TAG_FORMATTING_START);
			prefix_tag = gtk_text_tag_table_lookup(table, prefix_name);
			g_free(prefix_name);

			if(GTK_IS_TEXT_TAG(prefix_tag)) {
				GtkTextIter s;

				talkatu_buffer_insert_markup(
					TALKATU_BUFFER(buffer),
					pos,
					"\u200B", // Zero Width Space
					-1
				);

				s = *pos;

				gtk_text_iter_backward_char(&s);
				gtk_text_buffer_apply_tag(
					GTK_TEXT_BUFFER(buffer),
					prefix_tag,
					&s,
					pos
				);
			}
		}
	}

	children = &node->v.element.children;

	for(i = 0; i < children->length; i++) {
		GumboNode *child = (GumboNode *)children->data[i];

		if(child->type == GUMBO_NODE_TEXT || child->type == GUMBO_NODE_WHITESPACE) {
			gint text_length = g_utf8_strlen(child->v.text.text, -1);

			/* add the text */
			talkatu_buffer_insert_markup(TALKATU_BUFFER(buffer), pos, child->v.text.text, -1);

			/* now adjust pos to the new insertion point */
			gtk_text_buffer_get_iter_at_offset(
				buffer,
				pos,
				gtk_text_iter_get_offset(pos) + text_length
			);

			length += text_length;

		} else if(child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {
			length += talkatu_markup_deserialize_html_helper(buffer, pos, child, str);
		}
	}

	if(tag) {
		GtkTextIter start, end;

		gtk_text_buffer_get_iter_at_offset(buffer, &start, start_pos);
		gtk_text_buffer_get_iter_at_offset(buffer, &end, start_pos + length);

		gtk_text_buffer_apply_tag(
			buffer,
			tag,
			&start,
			&end
		);

		if(TALKATU_IS_TAG(tag)) {
			GtkTextTag *suffix_tag = NULL;
			gchar *suffix_name = g_strdup_printf("%s%s", tag_name, TALKATU_TAG_FORMATTING_END);

			suffix_tag = gtk_text_tag_table_lookup(table, suffix_name);
			g_free(suffix_name);

			if(GTK_IS_TEXT_TAG(suffix_tag)) {
				GtkTextIter s;
				gchar *last = NULL;
				gint offset = 0;
				gboolean coalescing = FALSE;

				/* now determine if we need to coalesce newlines */
				s = *pos;
				gtk_text_iter_backward_char(pos);
				last = gtk_text_iter_get_text(pos, &s);

				if(*last == '\n') {
					coalescing = TRUE;
				}
				g_free(last);

				/* stash the current position in the buffer so we can get
				 * back to easily.
				 */
				offset = gtk_text_iter_get_offset(pos);

				/* move pos to the start of the line */
				gtk_text_iter_set_line_offset(pos, 0);

				/* if our iterator is the start of a word, apply the tag to
				 * that, if not insert a zero width space and apply to that.
				 */
				if(gtk_text_iter_starts_word(pos)) {
					s = *pos;
					gtk_text_iter_forward_char(pos);
				} else {
					/* now insert the zero width space */
					talkatu_buffer_insert_markup(
						TALKATU_BUFFER(buffer),
						pos,
						"\u200B",
						-1
					);

					/* now get an iterator to the start of the line */
					s = *pos;
					gtk_text_iter_backward_char(&s);

					/* increment offset to account for the space */
					offset += 1;
				}

				/* now apply the suffix tag */
				gtk_text_buffer_apply_tag(
					GTK_TEXT_BUFFER(buffer),
					suffix_tag,
					&s,
					pos
				);

				/* now move pos back to its original position plus one for
				 * the zero width space.
				 */
				gtk_text_iter_set_offset(pos, offset);

				/* if we're not coalescing newlines, add one */
				if(!coalescing) {
					talkatu_buffer_insert_markup(
						TALKATU_BUFFER(buffer),
						pos,
						"\n",
						-1
					);
				}
			} else if(display == TALKATU_TAG_DISPLAY_BLOCK) {
				talkatu_buffer_insert_markup(TALKATU_BUFFER(buffer), pos, "\n", -1);
			}
		}
	}

	return length;
}

/**
 * talkatu_markup_deserialize_html:
 * @register_buffer: The #GtkTextBuffer the format is registered with.
 * @content_buffer: The #GtkTextBuffer to deserialize into.
 * @iter: Insertion point for the deserialized text.
 * @data: The data to deserialize.
 * @length: The length of @data.
 * @create_tags: TRUE if deserializing may create tags
 * @user_data: User data that was specified when registering the format.
 * @error: Return location for a #GError.
 *
 * This is a #GtkTextBufferDeserializeFunc function that will deserialize
 * HTML data from @data into @content_buffer.  It should be registered with
 * with #gtk_text_buffer_register_deserializer_format in the buffer's
 * instance_init function.
 *
 * Returns: TRUE on success, or FALSE on error with @error set.
 */
gboolean
talkatu_markup_deserialize_html(GtkTextBuffer *register_buffer,
                                G_GNUC_UNUSED GtkTextBuffer *content_buffer,
                                GtkTextIter *iter,
                                const guint8 *data,
                                gsize length,
                                G_GNUC_UNUSED gboolean create_tags,
                                G_GNUC_UNUSED gpointer user_data,
                                GError **error)
{
	GError *real_error = NULL;
	gchar *text = NULL;
	gsize text_length;

	text = talkatu_codeset_coerce_utf8(data, length, &text_length, &real_error);
	if(real_error) {
		if(error) {
			*error = real_error;
		}

		return FALSE;
	}

	talkatu_markup_insert_html(
		TALKATU_BUFFER(register_buffer),
		iter,
		text,
		text_length
	);

	g_free(text);

	return TRUE;
}

/**
 * talkatu_markup_append_html:
 * @buffer: The #TalkatuBuffer instance.
 * @text: The text to append.
 * @len: The len of @text.
 *
 * Appends @text to @buffer at the current insertion point of @buffer.
 */
void
talkatu_markup_append_html(TalkatuBuffer *buffer, const gchar *text, gint len) {
	GtkTextMark *ins_mark;
	GtkTextIter ins_iter;

	ins_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(buffer));
	gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(buffer), &ins_iter, ins_mark);

	talkatu_markup_insert_html(buffer, &ins_iter, text, len);
}

/**
 * talkatu_markup_insert_html:
 * @buffer: The #TalkatuBuffer instance.
 * @iter: The insertion point for the deserialized text.
 * @text: The text to insert.
 * @len: The length of @text.
 *
 * Inserts @text in @buffer at @iter.
 */
void
talkatu_markup_insert_html(TalkatuBuffer *buffer,
                           GtkTextIter *iter,
                           const gchar *text,
                           G_GNUC_UNUSED gint len)
{
	GString *str = NULL;
	GumboOutput *output = NULL;

	output = gumbo_parse(text);

	str = g_string_new("");
	talkatu_markup_deserialize_html_helper(
		GTK_TEXT_BUFFER(buffer),
		iter,
		output->document,
		str
	);
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	talkatu_buffer_insert_markup(
		TALKATU_BUFFER(buffer),
		iter,
		str->str,
		str->len
	);

	g_string_free(str, TRUE);
}

/**
 * talkatu_markup_set_html:
 * @buffer: A #TalkatuBuffer instance
 * @text: The HTML text to set.
 * @len: The length of @text, or -1.
 *
 * Replaces all text in @buffer with @text.
 */
void
talkatu_markup_set_html(TalkatuBuffer *buffer, const gchar *text, gint len) {
	GtkTextIter start, end;

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(buffer), &start, &end);
	talkatu_markup_insert_html(buffer, &start, text, len);
}

/**
 * talkatu_markup_get_html_range:
 * @buffer: The #GtkTextBuffer instance.
 * @start: The starting point to get.
 * @end: The ending point to get.
 * @len: A return address for the length of text extracted.
 *
 * Extracts a section of @buffer into an HTML string.
 *
 * Returns: (transfer full): The HTML string.
 */
gchar *
talkatu_markup_get_html_range(G_GNUC_UNUSED GtkTextBuffer *buffer,
                              const GtkTextIter *start,
                              const GtkTextIter *end,
                              G_GNUC_UNUSED gsize *len)
{
	GSList *stack = NULL, *l = NULL;
	GtkTextIter *real_start = NULL, *real_end = NULL, *pos = NULL, *next = NULL;
	GString *str = g_string_new("");
	gunichar c;

	real_start = gtk_text_iter_copy(start);
	real_end = gtk_text_iter_copy(end);

	/* correctly order the iterators */
	gtk_text_iter_order(real_start, real_end);

	/* create a copy of the start one which is what we will use to walk the
	 * buffer.
	 */
	pos = gtk_text_iter_copy(real_start);

	/* create a look ahead iter to help us tell when tags start/end */
	next = gtk_text_iter_copy(pos);
	gtk_text_iter_forward_char(next);

	/* create the stack of tags */
	stack = gtk_text_iter_get_tags(pos);

	/* add any start tags that are open and don't start here */
	for(l = stack; l != NULL; l = l->next) {
		GtkTextTag *tag = GTK_TEXT_TAG(l->data);

		if(!gtk_text_iter_starts_tag(pos, tag)) {
			gchar *name = talkatu_text_tag_get_name(tag);
			const gchar *html_name = talkatu_tag_name_to_html(name);
			g_free(name);

			if(html_name != NULL) {
				g_string_append_printf(str, "<%s>", html_name);
			}
		}
	}

	/* now clear the stack so we have a clean starting point */
	g_slist_free(stack);
	stack = NULL;

	while((c = gtk_text_iter_get_char(pos)) != 0 && !gtk_text_iter_equal(pos, end)) {
		GSList *cur_tags = gtk_text_iter_get_tags(pos);

		/* open any tags starting here */
		for(l = cur_tags; l != NULL; l = l->next) {
			GtkTextTag *tag = GTK_TEXT_TAG(l->data);

			if(gtk_text_iter_starts_tag(pos, tag)) {
				gchar *name = talkatu_text_tag_get_name(tag);
				const gchar *html_name = talkatu_tag_name_to_html(name);
				g_free(name);

				if(html_name != NULL) {
					g_string_append_printf(str, "<%s>", html_name);
					stack = g_slist_prepend(stack, tag);
				}
			}
		}

		str = g_string_append_unichar(str, c);

		/* close any tags ending here */
		for(l = cur_tags; l != NULL; l = l->next) {
			GtkTextTag *tag = GTK_TEXT_TAG(l->data);

			if(!gtk_text_iter_has_tag(next, tag)) {
				gchar *name = talkatu_text_tag_get_name(tag);
				const gchar *html_name = talkatu_tag_name_to_html(name);
				g_free(name);

				if(html_name != NULL) {
					g_string_append_printf(str, "</%s>", html_name);
					stack = g_slist_remove(stack, tag);
				}
			}
		}

		gtk_text_iter_forward_char(pos);
		gtk_text_iter_forward_char(next);
	}

	/* close any tags remaining on the stack */
	for(l = stack; l != NULL; l = l->next) {
		GtkTextTag *tag = GTK_TEXT_TAG(l->data);

		if(!gtk_text_iter_ends_tag(real_end, tag)) {
			gchar *name = talkatu_text_tag_get_name(tag);
			const gchar *html_name = talkatu_tag_name_to_html(name);
			g_free(name);

			if(html_name != NULL) {
				g_string_append_printf(str, "</%s>", html_name);
			}
		}
	}
	g_slist_free(stack);


	gtk_text_iter_free(pos);
	gtk_text_iter_free(real_start);
	gtk_text_iter_free(real_end);

	return g_string_free(str, FALSE);
}

/**
 * talkatu_markup_serialize_html:
 * @register_buffer: The #GtkTextBuffer for which the format is registered.
 * @content_buffer: The #GtkTextBuffer to serialize.
 * @start: Start of the block of text to serialize.
 * @end: End of the block of text to serialize.
 * @length: Return location for the length of the serialized data.
 * @user_data: User data that was specified when registering the format.
 *
 * This is a #GtkTextBufferSerializeFunc that should be registered with
 * #gtk_text_buffer_register_serialize_func in the buffer's instance_init
 * method.
 *
 * Returns: (transfer full): The serialized HTML data.
 */
guint8 *
talkatu_markup_serialize_html(G_GNUC_UNUSED GtkTextBuffer *register_buffer,
                              GtkTextBuffer *content_buffer,
                              const GtkTextIter *start,
                              const GtkTextIter *end,
                              gsize *length,
                              G_GNUC_UNUSED gpointer user_data)
{
	gchar *html = NULL;

	html = talkatu_markup_get_html_range(content_buffer, start, end, length);

	return (guint8 *)html;
}

/**
 * talkatu_markup_get_html:
 * @buffer: The #GtkTextBuffer instance.
 * @len: A return address for the length of text extracted.
 *
 * Extracts all text from @buffer as an HTML string.
 *
 * Returns: (transfer full): The extracted HTML string.
 */
gchar *
talkatu_markup_get_html(GtkTextBuffer *buffer, gsize *len) {
	GtkTextIter start, end;

	gtk_text_buffer_get_bounds(buffer, &start, &end);

	return talkatu_markup_get_html_range(buffer, &start, &end, len);
}
