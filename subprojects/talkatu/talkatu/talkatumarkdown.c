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

#include <cmark.h>

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatubuffer.h"
#include "talkatu/talkatucodeset.h"
#include "talkatu/talkatumarkdown.h"
#include "talkatu/talkatutag.h"
#include "talkatu/talkatutagtable.h"

/******************************************************************************
 * Helpers
 *****************************************************************************/
gboolean
talkatu_markdown_buffer_deserialize_markdown(GtkTextBuffer *register_buffer,
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

	talkatu_markdown_insert(TALKATU_BUFFER(register_buffer), iter, text,
	                        text_length);

	g_free(text);

	return TRUE;
}

static const gchar *
talkatu_markdown_buffer_get_tag_for_heading(cmark_node *node) {
	const gchar *tag_name = NULL;

	switch(cmark_node_get_heading_level(node)) {
		case 2:
			tag_name = TALKATU_TAG_H2;
			break;
		case 3:
			tag_name = TALKATU_TAG_H3;
			break;
		case 4:
			tag_name = TALKATU_TAG_H4;
			break;
		case 5:
			tag_name = TALKATU_TAG_H5;
			break;
		case 6:
			tag_name = TALKATU_TAG_H6;
			break;
		case 1:
		default:
			tag_name = TALKATU_TAG_H1;
			break;
	}

	return tag_name;
}

/******************************************************************************
 * Markdown Stuff
 *****************************************************************************/

/**
 * talkatu_markdown_insert:
 * @buffer: The instance.
 * @iter: The #GtkTextIter where the text should be inserted.
 * @text: The UTF-8 Markdown text that should be inserted.
 * @len: The length of @text or -1.
 *
 * Inserts and renders @text into @buffer.
 */
void
talkatu_markdown_insert(TalkatuBuffer *buffer, GtkTextIter *iter,
                        const gchar *text, gint len)
{
	GtkTextTagTable *tag_table = NULL;
	gint start_offset = -1;
	cmark_node *root = NULL;
	cmark_iter *mditer = NULL;
	cmark_event_type ev_type;

	g_return_if_fail(TALKATU_IS_BUFFER(buffer));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(text != NULL);

	tag_table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer));

	root = cmark_parse_document(text, len, CMARK_OPT_HARDBREAKS | CMARK_OPT_SAFE);
	mditer = cmark_iter_new(root);

	while((ev_type = cmark_iter_next(mditer)) != CMARK_EVENT_DONE) {
		cmark_node *node = cmark_iter_get_node(mditer);
		cmark_node_type node_type = cmark_node_get_type(node);

		if(node_type == CMARK_NODE_TEXT) {
			talkatu_buffer_insert_markup(
				TALKATU_BUFFER(buffer),
				iter,
				cmark_node_get_literal(node),
				-1
			);
		} else if(node_type == CMARK_NODE_CODE || node_type == CMARK_NODE_CODE_BLOCK) {
			talkatu_buffer_insert_markup_with_tags_by_name(
				TALKATU_BUFFER(buffer),
				iter,
				cmark_node_get_literal(node),
				-1,
				TALKATU_TAG_CODE,
				NULL
			);
		} else if(ev_type == CMARK_EVENT_ENTER) {
			start_offset = gtk_text_iter_get_offset(iter);
			g_message("enter node %s", cmark_node_get_type_string(node));
		} else if(ev_type == CMARK_EVENT_EXIT) {
			GSList *tags = NULL, *tag = NULL;
			const gchar *tag_name = NULL;

			if(start_offset == -1) {
				continue;
			}

			if(node_type == CMARK_NODE_EMPH) {
				tag_name = TALKATU_TAG_ITALIC;
			} else if(node_type == CMARK_NODE_STRONG) {
				tag_name = TALKATU_TAG_BOLD;
			} else if(node_type == CMARK_NODE_HEADING) {
				tag_name = talkatu_markdown_buffer_get_tag_for_heading(node);
			} else if(node_type == CMARK_NODE_LINK) {
				GtkTextTag *link_url = gtk_text_tag_new(NULL);

				tag_name = TALKATU_TAG_ANCHOR;

				g_object_set_data_full(
					G_OBJECT(link_url),
					"talkatu-anchor-url",
					g_strdup(cmark_node_get_url(node)),
					g_free
				);

				gtk_text_tag_table_add(tag_table, link_url);

				g_message("link url: %s", cmark_node_get_url(node));

				tags = g_slist_append(tags, link_url);
			} else if(node_type == CMARK_NODE_PARAGRAPH) {
				talkatu_buffer_insert_markup(
					TALKATU_BUFFER(buffer),
					iter,
					"\n",
					-1
				);
			} else {
				g_warning("unknown node type %s", cmark_node_get_type_string(node));
			}

			if(tag_name || tags != NULL) {
				GtkTextIter start_iter;

				gtk_text_buffer_get_iter_at_offset(
					GTK_TEXT_BUFFER(buffer),
					&start_iter,
					start_offset
				);

				if(tag_name) {
					gtk_text_buffer_apply_tag_by_name(
						GTK_TEXT_BUFFER(buffer),
						tag_name,
						&start_iter,
						iter
					);
				}

				if(tags) {
					for(tag = tags; tag; tag = tag->next) {
						gtk_text_buffer_apply_tag(
							GTK_TEXT_BUFFER(buffer),
							GTK_TEXT_TAG(tag->data),
							&start_iter,
							iter
						);
					}

					g_slist_free(tags);
				}
			}

			/* now that we're done applying tags, if we were in a block element
			 * add a newline.
			 */
			g_message("exit node %s", cmark_node_get_type_string(node));
			if(node_type >= CMARK_NODE_FIRST_BLOCK && node_type <= CMARK_NODE_LAST_BLOCK) {
				g_message("finishing block node %s", cmark_node_get_type_string(node));
				talkatu_buffer_insert_markup(
					TALKATU_BUFFER(buffer),
					iter,
					"\n",
					-1
				);
			}

			start_offset = -1;
		}
	}

	cmark_iter_free(mditer);
	cmark_node_free(root);
}
