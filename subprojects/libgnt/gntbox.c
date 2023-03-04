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
#include "gntbox.h"
#include "gntstyle.h"
#include "gntutils.h"

#include "gntmainprivate.h"
#include "gntwidgetprivate.h"

#include <string.h>

typedef struct
{
	gboolean vertical;
	gboolean homogeneous;
	gboolean fill;
	GList *list; /* List of widgets */

	GntWidget *active;
	int pad;                /* Number of spaces to use between widgets */
	GntAlignment alignment; /* How are the widgets going to be aligned? */

	char *title;
	GList *focus; /* List of widgets to cycle focus (only valid for parent
	                 boxes) */

	GntWidget *last_resize;
	GntWidget *size_queued;
} GntBoxPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GntBox, gnt_box, GNT_TYPE_WIDGET)

enum
{
	PROP_0,
	PROP_VERTICAL,
	PROP_HOMOGENEOUS
};

enum
{
	SIGS = 1,
};

static GntWidget * find_focusable_widget(GntBox *box);

static void
add_to_focus(GntWidget *w, GntBox *box)
{
	if (GNT_IS_BOX(w)) {
		GntBoxPrivate *priv = gnt_box_get_instance_private(GNT_BOX(w));
		g_list_foreach(priv->list, (GFunc)add_to_focus, box);
	} else if (gnt_widget_get_take_focus(w)) {
		GntBoxPrivate *priv = gnt_box_get_instance_private(box);
		priv->focus = g_list_append(priv->focus, w);
	}
}

static void
get_title_thingies(GntBox *box, char *title, int *p, int *r)
{
	gint width;
	int len;
	char *end;

	gnt_widget_get_internal_size(GNT_WIDGET(box), &width, NULL);
	end = (char *)gnt_util_onscreen_width_to_pointer(title, width - 4,
	                                                 &len);

	if (p)
		*p = (width - len) / 2;
	if (r)
		*r = (width + len) / 2;
	*end = '\0';
}

static void
draw_a_widget(GntWidget *widget, G_GNUC_UNUSED gpointer data)
{
	gnt_widget_draw(widget);
}

static void
gnt_box_draw(GntWidget *widget)
{
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	WINDOW *window = gnt_widget_get_window(widget);

	if (priv->focus == NULL && gnt_widget_get_parent(widget) == NULL) {
		g_list_foreach(priv->list, (GFunc)add_to_focus, box);
	}

	g_list_foreach(priv->list, (GFunc)draw_a_widget, NULL);

	if (priv->title && gnt_widget_get_has_border(widget)) {
		int pos, right;
		char *title = g_strdup(priv->title);

		get_title_thingies(box, title, &pos, &right);

		if (gnt_widget_has_focus(widget)) {
			wbkgdset(window,
			         '\0' | gnt_color_pair(GNT_COLOR_TITLE));
		} else {
			wbkgdset(window,
			         '\0' | gnt_color_pair(GNT_COLOR_TITLE_D));
		}
		mvwaddch(window, 0, pos - 1,
		         ACS_RTEE | gnt_color_pair(GNT_COLOR_NORMAL));
		mvwaddstr(window, 0, pos, C_(title));
		mvwaddch(window, 0, right,
		         ACS_LTEE | gnt_color_pair(GNT_COLOR_NORMAL));
		g_free(title);
	}

	gnt_box_sync_children(box);
}

static void
reposition_children(GntWidget *widget)
{
	GList *iter;
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	int w, h, curx, cury, max;
	gboolean has_border = FALSE;

	w = h = 0;
	max = 0;
	gnt_widget_get_position(widget, &curx, &cury);
	if (gnt_widget_get_has_border(widget)) {
		has_border = TRUE;
		curx += 1;
		cury += 1;
	}

	for (iter = priv->list; iter; iter = iter->next) {
		if (!gnt_widget_get_visible(GNT_WIDGET(iter->data)))
			continue;
		gnt_widget_set_position(GNT_WIDGET(iter->data), curx, cury);
		gnt_widget_get_size(GNT_WIDGET(iter->data), &w, &h);
		if (priv->vertical) {
			if (h)
			{
				cury += h + priv->pad;
				if (max < w)
					max = w;
			}
		} else {
			if (w)
			{
				curx += w + priv->pad;
				if (max < h)
					max = h;
			}
		}
	}

	if (has_border)
	{
		curx += 1;
		cury += 1;
		max += 2;
	}

	if (priv->list) {
		if (priv->vertical) {
			cury -= priv->pad;
		} else {
			curx -= priv->pad;
		}
	}

	if (priv->vertical) {
		gint widgety;
		gnt_widget_get_position(widget, NULL, &widgety);
		gnt_widget_set_internal_size(widget, max, cury - widgety);
	} else {
		gint widgetx;
		gnt_widget_get_position(widget, &widgetx, NULL);
		gnt_widget_set_internal_size(widget, curx - widgetx, max);
	}
}

static void
gnt_box_set_position(GntWidget *widget, int x, int y)
{
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	GList *iter;
	gint widgetx, widgety;
	gint changex, changey;

	gnt_widget_get_position(widget, &widgetx, &widgety);
	changex = widgetx - x;
	changey = widgety - y;

	for (iter = priv->list; iter; iter = iter->next) {
		GntWidget *w = GNT_WIDGET(iter->data);
		gint wx, wy;
		gnt_widget_get_position(w, &wx, &wy);
		gnt_widget_set_position(w, wx - changex, wy - changey);
	}
}

static void
gnt_box_size_request(GntWidget *widget)
{
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	GList *iter;
	int maxw = 0, maxh = 0;

	for (iter = priv->list; iter; iter = iter->next) {
		GntWidget *widget = GNT_WIDGET(iter->data);
		int w, h;
		gnt_widget_size_request(widget);
		gnt_widget_get_size(widget, &w, &h);
		if (maxh < h)
			maxh = h;
		if (maxw < w)
			maxw = w;
	}

	for (iter = priv->list; iter; iter = iter->next) {
		int w, h;
		GntWidget *wid = GNT_WIDGET(iter->data);

		gnt_widget_get_size(wid, &w, &h);

		if (priv->homogeneous) {
			if (priv->vertical) {
				h = maxh;
			} else {
				w = maxw;
			}
		}
		if (priv->fill) {
			if (priv->vertical) {
				w = maxw;
			} else {
				h = maxh;
			}
		}

		if (gnt_widget_confirm_size(wid, w, h))
			gnt_widget_set_size(wid, w, h);
	}

	reposition_children(widget);
}

static void
gnt_box_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
		find_focusable_widget(GNT_BOX(widget));
	}
}

/* Ensures that the current widget can take focus */
static GntWidget *
find_focusable_widget(GntBox *box)
{
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);

	/* XXX: Make sure the widget is visible? */
	if (priv->focus == NULL &&
	    gnt_widget_get_parent(GNT_WIDGET(box)) == NULL) {
		g_list_foreach(priv->list, (GFunc)add_to_focus, box);
	}

	if (priv->active == NULL && priv->focus) {
		priv->active = priv->focus->data;
	}

	return priv->active;
}

static void
find_next_focus(GntBoxPrivate *priv)
{
	gpointer last = priv->active;
	do
	{
		if (priv->focus) {
			GList *iter = g_list_find(priv->focus, priv->active);
			if (iter && iter->next) {
				priv->active = iter->next->data;
			} else {
				priv->active = priv->focus->data;
			}
		}
		if (gnt_widget_get_visible(priv->active) &&
		    gnt_widget_get_take_focus(priv->active)) {
			break;
		}
	} while (priv->active != last);
}

static void
find_prev_focus(GntBoxPrivate *priv)
{
	gpointer last = priv->active;

	if (!priv->focus) {
		return;
	}

	do
	{
		GList *iter = g_list_find(priv->focus, priv->active);
		if (!iter)
			priv->active = priv->focus->data;
		else if (!iter->prev)
			priv->active = g_list_last(priv->focus)->data;
		else
			priv->active = iter->prev->data;
		if (gnt_widget_get_visible(priv->active)) {
			break;
		}
	} while (priv->active != last);
}

static gboolean
gnt_box_key_pressed(GntWidget *widget, const char *text)
{
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	gboolean ret;

	if (!gnt_widget_get_disable_actions(widget))
		return FALSE;

	if (priv->active == NULL && !find_focusable_widget(box)) {
		return FALSE;
	}

	if (gnt_widget_key_pressed(priv->active, text)) {
		return TRUE;
	}

	/* This dance is necessary to make sure that the child widgets get a chance
	   to trigger their bindings first */
	gnt_widget_set_disable_actions(widget, FALSE);
	ret = gnt_widget_key_pressed(widget, text);
	gnt_widget_set_disable_actions(widget, TRUE);
	return ret;
}

static gboolean
box_focus_change(GntBox *box, gboolean next)
{
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	GntWidget *now;

	now = priv->active;

	if (next) {
		find_next_focus(priv);
	} else {
		find_prev_focus(priv);
	}

	if (now && now != priv->active) {
		gnt_widget_set_focus(now, FALSE);
		gnt_widget_set_focus(priv->active, TRUE);
		return TRUE;
	}

	return FALSE;
}

static gboolean
action_focus_next(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	return box_focus_change(GNT_BOX(bindable), TRUE);
}

static gboolean
action_focus_prev(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	return box_focus_change(GNT_BOX(bindable), FALSE);
}

static void
gnt_box_lost_focus(GntWidget *widget)
{
	GntBoxPrivate *priv = gnt_box_get_instance_private(GNT_BOX(widget));
	GntWidget *w = priv->active;
	if (w)
		gnt_widget_set_focus(w, FALSE);
	gnt_widget_draw(widget);
}

static void
gnt_box_gained_focus(GntWidget *widget)
{
	GntBoxPrivate *priv = gnt_box_get_instance_private(GNT_BOX(widget));
	GntWidget *w = priv->active;
	if (w)
		gnt_widget_set_focus(w, TRUE);
	gnt_widget_draw(widget);
}

static void
gnt_box_destroy(GntWidget *w)
{
	GntBox *box = GNT_BOX(w);

	gnt_box_remove_all(box);
	gnt_screen_release(w);
}

static void
gnt_box_expose(GntWidget *widget, int x, int y, int width, int height)
{
	WINDOW *win;
	gint widgetx, widgety;

	gnt_widget_get_position(widget, &widgetx, &widgety);
	win = newwin(height, width, widgety + y, widgetx + x);
	copywin(gnt_widget_get_window(widget), win, y, x, 0, 0, height - 1,
	        width - 1, FALSE);
	wrefresh(win);
	delwin(win);
}

static gboolean
gnt_box_confirm_size(GntWidget *widget, int width, int height)
{
	GList *iter;
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	gint widget_width, widget_height;
	int wchange, hchange;
	GntWidget *child, *last;

	if (!priv->list) {
		return TRUE;
	}

	gnt_widget_get_internal_size(widget, &widget_width, &widget_height);
	wchange = widget_width - width;
	hchange = widget_height - height;

	if (wchange == 0 && hchange == 0)
		return TRUE;		/* Quit playing games with my size */

	child = NULL;
	last = priv->last_resize;

	/* First, make sure all the widgets will fit into the box after resizing. */
	for (iter = priv->list; iter; iter = iter->next) {
		GntWidget *wid = iter->data;
		int w, h;

		gnt_widget_get_size(wid, &w, &h);

		if (wid != last && w > 0 && h > 0 &&
		    gnt_widget_get_visible(wid) &&
		    gnt_widget_confirm_size(wid, w - wchange, h - hchange)) {
			child = wid;
			break;
		}
	}

	if (!child && (child = last)) {
		int w, h;
		gnt_widget_get_size(child, &w, &h);
		if (!gnt_widget_confirm_size(child, w - wchange, h - hchange))
			child = NULL;
	}

	priv->size_queued = child;

	if (child) {
		for (iter = priv->list; iter; iter = iter->next) {
			GntWidget *wid = iter->data;
			gint cw, ch;
			int w, h;

			if (wid == child)
				continue;

			gnt_widget_get_size(wid, &w, &h);
			gnt_widget_get_internal_size(child, &cw, &ch);
			if (priv->vertical) {
				/* For a vertical box, if we are changing the width, make sure the widgets
				 * in the box will fit after resizing the width. */
				if (wchange > 0 && w >= cw &&
				    !gnt_widget_confirm_size(wid, w - wchange,
				                             h)) {
					return FALSE;
				}
			} else {
				/* If we are changing the height, make sure the widgets in the box fit after
				 * the resize. */
				if (hchange > 0 && h >= ch &&
				    !gnt_widget_confirm_size(wid, w,
				                             h - hchange)) {
					return FALSE;
				}
			}
		}
	}

	return (child != NULL);
}

static void
gnt_box_size_changed(GntWidget *widget, int oldw, int oldh)
{
	gint widget_width, widget_height;
	int wchange, hchange;
	GList *i;
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	GntWidget *wid;
	int tw, th;

	gnt_widget_get_internal_size(widget, &widget_width, &widget_height);
	wchange = widget_width - oldw;
	hchange = widget_height - oldh;

	wid = priv->size_queued;
	if (wid) {
		gnt_widget_get_size(wid, &tw, &th);
		gnt_widget_set_size(wid, tw + wchange, th + hchange);
		priv->size_queued = NULL;
		priv->last_resize = wid;
	}

	if (priv->vertical) {
		hchange = 0;
	} else {
		wchange = 0;
	}

	for (i = priv->list; i; i = i->next) {
		if (wid != i->data)
		{
			gnt_widget_get_size(GNT_WIDGET(i->data), &tw, &th);
			gnt_widget_set_size(i->data, tw + wchange, th + hchange);
		}
	}

	reposition_children(widget);
}

static gboolean
gnt_box_clicked(GntWidget *widget, GntMouseEvent event, int cx, int cy)
{
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	GList *iter;
	for (iter = priv->list; iter; iter = iter->next) {
		int x, y, w, h;
		GntWidget *wid = iter->data;

		gnt_widget_get_position(wid, &x, &y);
		gnt_widget_get_size(wid, &w, &h);

		if (cx >= x && cx < x + w && cy >= y && cy < y + h) {
			if (event <= GNT_MIDDLE_MOUSE_DOWN &&
			    gnt_widget_get_take_focus(wid)) {
				widget = gnt_widget_get_toplevel(widget);
				gnt_box_give_focus_to_child(GNT_BOX(widget), wid);
			}
			return gnt_widget_clicked(wid, event, cx, cy);
		}
	}
	return FALSE;
}

static void
gnt_box_set_property(GObject *obj, guint prop_id, const GValue *value,
                     G_GNUC_UNUSED GParamSpec *spec)
{
	GntBox *box = GNT_BOX(obj);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);

	switch (prop_id) {
		case PROP_VERTICAL:
			priv->vertical = g_value_get_boolean(value);
			break;
		case PROP_HOMOGENEOUS:
			priv->homogeneous = g_value_get_boolean(value);
			break;
		default:
			g_return_if_reached();
			break;
	}
}

static void
gnt_box_get_property(GObject *obj, guint prop_id, GValue *value,
                     G_GNUC_UNUSED GParamSpec *spec)
{
	GntBox *box = GNT_BOX(obj);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);

	switch (prop_id) {
		case PROP_VERTICAL:
			g_value_set_boolean(value, priv->vertical);
			break;
		case PROP_HOMOGENEOUS:
			g_value_set_boolean(value, priv->homogeneous);
			break;
		default:
			break;
	}
}

static void
gnt_box_class_init(GntBoxClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->destroy = gnt_box_destroy;
	widget_class->draw = gnt_box_draw;
	widget_class->expose = gnt_box_expose;
	widget_class->map = gnt_box_map;
	widget_class->size_request = gnt_box_size_request;
	widget_class->set_position = gnt_box_set_position;
	widget_class->key_pressed = gnt_box_key_pressed;
	widget_class->clicked = gnt_box_clicked;
	widget_class->lost_focus = gnt_box_lost_focus;
	widget_class->gained_focus = gnt_box_gained_focus;
	widget_class->confirm_size = gnt_box_confirm_size;
	widget_class->size_changed = gnt_box_size_changed;

	obj_class->set_property = gnt_box_set_property;
	obj_class->get_property = gnt_box_get_property;
	g_object_class_install_property(obj_class,
			PROP_VERTICAL,
			g_param_spec_boolean("vertical", "Vertical",
				"Whether the child widgets in the box should be stacked vertically.",
				TRUE,
				G_PARAM_READWRITE|G_PARAM_CONSTRUCT|G_PARAM_STATIC_STRINGS
			)
		);
	g_object_class_install_property(obj_class,
			PROP_HOMOGENEOUS,
			g_param_spec_boolean("homogeneous", "Homogeneous",
				"Whether the child widgets in the box should have the same size.",
				TRUE,
				G_PARAM_READWRITE|G_PARAM_CONSTRUCT|G_PARAM_STATIC_STRINGS
			)
		);

	gnt_bindable_class_register_action(bindable, "focus-next", action_focus_next,
			"\t", NULL);
	gnt_bindable_register_binding(bindable, "focus-next", GNT_KEY_RIGHT, NULL);
	gnt_bindable_class_register_action(bindable, "focus-prev", action_focus_prev,
			GNT_KEY_BACK_TAB, NULL);
	gnt_bindable_register_binding(bindable, "focus-prev", GNT_KEY_LEFT, NULL);

	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), bindable);
}

static void
gnt_box_init(GntBox *box)
{
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);
	GntWidget *widget = GNT_WIDGET(box);

	/* Initially make both the height and width resizable.
	 * Update the flags as necessary when widgets are added to it. */
	gnt_widget_set_grow_x(widget, TRUE);
	gnt_widget_set_grow_y(widget, TRUE);
	gnt_widget_set_take_focus(widget, TRUE);
	gnt_widget_set_disable_actions(widget, TRUE);
	gnt_widget_set_has_border(widget, FALSE);
	gnt_widget_set_has_shadow(widget, FALSE);

	priv->pad = 1;
	priv->fill = TRUE;
}

/******************************************************************************
 * GntBox API
 *****************************************************************************/
GntWidget *
gnt_box_new(gboolean homogeneous, gboolean vert)
{
	GntWidget *widget = g_object_new(GNT_TYPE_BOX, "homogeneous",
	                                 homogeneous, "vertical", vert, NULL);
	GntBox *box = GNT_BOX(widget);
	GntBoxPrivate *priv = gnt_box_get_instance_private(box);

	priv->alignment = vert ? GNT_ALIGN_LEFT : GNT_ALIGN_MID;

	return widget;
}

void
gnt_box_add_widget(GntBox *box, GntWidget *widget)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	priv->list = g_list_append(priv->list, widget);
	gnt_widget_set_parent(widget, GNT_WIDGET(box));
}

void
gnt_box_add_widget_in_front(GntBox *box, GntWidget *widget)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	priv->list = g_list_prepend(priv->list, widget);
	gnt_widget_set_parent(widget, GNT_WIDGET(box));
}

void
gnt_box_set_title(GntBox *box, const char *title)
{
	GntBoxPrivate *priv = NULL;
	char *prev = NULL;
	GntWidget *w = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	prev = priv->title;
	priv->title = g_strdup(title);
	w = GNT_WIDGET(box);
	if (gnt_widget_get_window(w) && gnt_widget_get_has_border(w)) {
		/* Erase the old title */
		int pos, right;
		get_title_thingies(box, prev, &pos, &right);
		mvwhline(gnt_widget_get_window(w), 0, pos - 1,
		         ACS_HLINE | gnt_color_pair(GNT_COLOR_NORMAL),
		         right - pos + 2);
	}
	g_free(prev);
}

/* Internal. */
const gchar *
gnt_box_get_title(GntBox *box)
{
	GntBoxPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_BOX(box), NULL);
	priv = gnt_box_get_instance_private(box);
	return priv->title;
}

void gnt_box_set_pad(GntBox *box, int pad)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	priv->pad = pad;
	/* XXX: Perhaps redraw if already showing? */
}

void gnt_box_set_toplevel(GntBox *box, gboolean set)
{
	GntWidget *widget = GNT_WIDGET(box);

	gnt_widget_set_has_border(widget, set);
	gnt_widget_set_has_shadow(widget, set);
	gnt_widget_set_take_focus(widget, set);
}

GList *
gnt_box_get_children(GntBox *box)
{
	GntBoxPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_BOX(box), NULL);
	priv = gnt_box_get_instance_private(box);

	return g_list_copy(priv->list);
}

void gnt_box_sync_children(GntBox *box)
{
	GntBoxPrivate *priv = NULL;
	GntWidget *widget = NULL;
	WINDOW *widget_window;
	GList *iter;
	gint widgetx, widgety, widgetwidth, widgetheight;
	int pos;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	widget = GNT_WIDGET(box);
	widget_window = gnt_widget_get_window(widget);
	gnt_widget_get_position(widget, &widgetx, &widgety);
	gnt_widget_get_internal_size(widget, &widgetwidth, &widgetheight);
	pos = gnt_widget_get_has_border(widget) ? 1 : 0;

	if (!priv->active) {
		find_focusable_widget(box);
	}

	for (iter = priv->list; iter; iter = iter->next) {
		GntWidget *w = GNT_WIDGET(iter->data);
		WINDOW *wwin;
		int height, width;
		gint x, y;

		if (G_UNLIKELY(w == NULL)) {
			g_warn_if_reached();
			continue;
		}

		if (!gnt_widget_get_visible(w))
			continue;

		if (GNT_IS_BOX(w))
			gnt_box_sync_children(GNT_BOX(w));

		gnt_widget_get_size(w, &width, &height);

		gnt_widget_get_position(w, &x, &y);
		x -= widgetx;
		y -= widgety;

		if (priv->vertical) {
			x = pos;
			if (priv->alignment == GNT_ALIGN_RIGHT) {
				x += widgetwidth - width;
			} else if (priv->alignment == GNT_ALIGN_MID) {
				x += (widgetwidth - width) / 2;
			}
			if (x + width > widgetwidth - pos) {
				x -= x + width - (widgetwidth - pos);
			}
		} else {
			y = pos;
			if (priv->alignment == GNT_ALIGN_BOTTOM) {
				y += widgetheight - height;
			} else if (priv->alignment == GNT_ALIGN_MID) {
				y += (widgetheight - height) / 2;
			}
			if (y + height >= widgetheight - pos) {
				y = widgetheight - height - pos;
			}
		}

		wwin = gnt_widget_get_window(w);
		copywin(wwin, widget_window, 0, 0, y, x, y + height - 1,
		        x + width - 1, FALSE);
		gnt_widget_set_position(w, x + widgetx, y + widgety);
		if (w == priv->active) {
			wmove(widget_window, y + getcury(wwin),
			      x + getcurx(wwin));
		}
	}
}

void gnt_box_set_alignment(GntBox *box, GntAlignment alignment)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	priv->alignment = alignment;
}

void gnt_box_remove(GntBox *box, GntWidget *widget)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	priv->list = g_list_remove(priv->list, widget);
	if (gnt_widget_get_take_focus(widget) &&
	    gnt_widget_get_parent(GNT_WIDGET(box)) == NULL && priv->focus) {
		if (widget == priv->active) {
			find_next_focus(priv);
			if (priv->active == widget) {
				/* There's only one widget */
				priv->active = NULL;
			}
		}
		priv->focus = g_list_remove(priv->focus, widget);
	}

	if (gnt_widget_get_mapped(GNT_WIDGET(box)))
		gnt_widget_draw(GNT_WIDGET(box));
}

void gnt_box_remove_all(GntBox *box)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	g_list_free_full(priv->list, (GDestroyNotify)gnt_widget_destroy);
	g_list_free(priv->focus);
	priv->list = NULL;
	priv->focus = NULL;
	gnt_widget_set_internal_size(GNT_WIDGET(box), 0, 0);
}

void gnt_box_readjust(GntBox *box)
{
	GntBoxPrivate *priv = NULL;
	GList *iter;
	GntWidget *wid;
	int width, height;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	if (gnt_widget_get_parent(GNT_WIDGET(box)) != NULL) {
		return;
	}

	for (iter = priv->list; iter; iter = iter->next) {
		GntWidget *w = iter->data;

		if (G_UNLIKELY(w == NULL)) {
			g_warn_if_reached();
			continue;
		}

		if (GNT_IS_BOX(w))
			gnt_box_readjust(GNT_BOX(w));
		else
		{
			gnt_widget_set_mapped(w, FALSE);
			gnt_widget_set_internal_size(w, 0, 0);
		}
	}

	wid = GNT_WIDGET(box);
	gnt_widget_set_mapped(wid, FALSE);
	gnt_widget_set_internal_size(wid, 0, 0);

	if (gnt_widget_get_parent(wid) == NULL) {
		g_list_free(priv->focus);
		priv->focus = NULL;
		priv->active = NULL;
		gnt_widget_size_request(wid);
		gnt_widget_get_size(wid, &width, &height);
		gnt_screen_resize_widget(wid, width, height);
		find_focusable_widget(box);
	}
}

void gnt_box_set_fill(GntBox *box, gboolean fill)
{
	GntBoxPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	priv->fill = fill;
}

/* Internal. */
GntWidget *
gnt_box_get_active(GntBox *box)
{
	GntBoxPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_BOX(box), NULL);
	priv = gnt_box_get_instance_private(box);
	return priv->active;
}

void gnt_box_move_focus(GntBox *box, int dir)
{
	GntBoxPrivate *priv = NULL;
	GntWidget *now;

	g_return_if_fail(GNT_IS_BOX(box));
	priv = gnt_box_get_instance_private(box);

	if (priv->active == NULL) {
		find_focusable_widget(box);
		return;
	}

	now = priv->active;

	if (dir == 1)
		find_next_focus(priv);
	else if (dir == -1)
		find_prev_focus(priv);

	if (now != priv->active) {
		gnt_widget_set_focus(now, FALSE);
		gnt_widget_set_focus(priv->active, TRUE);
	}

	if (gnt_widget_get_window(GNT_WIDGET(box))) {
		gnt_widget_draw(GNT_WIDGET(box));
	}
}

void gnt_box_give_focus_to_child(GntBox *box, GntWidget *widget)
{
	GntBoxPrivate *priv = NULL;
	GList *find;
	gpointer now;

	g_return_if_fail(GNT_IS_BOX(box));

	box = GNT_BOX(gnt_widget_get_toplevel(GNT_WIDGET(box)));
	priv = gnt_box_get_instance_private(box);

	find = g_list_find(priv->focus, widget);
	now = priv->active;
	if (find) {
		priv->active = widget;
	}
	if (now && now != priv->active) {
		gnt_widget_set_focus(now, FALSE);
		gnt_widget_set_focus(priv->active, TRUE);
	}

	if (gnt_widget_get_window(GNT_WIDGET(box))) {
		gnt_widget_draw(GNT_WIDGET(box));
	}
}

