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

#include "gntinternal.h"
#include "gntline.h"

#include "gntncurses.h"
#include "gntwidgetprivate.h"

struct _GntLine
{
	GntWidget parent;

	gboolean vertical;
};

enum
{
	PROP_0,
	PROP_VERTICAL
};

enum
{
	SIGS = 1,
};

G_DEFINE_TYPE(GntLine, gnt_line, GNT_TYPE_WIDGET)

static void
gnt_line_draw(GntWidget *widget)
{
	GntLine *line = GNT_LINE(widget);
	WINDOW *window = gnt_widget_get_window(widget);
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (line->vertical) {
		mvwvline(window, 1, 0,
		         ACS_VLINE | gnt_color_pair(GNT_COLOR_NORMAL),
		         height - 2);
	} else {
		mvwhline(window, 0, 1,
		         ACS_HLINE | gnt_color_pair(GNT_COLOR_NORMAL),
		         width - 2);
	}
}

static void
gnt_line_size_request(GntWidget *widget)
{
	if (GNT_LINE(widget)->vertical) {
		gnt_widget_set_internal_size(widget, 1, 5);
	} else {
		gnt_widget_set_internal_size(widget, 5, 1);
	}
}

static void
gnt_line_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
}

static void
gnt_line_set_property(GObject *obj, guint prop_id, const GValue *value,
                      G_GNUC_UNUSED GParamSpec *spec)
{
	GntLine *line = GNT_LINE(obj);
	switch (prop_id) {
		case PROP_VERTICAL:
			line->vertical = g_value_get_boolean(value);
			if (line->vertical) {
				gnt_widget_set_grow_y(GNT_WIDGET(line), TRUE);
			} else {
				gnt_widget_set_grow_x(GNT_WIDGET(line), TRUE);
			}
			break;
		default:
			break;
	}
}

static void
gnt_line_get_property(GObject *obj, guint prop_id, GValue *value,
                      G_GNUC_UNUSED GParamSpec *spec)
{
	GntLine *line = GNT_LINE(obj);
	switch (prop_id) {
		case PROP_VERTICAL:
			g_value_set_boolean(value, line->vertical);
			break;
		default:
			break;
	}
}

static void
gnt_line_class_init(GntLineClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->draw = gnt_line_draw;
	widget_class->map = gnt_line_map;
	widget_class->size_request = gnt_line_size_request;

	obj_class->set_property = gnt_line_set_property;
	obj_class->get_property = gnt_line_get_property;
	g_object_class_install_property(obj_class,
			PROP_VERTICAL,
			g_param_spec_boolean("vertical", "Vertical",
				"Whether it's a vertical line or a horizontal one.",
				TRUE,
				G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS
			)
		);
}

static void
gnt_line_init(GntLine *line)
{
	GntWidget *widget = GNT_WIDGET(line);
	gnt_widget_set_has_shadow(widget, FALSE);
	gnt_widget_set_has_border(widget, FALSE);
	gnt_widget_set_minimum_size(widget, 1, 1);
}

/******************************************************************************
 * GntLine API
 *****************************************************************************/
GntWidget *gnt_line_new(gboolean vertical)
{
	GntWidget *widget = g_object_new(GNT_TYPE_LINE, "vertical", vertical, NULL);
	return widget;
}
