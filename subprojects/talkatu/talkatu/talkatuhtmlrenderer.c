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

#include "talkatuhtmlrenderer.h"

/**
 * TalkatuHtmlRendererClass:
 * @reset: The method to call to reset the renderer.  This allows the renderer
 *         to be reused.
 * @element_start: The method to call when an element is found.  The attribute
 *                 names and values are passed in as a %NULL terminated array of
 *                 strings.
 * @element_finish: The method to call when all children of an element have been
 *                  processed.
 * @text: The method to call when can text or character data is found.
 * @comment: The method to call when a comment is found.  The passed in comment
 *           is the contents only and does not contain the start (<!--) and end
 *           (-->) tags.
 *
 * An abstract class that will walk an HTML document and call the instance
 * methods of the child class for each node that is found.
 */

#define _GUMBO_NODE_IS_CONTAINER(node) \
	((node)->type == GUMBO_NODE_ELEMENT || \
	 (node)->type == GUMBO_NODE_TEMPLATE)

static GumboNode *
talkatu_html_renderer_find_next_sibling(GumboNode *node) {
	GumboNode *next = NULL;

	if(node->parent == NULL) {
		return NULL;
	}

	/* As long as we have a parent, we can use it with our node's
	 * `index_within_parent` to figure out if we have any more siblings.
	 */
	if(_GUMBO_NODE_IS_CONTAINER(node->parent)) {
		GumboElement element = node->parent->v.element;

		if(node->index_within_parent != element.children.length - 1) {
			next = element.children.data[node->index_within_parent+1];
		}
	}

	return next;
}

/******************************************************************************
 * Helper Implementations
 *****************************************************************************/
static void
talkatu_html_renderer_element_start(TalkatuHtmlRenderer *renderer,
                                    const gchar *name,
                                    GumboElement *element)
{
	TalkatuHtmlRendererClass *klass = TALKATU_HTML_RENDERER_GET_CLASS(renderer);

	if(klass->element_start) {
		const gchar **names = NULL, **values = NULL;
		guint length = element->attributes.length;

		if(length > 0) {
			guint i = 0;

			names = g_new(const gchar *, length + 1);
			values = g_new(const gchar *, length + 1);

			for(i = 0; i < length; i++) {
				GumboAttribute *attr = NULL;

				attr = (GumboAttribute *)element->attributes.data[i];

				names[i] = attr->name;
				values[i] = attr->value;
			}

			/* add our terminating null values to the end */
			names[i] = NULL;
			values[i] = NULL;
		}

		klass->element_start(renderer, name, names, values);

		g_free(names);
		g_free(values);
	}
}

static void
talkatu_html_renderer_element_finish(TalkatuHtmlRenderer *renderer,
                                     const gchar *name)
{
	TalkatuHtmlRendererClass *klass = TALKATU_HTML_RENDERER_GET_CLASS(renderer);

	if(klass->element_finish) {
		klass->element_finish(renderer, name);
	}
}

static void
talkatu_html_renderer_text(TalkatuHtmlRenderer *renderer, const gchar *text) {
	TalkatuHtmlRendererClass *klass = TALKATU_HTML_RENDERER_GET_CLASS(renderer);

	if(klass->text) {
		klass->text(renderer, text);
	}
}

static void
talkatu_html_renderer_comment(TalkatuHtmlRenderer *renderer,
                              const gchar *comment)
{
	TalkatuHtmlRendererClass *klass = TALKATU_HTML_RENDERER_GET_CLASS(renderer);

	if(klass->comment) {
		klass->comment(renderer, comment);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_ABSTRACT_TYPE(TalkatuHtmlRenderer, talkatu_html_renderer,
                       G_TYPE_OBJECT)

static void
talkatu_html_renderer_init(G_GNUC_UNUSED TalkatuHtmlRenderer *renderer) {
}

static void
talkatu_html_renderer_class_init(G_GNUC_UNUSED TalkatuHtmlRendererClass *klass) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_html_renderer_render:
 * @renderer: The #TalkatuHtmlRenderer instance.
 * @html: The HTML text to render.
 *
 * Renders the given @html calling the #TalkatuHtmlRendererClass functions as
 * necessary.
 */
void
talkatu_html_renderer_render(TalkatuHtmlRenderer *renderer, const gchar *html) {
	GList *stack = NULL;
	GumboOutput *output = NULL;

	output = gumbo_parse(html);

	stack = g_list_prepend(stack, output->root);

	/* We create a stack with the first node and then process according to the
	 * node type.
	 *
	 * For non-element nodes, we call the text/comment function as appropriate
	 * and then look for their siblings.  If the node has a sibling, we remove
	 * the current node from the stack and replace it with its sibling.
	 *
	 * For element nodes, we call element_start and push the first child to the
	 * stack if the node has children and immediately start processing the
	 * child.  If the element does not have children, we call element_finish,
	 * remove it from the stack and look for a sibling to push to the stack.
	 *
	 * If the node does not have a sibling, we call element_finish on its parent
	 * and remove it from the stack.  Then we check for its parent and repeat
	 * the process until we have found a sibling or have exhausted the stack.
	 */

	while(stack != NULL) {
		GumboNode *node = (GumboNode *)stack->data;
		GumboNode *next = NULL;
		const gchar *tagname = NULL;

		switch(node->type) {
			case GUMBO_NODE_DOCUMENT:
				/* this is here to stop a warning from gcc.  We could add a
				 * default case, but then if a new type is added or something we
				 * would mask the warning that it would generate.
				 */
				break;
			case GUMBO_NODE_ELEMENT:
			case GUMBO_NODE_TEMPLATE:
				tagname = gumbo_normalized_tagname(node->v.element.tag);
				talkatu_html_renderer_element_start(renderer, tagname,
				                                    &node->v.element);

				if(node->v.element.children.length > 0) {
					/* if we have at least one child, we throw it on the stack
					 * and start processing that node.
					 */
					node = (GumboNode *)(&node->v.element.children)->data[0];
					stack = g_list_prepend(stack, node);

					continue;
				} else {
					/* We have no children so we just call the finish method. */
					talkatu_html_renderer_element_finish(renderer, tagname);
				}
				break;
			case GUMBO_NODE_CDATA:
			case GUMBO_NODE_TEXT:
			case GUMBO_NODE_WHITESPACE:
				talkatu_html_renderer_text(renderer, node->v.text.text);
				break;
			case GUMBO_NODE_COMMENT:
				talkatu_html_renderer_comment(renderer, node->v.text.text);
				break;
		}

		/* check if we have a sibling */
		next = talkatu_html_renderer_find_next_sibling(node);

		/* remove the node from the stack */
		stack = g_list_remove(stack, node);

		/* if the node was the last, we need to end element_finish for the
		 * parent and pop the parent from the stack as well.
		 */
		if(next != NULL) {
			stack = g_list_prepend(stack, next);
		} else if(node->parent != NULL && _GUMBO_NODE_IS_CONTAINER(node->parent)) {
			/* Our node has no other siblings, so we need to finish the parent
			 * element.
			 */
			GumboElement parent_element = node->parent->v.element;
			tagname = gumbo_normalized_tagname(parent_element.tag);

			talkatu_html_renderer_element_finish(renderer, tagname);

			/* while we still have elements on the list, pop them off until we
			 * find one that still has children we haven't visited yet.
			 */
			while(stack != NULL) {
				GumboElement element;

				node = (GumboNode *)stack->data;

				next = talkatu_html_renderer_find_next_sibling(node);

				if(next != NULL) {
					/* we found a sibling, so drop the top most item and
					 * put the sibling on the top of the stack.
					 */
					stack = g_list_remove(stack, node);
					stack = g_list_prepend(stack, next);

					break;
				}

				if(node->parent->type != GUMBO_NODE_DOCUMENT) {
					element = node->parent->v.element;
					tagname = gumbo_normalized_tagname(element.tag);

					talkatu_html_renderer_element_finish(renderer, tagname);
				}

				/* If this node doesn't have a sibling, then pop it off the
				 * stack.
				 */
				stack = g_list_remove(stack, node);
			}
		}
	}

	gumbo_destroy_output(&kGumboDefaultOptions, output);
}

/**
 * talkatu_html_renderer_reset:
 * @renderer: The #TalkatuHtmlRenderer instance.
 *
 * Resets @renderer back to a clean state so that it can render new HTML.
 */
void
talkatu_html_renderer_reset(TalkatuHtmlRenderer *renderer) {
	TalkatuHtmlRendererClass *klass = NULL;

	g_return_if_fail(TALKATU_IS_HTML_RENDERER(renderer));

	klass = TALKATU_HTML_RENDERER_GET_CLASS(renderer);
	if(klass && klass->reset) {
		klass->reset(renderer);
	}
}
