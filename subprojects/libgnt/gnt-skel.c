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

#include "gnt-skel.h"

#include "gntwidgetprivate.h"

struct _GntSkel
{
	GntWidget parent;
};

enum
{
	SIGS = 1,
};

static guint signals[SIGS] = { 0 };

G_DEFINE_TYPE(GntSkel, gnt_skel, GNT_TYPE_WIDGET)

static void
gnt_skel_draw(GntWidget *widget)
{
}

static void
gnt_skel_size_request(GntWidget *widget)
{
}

static void
gnt_skel_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
}

static gboolean
gnt_skel_key_pressed(GntWidget *widget, const char *text)
{
	return FALSE;
}

static void
gnt_skel_destroy(GntWidget *widget)
{
}

static void
gnt_skel_class_init(GntSkelClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->destroy = gnt_skel_destroy;
	widget_class->draw = gnt_skel_draw;
	widget_class->map = gnt_skel_map;
	widget_class->size_request = gnt_skel_size_request;
	widget_class->key_pressed = gnt_skel_key_pressed;

	widget_class->actions = gnt_hash_table_duplicate(widget_class->actions,
			g_str_hash, g_str_equal, NULL,
			(GDestroyNotify)gnt_widget_action_free);
	widget_class->bindings =
		gnt_hash_table_duplicate(widget_class->bindings, g_str_hash,
				g_str_equal, NULL,
				(GDestroyNotify)gnt_widget_action_param_free);

	gnt_widget_actions_read(G_OBJECT_CLASS_TYPE(klass), klass);
}

static void
gnt_skel_init(G_GNUC_UNUSED GntSkel *self)
{
}

/******************************************************************************
 * GntSkel API
 *****************************************************************************/
GntWidget *
gnt_skel_new(void)
{
	GntWidget *widget = g_object_new(GNT_TYPE_SKEL, NULL);
	GntSkel *skel = GNT_SKEL(widget);

	return widget;
}

