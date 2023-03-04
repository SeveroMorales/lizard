/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#include "pidgindisplayitem.h"

struct _PidginDisplayItem {
	GObject parent;

	GtkWidget *widget;

	char *id;
	char *title;

	char *icon_name;

	gboolean needs_attention;

	guint badge_number;

	GListModel *children;
};

enum {
	PROP_0,
	PROP_WIDGET,
	PROP_ID,
	PROP_TITLE,
	PROP_ICON_NAME,
	PROP_NEEDS_ATTENTION,
	PROP_BADGE_NUMBER,
	PROP_CHILDREN,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE(PidginDisplayItem, pidgin_display_item, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_display_item_set_widget(PidginDisplayItem *item, GtkWidget *widget) {
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));

	if(g_set_object(&item->widget, widget)) {
		g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_WIDGET]);
	}
}

static void
pidgin_display_item_set_id(PidginDisplayItem *item, const char *id) {
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));
	g_return_if_fail(id != NULL);

	if(item->id == id) {
		return;
	}

	g_free(item->id);
	item->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_ID]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_display_item_dispose(GObject *obj) {
	PidginDisplayItem *item = PIDGIN_DISPLAY_ITEM(obj);

	g_clear_object(&item->widget);
	g_clear_object(&item->children);

	G_OBJECT_CLASS(pidgin_display_item_parent_class)->dispose(obj);
}

static void
pidgin_display_item_finalize(GObject *obj) {
	PidginDisplayItem *item = PIDGIN_DISPLAY_ITEM(obj);

	g_clear_pointer(&item->id, g_free);
	g_clear_pointer(&item->title, g_free);
	g_clear_pointer(&item->icon_name, g_free);

	G_OBJECT_CLASS(pidgin_display_item_parent_class)->finalize(obj);
}

static void
pidgin_display_item_get_property(GObject *obj, guint param_id, GValue *value,
                                 GParamSpec *pspec)
{
	PidginDisplayItem *item = PIDGIN_DISPLAY_ITEM(obj);

	switch(param_id) {
		case PROP_WIDGET:
			g_value_set_object(value, pidgin_display_item_get_widget(item));
			break;
		case PROP_ID:
			g_value_set_string(value, pidgin_display_item_get_id(item));
			break;
		case PROP_TITLE:
			g_value_set_string(value, pidgin_display_item_get_title(item));
			break;
		case PROP_ICON_NAME:
			g_value_set_string(value, pidgin_display_item_get_icon_name(item));
			break;
		case PROP_NEEDS_ATTENTION:
			g_value_set_boolean(value,
			                    pidgin_display_item_get_needs_attention(item));
			break;
		case PROP_BADGE_NUMBER:
			g_value_set_uint(value,
			                 pidgin_display_item_get_badge_number(item));
			break;
		case PROP_CHILDREN:
			g_value_set_object(value, pidgin_display_item_get_children(item));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_display_item_set_property(GObject *obj, guint param_id,
                                 const GValue *value, GParamSpec *pspec)
{
	PidginDisplayItem *item = PIDGIN_DISPLAY_ITEM(obj);

	switch(param_id) {
		case PROP_WIDGET:
			pidgin_display_item_set_widget(item, g_value_get_object(value));
			break;
		case PROP_ID:
			pidgin_display_item_set_id(item, g_value_get_string(value));
			break;
		case PROP_TITLE:
			pidgin_display_item_set_title(item, g_value_get_string(value));
			break;
		case PROP_ICON_NAME:
			pidgin_display_item_set_icon_name(item, g_value_get_string(value));
			break;
		case PROP_NEEDS_ATTENTION:
			pidgin_display_item_set_needs_attention(item,
			                                        g_value_get_boolean(value));
			break;
		case PROP_BADGE_NUMBER:
			pidgin_display_item_set_badge_number(item,
			                                     g_value_get_uint(value));
			break;
		case PROP_CHILDREN:
			pidgin_display_item_set_children(item, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_display_item_init(G_GNUC_UNUSED PidginDisplayItem *window) {
}

static void
pidgin_display_item_class_init(PidginDisplayItemClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = pidgin_display_item_dispose;
	obj_class->finalize = pidgin_display_item_finalize;
	obj_class->get_property = pidgin_display_item_get_property;
	obj_class->set_property = pidgin_display_item_set_property;

	/**
	 * PidginDisplayItem:widget:
	 *
	 * The [class@Gtk.Widget] that this item is for.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_WIDGET] = g_param_spec_object(
		"widget", "widget",
		"The widget for this item",
		GTK_TYPE_WIDGET,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginDisplayItem:id:
	 *
	 * A unique identifier for this item. This is used for things like
	 * remembering positions and selections.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"A unique identifier for the item",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginDisplayItem:title:
	 *
	 * The title that should be displayed for this item.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TITLE] = g_param_spec_string(
		"title", "title",
		"The title for the item",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginDisplayItem:icon-name:
	 *
	 * The icon name to use for this item.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ICON_NAME] = g_param_spec_string(
		"icon-name", "icon-name",
		"The icon-name to use for this item",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginDisplayItem:needs-attention:
	 *
	 * Determines whether the item should show that it needs attention or not.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NEEDS_ATTENTION] = g_param_spec_boolean(
		"needs-attention", "needs-attention",
		"Whether or not the item needs attention",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginDisplayItem:badge-number:
	 *
	 * The number that should be shown in the badge. Typically this is an
	 * unread count. If this is 0 it should not be displayed.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_BADGE_NUMBER] = g_param_spec_uint(
		"badge-number", "badge-number",
		"The number to show in the badge",
		0, G_MAXUINT, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginDisplayItem:children:
	 *
	 * A [iface@Gio.ListModel] of child items. The type of the model needs to
	 * be [class@Pidgin.DisplayItem].
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_CHILDREN] = g_param_spec_object(
		"children", "children",
		"A GListModel of child items",
		G_TYPE_LIST_MODEL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * API
 *****************************************************************************/
PidginDisplayItem *
pidgin_display_item_new(GtkWidget *widget, const char *id) {
	g_return_val_if_fail(GTK_IS_WIDGET(widget), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	return g_object_new(
		PIDGIN_TYPE_DISPLAY_ITEM,
		"widget", widget,
		"id", id,
		NULL);
}

GtkWidget *
pidgin_display_item_get_widget(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), NULL);

	return item->widget;
}

const char *
pidgin_display_item_get_id(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), NULL);

	return item->id;
}

const char *
pidgin_display_item_get_title(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), NULL);

	return item->title;
}

void
pidgin_display_item_set_title(PidginDisplayItem *item, const char *title) {
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));

	if(item->title == title) {
		return;
	}

	g_free(item->title);
	item->title = g_strdup(title);

	g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_TITLE]);
}

const char *
pidgin_display_item_get_icon_name(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), NULL);

	return item->icon_name;
}

void
pidgin_display_item_set_icon_name(PidginDisplayItem *item,
                                  const char *icon_name)
{
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));

	if(item->icon_name == icon_name) {
		return;
	}

	g_free(item->icon_name);
	item->icon_name = g_strdup(icon_name);

	g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_ICON_NAME]);
}

gboolean
pidgin_display_item_get_needs_attention(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), FALSE);

	return item->needs_attention;
}

void
pidgin_display_item_set_needs_attention(PidginDisplayItem *item,
                                        gboolean needs_attention)
{
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));

	if(item->needs_attention != needs_attention) {
		item->needs_attention = needs_attention;

		g_object_notify_by_pspec(G_OBJECT(item),
		                         properties[PROP_NEEDS_ATTENTION]);
	}
}

guint
pidgin_display_item_get_badge_number(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), 0);

	return item->badge_number;
}

void
pidgin_display_item_set_badge_number(PidginDisplayItem *item,
                                     guint badge_number)
{
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));

	if(item->badge_number != badge_number) {
		item->badge_number = badge_number;

		g_object_notify_by_pspec(G_OBJECT(item),
		                         properties[PROP_BADGE_NUMBER]);
	}
}

GListModel *
pidgin_display_item_get_children(PidginDisplayItem *item) {
	g_return_val_if_fail(PIDGIN_IS_DISPLAY_ITEM(item), NULL);

	return item->children;
}

void
pidgin_display_item_set_children(PidginDisplayItem *item,
                                 GListModel *children)
{
	g_return_if_fail(PIDGIN_IS_DISPLAY_ITEM(item));

	if(g_set_object(&item->children, children)) {
		g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_CHILDREN]);
	}
}
