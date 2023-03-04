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
#include "gntcolors.h"
#include "gntkeys.h"
#include "gntslider.h"
#include "gntstyle.h"

#include "gntwidgetprivate.h"

typedef struct
{
	gboolean vertical;
	int max;     /* maximum value */
	int min;     /* minimum value */
	int step;    /* amount to change at each step */
	int current; /* current value */
	int smallstep;
	int largestep;
} GntSliderPrivate;

enum
{
	SIG_VALUE_CHANGED,
	SIGS,
};

static guint signals[SIGS] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GntSlider, gnt_slider, GNT_TYPE_WIDGET)

/* returns TRUE if the value was changed */
static gboolean
sanitize_value(GntSliderPrivate *priv)
{
	if (priv->current < priv->min)
		priv->current = priv->min;
	else if (priv->current > priv->max)
		priv->current = priv->max;
	else
		return FALSE;
	return TRUE;
}

static void
redraw_slider(GntSlider *slider)
{
	GntWidget *widget = GNT_WIDGET(slider);
	if (gnt_widget_get_mapped(widget))
		gnt_widget_draw(widget);
}

static void
slider_value_changed(GntSlider *slider)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	g_signal_emit(slider, signals[SIG_VALUE_CHANGED], 0, priv->current);
}

static void
gnt_slider_draw(GntWidget *widget)
{
	GntSlider *slider = GNT_SLIDER(widget);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	WINDOW *window = gnt_widget_get_window(widget);
	int attr = 0;
	int position, size = 0;

	if (priv->vertical)
		gnt_widget_get_internal_size(widget, NULL, &size);
	else
		gnt_widget_get_internal_size(widget, &size, NULL);

	if (gnt_widget_has_focus(widget))
		attr |= GNT_COLOR_HIGHLIGHT;
	else
		attr |= GNT_COLOR_HIGHLIGHT_D;

	if (priv->max != priv->min)
		position = ((size - 1) * (priv->current - priv->min)) /
		           (priv->max - priv->min);
	else
		position = 0;
	if (priv->vertical) {
		mvwvline(window, size - position, 0,
		         ACS_VLINE | gnt_color_pair(GNT_COLOR_NORMAL) | A_BOLD,
		         position);
		mvwvline(window, 0, 0,
		         ACS_VLINE | gnt_color_pair(GNT_COLOR_NORMAL),
		         size - position);
	} else {
		mvwhline(window, 0, 0,
		         ACS_HLINE | gnt_color_pair(GNT_COLOR_NORMAL) | A_BOLD,
		         position);
		mvwhline(window, 0, position,
		         ACS_HLINE | gnt_color_pair(GNT_COLOR_NORMAL),
		         size - position);
	}

	mvwaddch(window, priv->vertical ? (size - position - 1) : 0,
	         priv->vertical ? 0 : position,
	         ACS_CKBOARD | gnt_color_pair(attr));
}

static void
gnt_slider_size_request(GntWidget *widget)
{
	GntSliderPrivate *priv =
	        gnt_slider_get_instance_private(GNT_SLIDER(widget));
	if (priv->vertical) {
		gnt_widget_set_internal_size(widget, 1, 5);
	} else {
		gnt_widget_set_internal_size(widget, 5, 1);
	}
}

static void
gnt_slider_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
}

static gboolean
step_back(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	gnt_slider_advance_step(slider, -1);
	return TRUE;
}

static gboolean
small_step_back(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->current - priv->smallstep);
	return TRUE;
}

static gboolean
large_step_back(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->current - priv->largestep);
	return TRUE;
}

static gboolean
step_forward(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	gnt_slider_advance_step(slider, 1);
	return TRUE;
}

static gboolean
small_step_forward(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->current + priv->smallstep);
	return TRUE;
}

static gboolean
large_step_forward(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->current + priv->largestep);
	return TRUE;
}

static gboolean
move_min_value(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->min);
	return TRUE;
}

static gboolean
move_max_value(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntSlider *slider = GNT_SLIDER(bindable);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->max);
	return TRUE;
}

static void
gnt_slider_class_init(GntSliderClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->draw = gnt_slider_draw;
	widget_class->map = gnt_slider_map;
	widget_class->size_request = gnt_slider_size_request;

	klass->changed = NULL;

	signals[SIG_VALUE_CHANGED] =
		g_signal_new("changed",
		             G_TYPE_FROM_CLASS(klass),
		             G_SIGNAL_RUN_LAST,
		             G_STRUCT_OFFSET(GntSliderClass, changed),
		             NULL, NULL, NULL,
		             G_TYPE_NONE, 1, G_TYPE_INT);

	gnt_bindable_class_register_action(bindable, "step-backward", step_back, GNT_KEY_LEFT, NULL);
	gnt_bindable_register_binding(bindable, "step-backward", GNT_KEY_DOWN, NULL);
	gnt_bindable_class_register_action(bindable, "step-forward", step_forward, GNT_KEY_RIGHT, NULL);
	gnt_bindable_register_binding(bindable, "step-forward", GNT_KEY_UP, NULL);
	gnt_bindable_class_register_action(bindable, "small-step-backward", small_step_back, GNT_KEY_CTRL_LEFT, NULL);
	gnt_bindable_register_binding(bindable, "small-step-backward", GNT_KEY_CTRL_DOWN, NULL);
	gnt_bindable_class_register_action(bindable, "small-step-forward", small_step_forward, GNT_KEY_CTRL_RIGHT, NULL);
	gnt_bindable_register_binding(bindable, "small-step-forward", GNT_KEY_CTRL_UP, NULL);
	gnt_bindable_class_register_action(bindable, "large-step-backward", large_step_back, GNT_KEY_PGDOWN, NULL);
	gnt_bindable_class_register_action(bindable, "large-step-forward", large_step_forward, GNT_KEY_PGUP, NULL);
	gnt_bindable_class_register_action(bindable, "min-value", move_min_value, GNT_KEY_HOME, NULL);
	gnt_bindable_class_register_action(bindable, "max-value", move_max_value, GNT_KEY_END, NULL);

	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), GNT_BINDABLE_CLASS(klass));
}

static void
gnt_slider_init(GntSlider *slider)
{
	GntWidget *widget = GNT_WIDGET(slider);
	gnt_widget_set_has_shadow(widget, FALSE);
	gnt_widget_set_has_border(widget, FALSE);
	gnt_widget_set_take_focus(widget, TRUE);
	gnt_widget_set_minimum_size(widget, 1, 1);
}

/******************************************************************************
 * GntSlider API
 *****************************************************************************/
GntWidget *gnt_slider_new(gboolean vertical, int max, int min)
{
	GntWidget *widget = g_object_new(GNT_TYPE_SLIDER, NULL);
	GntSlider *slider = GNT_SLIDER(widget);
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);

	priv->vertical = vertical;

	if (vertical) {
		gnt_widget_set_grow_y(widget, TRUE);
	} else {
		gnt_widget_set_grow_x(widget, TRUE);
	}

	gnt_slider_set_range(slider, max, min);
	priv->step = 1;

	return widget;
}

gboolean
gnt_slider_get_vertical(GntSlider *slider)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	g_return_val_if_fail(GNT_IS_SLIDER(slider), FALSE);

	return priv->vertical;
}

void gnt_slider_set_value(GntSlider *slider, int value)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	int old;
	if (priv->current == value)
		return;
	old = priv->current;
	priv->current = value;
	sanitize_value(priv);
	if (old == priv->current)
		return;
	redraw_slider(slider);
	slider_value_changed(slider);
}

int gnt_slider_get_value(GntSlider *slider)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	return priv->current;
}

int gnt_slider_advance_step(GntSlider *slider, int steps)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	gnt_slider_set_value(slider, priv->current + steps * priv->step);
	return priv->current;
}

void gnt_slider_set_step(GntSlider *slider, int step)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	priv->step = step;
}

int
gnt_slider_get_step(GntSlider *slider)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	g_return_val_if_fail(GNT_IS_SLIDER(slider), 0);

	return priv->step;
}

void gnt_slider_set_small_step(GntSlider *slider, int step)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	priv->smallstep = step;
}

int
gnt_slider_get_small_step(GntSlider *slider)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	g_return_val_if_fail(GNT_IS_SLIDER(slider), 0);

	return priv->smallstep;
}

void gnt_slider_set_large_step(GntSlider *slider, int step)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	priv->largestep = step;
}

int
gnt_slider_get_large_step(GntSlider *slider)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	g_return_val_if_fail(GNT_IS_SLIDER(slider), 0);

	return priv->largestep;
}

void gnt_slider_set_range(GntSlider *slider, int max, int min)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	priv->max = MAX(max, min);
	priv->min = MIN(max, min);
	sanitize_value(priv);
}

void
gnt_slider_get_range(GntSlider *slider, int *max, int *min)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	g_return_if_fail(GNT_IS_SLIDER(slider));

	*max = priv->max;
	*min = priv->min;
}

static void
update_label(GntSlider *slider, int current_value, GntLabel *label)
{
	GntSliderPrivate *priv = gnt_slider_get_instance_private(slider);
	char value[256];
	g_snprintf(value, sizeof(value), "%d/%d", current_value, priv->max);
	gnt_label_set_text(label, value);
}

void gnt_slider_reflect_label(GntSlider *slider, GntLabel *label)
{
	g_signal_connect(G_OBJECT(slider), "changed", G_CALLBACK(update_label), label);
}

