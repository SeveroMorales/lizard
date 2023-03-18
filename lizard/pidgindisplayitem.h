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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_DISPLAY_ITEM_H
#define PIDGIN_DISPLAY_ITEM_H

#include <glib.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * PidginDisplayItem:
 *
 * A class that's used by [class@Pidgin.DisplayWindow] to represent all of the
 * displays items.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_DISPLAY_ITEM (pidgin_display_item_get_type())
G_DECLARE_FINAL_TYPE(PidginDisplayItem, pidgin_display_item,
                     PIDGIN, DISPLAY_ITEM, GObject)

/**
 * pidgin_display_item_new:
 * @child: The [class@Gtk.Widget] that this item represents.
 * @id: A unique identifier that will be used internally.
 *
 * Creates a new #PidginDisplayItem instance.
 *
 * Returns: (transfer full): The new #PidginDisplayItem instance.
 */
PidginDisplayItem *pidgin_display_item_new(GtkWidget *child, const char *id);

/**
 * pidgin_display_item_get_widget:
 * @item: The instance.
 *
 * Gets the widget that @item was created for.
 *
 * Returns: (transfer none): The widget.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_display_item_get_widget(PidginDisplayItem *item);

/**
 * pidgin_display_item_get_id:
 * @item: The instance.
 *
 * Gets the unique identifier of @item.
 *
 * Returns: The unique identifier of @item.
 *
 * Since: 3.0.0
 */
const char *pidgin_display_item_get_id(PidginDisplayItem *item);

/**
 * pidgin_display_item_get_title:
 * @item: The instance.
 *
 * Gets the title of @item.
 *
 * Returns: (nullable): The title of @item.
 *
 * Since: 3.0.0
 */
const char *pidgin_display_item_get_title(PidginDisplayItem *item);

/**
 * pidgin_display_item_set_title:
 * @item: The instance.
 * @title: (nullable): The new title.
 *
 * Sets the title for @item to @title.
 *
 * Since: 3.0.0
 */
void pidgin_display_item_set_title(PidginDisplayItem *item, const char *title);

/**
 * pidgin_display_item_get_icon_name:
 * @item: The instance.
 *
 * Gets the icon name if any that should be used when displaying @item.
 *
 * Returns: (nullable): The icon name to use.
 *
 * Since: 3.0.0
 */
const char *pidgin_display_item_get_icon_name(PidginDisplayItem *item);

/**
 * pidgin_display_item_set_icon_name:
 * @item: The instance.
 * @icon_name: (nullable): The icon-name to use.
 *
 * Sets the icon name that should be used when displaying @item.
 *
 * Since: 3.0.0
 */
void pidgin_display_item_set_icon_name(PidginDisplayItem *item, const char *icon_name);

/**
 * pidgin_display_item_get_needs_attention:
 * @item: The instance.
 *
 * Gets whether or not @item needs attention.
 *
 * Returns: %TRUE if @item needs attention otherwise %FALSE.
 *
 * Since: 3.0.0
 */
gboolean pidgin_display_item_get_needs_attention(PidginDisplayItem *item);

/**
 * pidgin_display_item_set_needs_attention:
 * @item: The instance.
 * @needs_attention: Whether or not attention is needed.
 *
 * Sets whether or not @item needs attention.
 *
 * Since: 3.0.0
 */
void pidgin_display_item_set_needs_attention(PidginDisplayItem *item, gboolean needs_attention);

/**
 * pidgin_display_item_get_badge_number:
 * @item: The instance.
 *
 * Gets the number that should be displayed in the badge for @item.
 *
 * Returns: The value to display or %0 to display nothing.
 *
 * Since: 3.0.0
 */
guint pidgin_display_item_get_badge_number(PidginDisplayItem *item);

/**
 * pidgin_display_item_set_badge_number:
 * @item: The instance.
 * @badge_number: The new value.
 *
 * Sets the values to be displayed in the badge for @item to @badge_number. A
 * value of %0 indicates that the badge should not be displayed.
 *
 * Since: 3.0.0
 */
void pidgin_display_item_set_badge_number(PidginDisplayItem *item, guint badge_number);

/**
 * pidgin_display_item_get_children:
 * @item: The instance.
 *
 * Gets the children for @item if there are any.
 *
 * Returns: (transfer none) (nullable): The children of @item if there are any.
 *
 * Since: 3.0.0
 */
GListModel *pidgin_display_item_get_children(PidginDisplayItem *item);

/**
 * pidgin_display_item_set_children:
 * @item: The instance:
 * @children: (nullable): The new children.
 *
 * Sets the children of @item to @children. If @children is not %NULL then the
 * model must have an item type of [class@Pidgin.DisplayItem].
 *
 * Since: 3.0.0
 */
void pidgin_display_item_set_children(PidginDisplayItem *item, GListModel *children);

G_END_DECLS

#endif /* PIDGIN_DISPLAY_ITEM_H */
