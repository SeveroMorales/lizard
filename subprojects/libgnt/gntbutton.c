/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "gntinternal.h"
#include "gntbutton.h"
#include "gntstyle.h"
#include "gntutils.h"

#include "gntwidgetprivate.h"

typedef struct
{
	gchar *text;
} GntButtonPrivate;

enum
{
	SIGS = 1,
};

static gboolean small_button = FALSE;

G_DEFINE_TYPE_WITH_PRIVATE(GntButton, gnt_button, GNT_TYPE_WIDGET)

static void
gnt_button_draw(GntWidget *widget)
{
	GntButton *button = GNT_BUTTON(widget);
	GntButtonPrivate *priv = gnt_button_get_instance_private(button);
	WINDOW *window = gnt_widget_get_window(widget);
	GntColorType type;
	gboolean focus;

	if ((focus = gnt_widget_has_focus(widget)))
		type = GNT_COLOR_HIGHLIGHT;
	else
		type = GNT_COLOR_NORMAL;

	wbkgdset(window, '\0' | gnt_color_pair(type));
	mvwaddstr(window, (small_button) ? 0 : 1, 2, C_(priv->text));
	if (small_button) {
		gint width;
		type = GNT_COLOR_HIGHLIGHT;
		gnt_widget_get_internal_size(widget, &width, NULL);
		mvwchgat(window, 0, 0, width, focus ? A_BOLD : A_REVERSE, type,
		         NULL);
	}
}

static void
gnt_button_size_request(GntWidget *widget)
{
	GntButton *button = GNT_BUTTON(widget);
	GntButtonPrivate *priv = gnt_button_get_instance_private(button);
	gint width, height;

	gnt_util_get_text_bound(priv->text, &width, &height);
	width += 4;
	if (gnt_widget_get_has_border(widget)) {
		height += 2;
	}
	gnt_widget_set_internal_size(widget, width, height);
}

static void
gnt_button_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
}

static gboolean
gnt_button_clicked(GntWidget *widget, GntMouseEvent event, G_GNUC_UNUSED int x,
                   G_GNUC_UNUSED int y)
{
	if (event == GNT_LEFT_MOUSE_DOWN) {
		gnt_widget_activate(widget);
		return TRUE;
	}
	return FALSE;
}

static void
gnt_button_destroy(GntWidget *widget)
{
	GntButton *button = GNT_BUTTON(widget);
	GntButtonPrivate *priv = gnt_button_get_instance_private(button);

	g_clear_pointer(&priv->text, g_free);
}

static gboolean
button_activate(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	gnt_widget_activate(GNT_WIDGET(bind));
	return TRUE;
}

static void
gnt_button_class_init(GntButtonClass *klass)
{
	char *style;
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->draw = gnt_button_draw;
	widget_class->map = gnt_button_map;
	widget_class->size_request = gnt_button_size_request;
	widget_class->clicked = gnt_button_clicked;
	widget_class->destroy = gnt_button_destroy;

	style = gnt_style_get_from_name(NULL, "small-button");
	small_button = gnt_style_parse_bool(style);
	g_free(style);

	gnt_bindable_class_register_action(bindable, "activate", button_activate,
				GNT_KEY_ENTER, NULL);
	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), GNT_BINDABLE_CLASS(klass));
}

static void
gnt_button_init(GntButton *button)
{
	GntWidget *widget = GNT_WIDGET(button);

	gnt_widget_set_minimum_size(widget, 4, small_button ? 1 : 3);
	if (small_button) {
		gnt_widget_set_has_border(widget, FALSE);
		gnt_widget_set_has_shadow(widget, FALSE);
	}
	gnt_widget_set_grow_x(widget, FALSE);
	gnt_widget_set_grow_y(widget, FALSE);
}

/******************************************************************************
 * GntButton API
 *****************************************************************************/
GntWidget *gnt_button_new(const char *text)
{
	GntWidget *widget = g_object_new(GNT_TYPE_BUTTON, NULL);
	GntButton *button = GNT_BUTTON(widget);
	GntButtonPrivate *priv = gnt_button_get_instance_private(button);

	priv->text = gnt_util_onscreen_fit_string(text, -1);
	gnt_widget_set_take_focus(widget, TRUE);

	return widget;
}

const gchar *
gnt_button_get_text(GntButton *button)
{
	GntButtonPrivate *priv = gnt_button_get_instance_private(button);

	g_return_val_if_fail(GNT_IS_BUTTON(button), NULL);

	return priv->text;
}

void
gnt_button_set_text(GntButton *button, const gchar *text)
{
	GntButtonPrivate *priv = gnt_button_get_instance_private(button);

	g_return_if_fail(GNT_IS_BUTTON(button));

	g_free(priv->text);
	priv->text = g_strdup(text);
}
