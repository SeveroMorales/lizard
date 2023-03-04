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

#include "purplemenu.h"

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_menu_populate_dynamic_targets_func(GMenuModel *model, gint index,
                                          gpointer data)
{
	GHashTable *table = data;
	gchar *property = NULL;

	g_return_if_fail(G_IS_MENU(model));

	if(g_menu_model_get_item_attribute(model, index,
	                                   PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET,
	                                   "s", &property))
	{
		const gchar *value;

		value = g_hash_table_lookup(table, property);
		g_free(property);

		if(value != NULL) {
			GMenuItem *item = NULL;

			item = g_menu_item_new_from_model(model, index);
			g_menu_item_set_attribute(item, G_MENU_ATTRIBUTE_TARGET, "s",
			                          value);

			g_menu_remove(G_MENU(model), index);
			g_menu_insert_item(G_MENU(model), index, item);
			g_object_unref(item);
		}
	}
}

static void
purple_menu_copy_helper(GMenuModel *source, GMenu *destination) {
	gint index = 0;

	for(index = 0; index < g_menu_model_get_n_items(source); index++) {
		GMenuItem *item = NULL;
		GMenuAttributeIter *attr_iter = NULL;
		GMenuLinkIter *link_iter = NULL;

		item = g_menu_item_new(NULL, NULL);

		attr_iter = g_menu_model_iterate_item_attributes(source, index);
		while(g_menu_attribute_iter_next(attr_iter)) {
			const gchar *name = g_menu_attribute_iter_get_name(attr_iter);
			GVariant *value = g_menu_attribute_iter_get_value(attr_iter);

			if(value != NULL) {
				g_menu_item_set_attribute_value(item, name, value);
				g_variant_unref(value);
			}
		}
		g_clear_object(&attr_iter);

		link_iter = g_menu_model_iterate_item_links(source, index);
		while(g_menu_link_iter_next(link_iter)) {
			GMenuModel *link_source = g_menu_link_iter_get_value(link_iter);
			GMenu *link_destination = g_menu_new();

			purple_menu_copy_helper(link_source, link_destination);

			g_menu_item_set_link(item, g_menu_link_iter_get_name(link_iter),
			                     G_MENU_MODEL(link_destination));

			g_clear_object(&link_source);
			g_clear_object(&link_destination);
		}
		g_clear_object(&link_iter);

		g_menu_append_item(destination, item);
		g_clear_object(&item);
	}
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_menu_walk(GMenuModel *model, PurpleMenuWalkFunc func, gpointer data) {
	gint index = 0;

	for(index = 0; index < g_menu_model_get_n_items(model); index++) {
		GMenuLinkIter *iter = NULL;

		func(model, index, data);

		iter = g_menu_model_iterate_item_links(model, index);
		while(g_menu_link_iter_next(iter)) {
			GMenuModel *link = g_menu_link_iter_get_value(iter);

			purple_menu_walk(link, func, data);

			g_clear_object(&link);
		}

		g_clear_object(&iter);
	}
}

void
purple_menu_populate_dynamic_targetsv(GMenu *menu, GHashTable *properties) {
	g_return_if_fail(G_IS_MENU(menu));
	g_return_if_fail(properties != NULL);

	purple_menu_walk(G_MENU_MODEL(menu),
	                 purple_menu_populate_dynamic_targets_func, properties);
}

void
purple_menu_populate_dynamic_targets(GMenu *menu, const gchar *first_property,
                                     ...)
{
	GHashTable *table = NULL;
	const gchar *property = NULL, *value = NULL;
	va_list vargs;

	g_return_if_fail(G_IS_MENU(menu));
	g_return_if_fail(first_property != NULL);

	table = g_hash_table_new(g_str_hash, g_str_equal);

	property = first_property;
	va_start(vargs, first_property);

	/* Iterate through the vargs adding values when we find one that isn't
	 * NULL.
	 */
	do {
		value = va_arg(vargs, const gchar *);
		if(value != NULL) {
			g_hash_table_insert(table, (gpointer)property, (gpointer)value);
		}

		/* After adding the value, see if we have another property. */
		property = va_arg(vargs, const gchar *);
	} while(property != NULL);

	va_end(vargs);

	purple_menu_populate_dynamic_targetsv(menu, table);

	g_hash_table_unref(table);
}

GMenu *
purple_menu_copy(GMenuModel *model) {
	GMenu *menu = NULL;

	g_return_val_if_fail(G_IS_MENU_MODEL(model), NULL);

	menu = g_menu_new();

	purple_menu_copy_helper(model, menu);

	return menu;
}
