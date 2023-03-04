/*
 * purple
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

#include "purplekeyvaluepair.h"

G_DEFINE_BOXED_TYPE(PurpleKeyValuePair, purple_key_value_pair,
                    purple_key_value_pair_copy, purple_key_value_pair_free);

PurpleKeyValuePair *
purple_key_value_pair_new(const gchar *key, gpointer value) {
	return purple_key_value_pair_new_full(key, value, NULL);
}

PurpleKeyValuePair *
purple_key_value_pair_new_full(const gchar *key, gpointer value,
                               GDestroyNotify value_destroy_func)
{
	PurpleKeyValuePair *kvp;

	kvp = g_new(PurpleKeyValuePair, 1);
	kvp->key = g_strdup(key);
	kvp->value = value;
	kvp->value_destroy_func = value_destroy_func;

	return kvp;
}

void
purple_key_value_pair_free(PurpleKeyValuePair *kvp) {
	g_return_if_fail(kvp != NULL);

	g_free(kvp->key);

	if(kvp->value_destroy_func) {
		kvp->value_destroy_func(kvp->value);
	}

	g_free(kvp);
}

PurpleKeyValuePair *
purple_key_value_pair_copy(PurpleKeyValuePair *kvp) {
	g_return_val_if_fail(kvp != NULL, NULL);
	g_return_val_if_fail(kvp->value_destroy_func == NULL, NULL);

	return purple_key_value_pair_new(kvp->key, kvp->value);
}
