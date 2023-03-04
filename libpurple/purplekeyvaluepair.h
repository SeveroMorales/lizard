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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_KEY_VALUE_PAIR_H
#define PURPLE_KEY_VALUE_PAIR_H

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_KEY_VALUE_PAIR (purple_key_value_pair_get_type())

/**
 * PurpleKeyValuePair:
 * @key: The key
 * @value: The value
 * @value_destroy_func: A #GDestroyNotify to free @value when
 *                      purple_key_value_pair_free() is called.
 *
 * A key-value pair.
 *
 * This is used by, among other things, purple_gtk_combo* functions to pass in
 * a list of key-value pairs so it can display a user-friendly value.
 */
typedef struct _PurpleKeyValuePair PurpleKeyValuePair;

G_BEGIN_DECLS

struct _PurpleKeyValuePair {
	gchar *key;
	gpointer value;
	GDestroyNotify value_destroy_func;
};

/**
 * purple_key_value_pair_get_type:
 *
 * The standard %_GET_TYPE function for #PurpleKeyValuePair.
 *
 * Returns: The #GType for #PurpleKeyValuePair.
 */
GType purple_key_value_pair_get_type(void);

/**
 * purple_key_value_pair_new:
 * @key: The key part of PurpleKeyValuePair
 * @value: The value part of PurpleKeyValuePair
 *
 * Creates a new PurpleKeyValuePair allocating memory for @key,
 * free value function is NULL.
 *
 * Returns: (transfer full): The created PurpleKeyValuePair
 *
 * Since: 3.0.0
 */
PurpleKeyValuePair *purple_key_value_pair_new(const gchar *key, gpointer value);

/**
 * purple_key_value_pair_new_full:
 * @key: The key part of PurpleKeyValuePair
 * @value: The value part of PurpleKeyValuePair
 * @value_destroy_func: a function to free the memory for the @value
 *
 * Creates a new PurpleKeyValuePair allocating memory for @key,
 * set free value function to @value_destroy_func.
 *
 * Returns: (transfer full): The created PurpleKeyValuePair
 *
 * Since: 3.0.0
 */
PurpleKeyValuePair *purple_key_value_pair_new_full(const gchar *key, gpointer value, GDestroyNotify value_destroy_func);

/**
 * purple_key_value_pair_free:
 * @kvp: The PurpleKeyValuePair to free.
 *
 * Frees @kvp.
 *
 * Since: 3.0.0
 */
void purple_key_value_pair_free(PurpleKeyValuePair *kvp);

/**
 * purple_key_value_pair_copy:
 * @kvp: The #PurpleKeyValuePair to copy.
 *
 * Creates a copy of @kvp.
 *
 * If @kvp has a value_destroy_func, %NULL will be returned as this function
 * has no way to know how to allocate a new copy of the value.
 *
 * Returns: (transfer full): A new copy of @kvp.
 */
PurpleKeyValuePair *purple_key_value_pair_copy(PurpleKeyValuePair *kvp);

G_END_DECLS

#endif /* PURPLE_KEY_VALUE_PAIR_H */

