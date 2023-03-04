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

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatutag.h"
#include "talkatu/talkatuenums.h"

/**
 * TALKATU_TAG_ANCHOR:
 *
 * A constant that represents an anchor or link font style.
 */

/**
 * TALKATU_TAG_AUTHOR:
 *
 * A constant for referencing the styling for an author.
 */

/**
 * TALKATU_TAG_BOLD:
 *
 * A constant that represents the bold font style.
 */

/**
 * TALKATU_TAG_CODE:
 *
 * A constant that represents a code font style.
 */

/**
 * TALKATU_TAG_CONTENTS:
 *
 * A constant for the tag that tags message contents in a #TalkatuHistory
 * buffer.
 */

/**
 * TALKATU_TAG_DD:
 *
 * A constant that represents the dd HTML element.
 */

/**
 * TALKATU_TAG_DL:
 *
 * A constant that represents the dl HTML element.
 */

/**
 * TALKATU_TAG_DT:
 *
 * A constant that reprensents the dt HTML element.
 */

/**
 * TALKATU_TAG_FORMATTING_END:
 *
 * A constant to be used as a suffix for tags that need formatting applied
 * after the original tag.
 */

/**
 * TALKATU_TAG_FORMATTING_START:
 *
 * A constant to be used as a prefix for tags that need formatting applied
 * before the original tag.
 */

/**
 * TALKATU_TAG_H1:
 *
 * A constant that represents the h1 header font style.
 */

/**
 * TALKATU_TAG_H2:
 *
 * A constant that represents the h2 header font style.
 */

/**
 * TALKATU_TAG_H3:
 *
 * A constant that represents the h3 header font style.
 */

/**
 * TALKATU_TAG_H4:
 *
 * A constant that represents the h4 header font style.
 */

/**
 * TALKATU_TAG_H5:
 *
 * A constant that represents the h5 header font style.
 */

/**
 * TALKATU_TAG_H6:
 *
 * A constant that represents the h6 header font style.
 */

/**
 * TALKATU_TAG_ITALIC:
 *
 * A constant that represents the italic font style.
 */

/**
 * TALKATU_TAG_MESSAGE:
 *
 * A constant for the tag that tags an entire message in a #TalkatuHistory
 * buffer.
 */

/**
 * TALKATU_TAG_PRE:
 *
 * A constant that represents a preformatted font style.
 */

/**
 * TALKATU_TAG_PREFIX:
 *
 * The prefix that all Talkatu tags use.
 */

/**
 * TALKATU_TAG_PREFIX_LEN:
 *
 * The length of #TALKATU_TAG_PREFIX for easy computation.
 */

/**
 * TALKATU_TAG_SEARCH:
 *
 * A constant that represents the highlighted search term font style.
 */

/**
 * TALKATU_TAG_STRIKETHROUGH:
 *
 * A constant that represents the strikethrough font style.
 */

/**
 * TALKATU_TAG_SUBSCRIPT:
 *
 * A constant that represents the sub-script font style.
 */

/**
 * TALKATU_TAG_SUPERSCRIPT:
 *
 * A constant that represents the super-script font style.
 */

/**
 * TALKATU_TAG_TIMESTAMP:
 *
 * A constant that represents the timestamp font style.
 */

/**
 * TALKATU_TAG_UNDERLINE:
 *
 * A constant that represents the underlined font style.
 */

/**
 * TalkatuTagDisplay:
 * @TALKATU_TAG_DISPLAY_INLINE: The tag should be rendered inline.
 * @TALKATU_TAG_DISPLAY_BLOCK: The tag should be rendered on its own line.
 *
 * An enum representing how a tag should be rendered.
 */

/**
 * TalkatuTag:
 *
 * TalkatuTag is a #GtkTextTag subclass that has a few additional properties
 * that allows greater control of how text is rendered.
 */
struct _TalkatuTag {
	GtkTextTag parent;

	TalkatuTagDisplay display;
};

G_DEFINE_TYPE(TalkatuTag, talkatu_tag, GTK_TYPE_TEXT_TAG);

enum {
	PROP_0 = 0,
	PROP_DISPLAY,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES];

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_tag_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
	TalkatuTag *tag = TALKATU_TAG(obj);

	switch(prop_id) {
		case PROP_DISPLAY:
			g_value_set_enum(value, talkatu_tag_get_display(tag));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_tag_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
	TalkatuTag *tag = TALKATU_TAG(obj);

	switch(prop_id) {
		case PROP_DISPLAY:
			talkatu_tag_set_display(tag, g_value_get_enum(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_tag_init(TalkatuTag *tag) {
	tag->display = TALKATU_TAG_DISPLAY_INLINE;
}

static void
talkatu_tag_class_init(TalkatuTagClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = talkatu_tag_get_property;
	obj_class->set_property = talkatu_tag_set_property;

	/* add our properties */
	properties[PROP_DISPLAY] = g_param_spec_enum(
		"display", "display", "How to display the text",
		TALKATU_TYPE_TAG_DISPLAY,
		TALKATU_TAG_DISPLAY_INLINE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_tag_new:
 * @name: The name to give the tag.
 * @first_property: The name of the first property to set.
 * @...: The value of the first property followed optionally by more name/value
 *       pairs, followed by NULL.
 *
 * Creates a new #TalkatuTag to be used for styling text.
 *
 * Returns: (transfer full): The new #TalkatuTag.
 */
GtkTextTag *
talkatu_tag_new(const gchar *name, const gchar *first_property, ...) {
	GtkTextTag *tag = NULL;
	va_list vargs;

	g_return_val_if_fail(name != NULL, NULL);

	tag = g_object_new(
		TALKATU_TYPE_TAG,
		"name", name,
		NULL
	);

	va_start(vargs, first_property);
	g_object_set_valist(G_OBJECT(tag), first_property, vargs);
	va_end(vargs);

	return tag;
}

/**
 * talkatu_tag_name_for_action_name:
 * @action_name: The name of the action to find a tag name for.
 *
 * Gets the tag name that should be used for @action_name.
 *
 * Returns: The tag name to be used for @action_name, or %NULL.
 */
const gchar *
talkatu_tag_name_for_action_name(const gchar *action_name) {
	if(g_ascii_strcasecmp(action_name, TALKATU_ACTION_FORMAT_BOLD) == 0) {
		return TALKATU_TAG_BOLD;
	} else if(g_ascii_strcasecmp(action_name, TALKATU_ACTION_FORMAT_ITALIC) == 0) {
		return TALKATU_TAG_ITALIC;
	} else if(g_ascii_strcasecmp(action_name, TALKATU_ACTION_FORMAT_UNDERLINE) == 0) {
		return TALKATU_TAG_UNDERLINE;
	} else if(g_ascii_strcasecmp(action_name, TALKATU_ACTION_FORMAT_STRIKETHROUGH) == 0) {
		return TALKATU_TAG_STRIKETHROUGH;
	}

	return NULL;
}

/**
 * talkatu_tag_name_to_html:
 * @tag_name: The name of the tag.
 *
 * Determines the HTML tag for @tag_name.
 *
 * Returns: The HTML tag if any for @tag_name.
 */
const gchar *
talkatu_tag_name_to_html(const gchar *tag_name) {
	if(tag_name == NULL) {
		return NULL;
	}

	if(g_ascii_strncasecmp(TALKATU_TAG_PREFIX, tag_name, TALKATU_TAG_PREFIX_LEN) != 0) {
		return NULL;
	}

	if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_BOLD) == 0) {
		return "b";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_ITALIC) == 0) {
		return "i";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_UNDERLINE) == 0) {
		return "u";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_STRIKETHROUGH) == 0) {
		return "s";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_SUBSCRIPT) == 0) {
		return "sub";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_SUPERSCRIPT) == 0) {
		return "sup";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_PRE) == 0) {
		return "pre";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_H1) == 0) {
		return "h1";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_H2) == 0) {
		return "h2";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_H3) == 0) {
		return "h3";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_H4) == 0) {
		return "h4";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_H5) == 0) {
		return "h5";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_H6) == 0) {
		return "h6";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_ANCHOR) == 0) {
		return "a";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_DL) == 0) {
		return "dl";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_DT) == 0) {
		return "dt";
	} else if(g_ascii_strcasecmp(tag_name, TALKATU_TAG_DD) == 0) {
		return "dd";
	}

	return NULL;
}

/**
 * talkatu_tag_get_display:
 * @tag: The #TalkatuTag instance.
 *
 * Gets the #TalkatuTagDisplay for @tag.
 *
 * Returns: The #TalkatuTagDisplay for @tag.
 */
TalkatuTagDisplay
talkatu_tag_get_display(TalkatuTag *tag) {
	g_return_val_if_fail(TALKATU_IS_TAG(tag), TALKATU_TAG_DISPLAY_INLINE);

	return tag->display;
}

/**
 * talkatu_tag_set_display:
 * @tag: The #TalkatuTag instance.
 * @display: The #TalkatuTagDisplay value to set.
 *
 * Sets the display type for @tag to @display.
 */
void
talkatu_tag_set_display(TalkatuTag *tag, TalkatuTagDisplay display) {
	g_return_if_fail(TALKATU_IS_TAG(tag));

	tag->display = display;

	g_object_notify_by_pspec(G_OBJECT(tag), properties[PROP_DISPLAY]);
}
