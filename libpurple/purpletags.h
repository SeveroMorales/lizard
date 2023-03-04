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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_TAGS_H
#define PURPLE_TAGS_H

#include <glib.h>
#include <gio/gio.h>

#define PURPLE_TYPE_TAGS (purple_tags_get_type())
G_DECLARE_FINAL_TYPE(PurpleTags, purple_tags, PURPLE, TAGS, GObject)

/**
 * PurpleTags:
 *
 * Tags is an object that can be used to keep track of arbitrary tags on
 * objects. Tags are simple strings that use the first ':' to delimit a value.
 * For example: `foo` is a tag with just a name and no value, but `foo:bar` is
 * a tag with a name and value. Please note this distinction when the API calls
 * for a name versus a tag which would be the name and the value.
 *
 * Since: 3.0.0
 */

G_BEGIN_DECLS

/**
 * purple_tags_new:
 *
 * Creates a new tags object.
 *
 * Returns: (transfer full): The new tags object.
 *
 * Since: 3.0.0
 */
PurpleTags *purple_tags_new(void);

/**
 * purple_tags_lookup:
 * @tags: The instance.
 * @name: The name of the tag to check if it exists.
 * @found: (out) (nullable): A return address for a boolean on whether the tag
 *         was found or not.
 *
 * Gets the value of @name if it exists in @tags.
 *
 * If @found is non %NULL, it will be set to %TRUE if @name was found.
 *
 * Returns: The value of the first tag matching @name. If there is no value,
 *          %NULL is returned.
 *
 * Since: 3.0.0
 */
const gchar *purple_tags_lookup(PurpleTags *tags, const gchar *name, gboolean *found);

/**
 * purple_tags_get:
 * @tags: The instance.
 * @name: The name of the tag to get.
 *
 * Gets the first tag that matches @name.
 *
 * Returns: The value of the first tag matching @name. If there is no value,
 *          %NULL is returned.
 *
 * Since: 3.0.0
 */
const gchar *purple_tags_get(PurpleTags *tags, const gchar *name);

/**
 * purple_tags_add:
 * @tags: The instance.
 * @tag: The tag data.
 *
 * Adds @tag to @tags. If the tag already exists, the existing tag will be
 * replaced.
 *
 * Since: 3.0.0
 */
void purple_tags_add(PurpleTags *tags, const gchar *tag);

/**
 * purple_tags_add_with_value:
 * @tags: The instance.
 * @name: The name of the tag.
 * @value: (nullable): The value of the tag.
 *
 * Formats @name and @value into a tag and adds it to @tags. If the tag already
 * exists, the existing tag will be replaced.
 *
 * Since: 3.0.0
 */
void purple_tags_add_with_value(PurpleTags *tags, const char *name, const char *value);

/**
 * purple_tags_remove:
 * @tags: The instance.
 * @tag: The tag data.
 *
 * Removes the first occurrence of @tag from @tags. Note that this is the tag
 * name and value not just the name.
 *
 * Returns: %TRUE if @tag was found and removed, otherwise %FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_tags_remove(PurpleTags *tags, const gchar *tag);

/**
 * purple_tags_get_count:
 * @tags: The instance.
 *
 * Gets the number of tags in @tags.
 *
 * Returns: The number of tags that @tags is keeping track of.
 *
 * Since: 3.0.0
 */
guint purple_tags_get_count(PurpleTags *tags);

/**
 * purple_tags_get_all:
 * @tags: The instance.
 *
 * Gets a list of all the tags.
 *
 * Returns: (transfer none) (element-type utf8): The list of all the tags.
 *
 * Since: 3.0.0
 */
GList *purple_tags_get_all(PurpleTags *tags);

/**
 * purple_tags_to_string:
 * @tags: The instance.
 * @separator: (nullable): A string to separate the items.
 *
 * Creates a @separator delimited string containing each item in @tags.
 *
 * Returns: (transfer full): The string representation.
 *
 * Since: 3.0.0
 */
gchar *purple_tags_to_string(PurpleTags *tags, const gchar *separator);

/**
 * purple_tag_parse:
 * @tag: The tag.
 * @name: (nullable) (optional) (out): An optional return address for the name
 *        of the tag.
 * @value: (nullable) (optional) (out): An optional return address for the
 *         value of the tag.
 *
 * Splits a tag into its name and value parts.
 *
 * Since: 3.0.0
 */
void purple_tag_parse(const char *tag, char **name, char **value);

G_END_DECLS

#endif /* PURPLE_TAGS_H */
