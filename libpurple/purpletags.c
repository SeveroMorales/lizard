/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#include "purpletags.h"

#include "util.h"

struct _PurpleTags {
	GObject parent;

	GList *tags;
};

G_DEFINE_TYPE(PurpleTags, purple_tags, G_TYPE_OBJECT)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/

static void
purple_tags_dispose(GObject *obj) {
	PurpleTags *tags = PURPLE_TAGS(obj);

	if(tags->tags != NULL) {
		g_list_free_full(tags->tags, g_free);
		tags->tags = NULL;
	}

	G_OBJECT_CLASS(purple_tags_parent_class)->dispose(obj);
}

static void
purple_tags_init(G_GNUC_UNUSED PurpleTags *tags) {
}

static void
purple_tags_class_init(PurpleTagsClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = purple_tags_dispose;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleTags *
purple_tags_new(void) {
	return g_object_new(PURPLE_TYPE_TAGS, NULL);
}

const gchar *
purple_tags_lookup(PurpleTags *tags, const gchar *name, gboolean *found) {
	size_t name_len = 0;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), FALSE);
	g_return_val_if_fail(name != NULL, FALSE);

	/* Assume we're going to find the tag, if we don't we set found to false
	 * before we return. This sounds silly, but it saves some additional logic
	 * below.
	 */
	if(found) {
		*found = TRUE;
	}

	name_len = strlen(name);

	for(GList *l = tags->tags; l != NULL; l = l->next) {
		const gchar *tag = l->data;

		if(g_str_has_prefix(tag, name)) {
			const gchar *value = tag + name_len;

			if(*value == '\0') {
				return NULL;
			} else if(*value == ':') {
				return value+1;
			}
		}
	}

	/* We didn't find the tag, so set found to false if necessary. */
	if(found) {
		*found = FALSE;
	}

	return NULL;
}

const gchar *
purple_tags_get(PurpleTags *tags, const gchar *name) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return purple_tags_lookup(tags, name, NULL);
}

void
purple_tags_add(PurpleTags *tags, const gchar *tag) {
	g_return_if_fail(PURPLE_IS_TAGS(tags));
	g_return_if_fail(tag != NULL);

	/* Remove any existing tags with this value. */
	purple_tags_remove(tags, tag);

	/* Add the new tag. */
	tags->tags = g_list_append(tags->tags, g_strdup(tag));
}

void
purple_tags_add_with_value(PurpleTags *tags, const char *name,
                           const char *value)
{
	char *tag = NULL;

	g_return_if_fail(PURPLE_IS_TAGS(tags));
	g_return_if_fail(name != NULL);

	if(value != NULL) {
		tag = g_strdup_printf("%s:%s", name, value);
	} else {
		tag = g_strdup(name);
	}

	purple_tags_add(tags, tag);

	g_free(tag);
}

gboolean
purple_tags_remove(PurpleTags *tags, const gchar *tag) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), FALSE);
	g_return_val_if_fail(tag != NULL, FALSE);

	for(GList *l = tags->tags; l != NULL; l = l->next) {
		gchar *etag = l->data;

		if(purple_strequal(etag, tag)) {
			g_free(etag);
			tags->tags = g_list_delete_link(tags->tags, l);

			return TRUE;
		}
	}

	return FALSE;
}

guint
purple_tags_get_count(PurpleTags *tags) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), 0);

	return g_list_length(tags->tags);
}

GList *
purple_tags_get_all(PurpleTags *tags) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);

	return tags->tags;
}

gchar *
purple_tags_to_string(PurpleTags *tags, const gchar *separator) {
	GString *value = NULL;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);

	value = g_string_new("");

	for(GList *l = tags->tags; l != NULL; l = l->next) {
		const gchar *tag = l->data;

		g_string_append(value, tag);

		if(separator != NULL && l->next != NULL) {
			g_string_append(value, separator);
		}
	}

	return g_string_free(value, FALSE);
}

void
purple_tag_parse(const char *tag, char **name, char **value) {
	const char *colon = NULL;

	g_return_if_fail(tag != NULL);

	colon = g_strstr_len(tag, -1, ":");
	if(colon == NULL) {
		if(name != NULL) {
			*name = g_strdup(tag);
		}
		if(value != NULL) {
			*value = NULL;
		}
	} else {
		if(name != NULL) {
			*name = g_strndup(tag, colon - tag);
		}
		if(value != NULL) {
			*value = g_strdup(colon + 1);
		}
	}
}
