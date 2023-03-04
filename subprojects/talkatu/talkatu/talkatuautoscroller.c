/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include <talkatu/talkatuautoscroller.h>

/**
 * TalkatuAutoScroller:
 *
 * This is a simple subclass of [class@Gtk.Adjustment] that has helpers for
 * keyboard navigation as well as the ability to automatically scroll to the
 * max when new items are added if the widget was already scrolled all the
 * way to the bottom.
 */
struct _TalkatuAutoScroller {
	GtkAdjustment parent;

	gboolean auto_scroll;
};

G_DEFINE_TYPE(TalkatuAutoScroller, talkatu_auto_scroller, GTK_TYPE_ADJUSTMENT)

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
talkatu_auto_scroller_scroll(gpointer data) {
	GtkAdjustment *adjustment = data;

	gdouble upper, pagesize;

	upper = gtk_adjustment_get_upper(adjustment);
	pagesize = gtk_adjustment_get_page_size(adjustment);

	gtk_adjustment_set_value(adjustment, upper - pagesize);

	return G_SOURCE_REMOVE;
}

static void
talkatu_auto_scroller_changed(GtkAdjustment *adjustment) {
	TalkatuAutoScroller *auto_scroller = TALKATU_AUTO_SCROLLER(adjustment);

	if(auto_scroller->auto_scroll) {
		/* If we set the value here, we interrupt the updates to the other
		 * values of the adjustment which breaks the adjustment.
		 *
		 * The specific behavior we've seen is that the upper property doesn't
		 * get updated when we call set value here. Which means you can't even
		 * scroll down anymore in a scrolled window because you're already at
		 * the upper bound. However, if you scroll up, even if there's no where
		 * to scroll, it will update the adjustment's properties and you can
		 * then scroll down.
		 *
		 * By using a timeout to set the value during the next main loop
		 * iteration we avoid this problem.
		 */
		g_timeout_add(0, talkatu_auto_scroller_scroll, adjustment);
	}
}

static void
talkatu_auto_scroller_value_changed(GtkAdjustment *adjustment) {
	TalkatuAutoScroller *auto_scroller = TALKATU_AUTO_SCROLLER(adjustment);
	gdouble current, upper, pagesize;

	current = gtk_adjustment_get_value(adjustment);
	upper = gtk_adjustment_get_upper(adjustment);
	pagesize = gtk_adjustment_get_page_size(adjustment);

	auto_scroller->auto_scroll = (current + pagesize >= upper);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_auto_scroller_init(TalkatuAutoScroller *auto_scroller) {
	auto_scroller->auto_scroll = TRUE;
}

static void
talkatu_auto_scroller_class_init(TalkatuAutoScrollerClass *klass) {
	GtkAdjustmentClass *adjustment_class = GTK_ADJUSTMENT_CLASS(klass);

	adjustment_class->changed = talkatu_auto_scroller_changed;
	adjustment_class->value_changed = talkatu_auto_scroller_value_changed;
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_auto_scroller_new:
 *
 * Creates a new #TalkatuAutoScroller.
 *
 * Returns: (transfer full): The new #TalkatuAutoScroller instance.
 */
GtkAdjustment *
talkatu_auto_scroller_new(void) {
	return g_object_new(TALKATU_TYPE_AUTO_SCROLLER, NULL);
}

/**
 * talkatu_auto_scroller_decrement:
 * @auto_scroller: The #TalkatuAutoScroller instance.
 *
 * Decrements the value of @auto_scroller by a page increment.
 */
void
talkatu_auto_scroller_decrement(TalkatuAutoScroller *auto_scroller) {
	gdouble value = 0.0;

	g_return_if_fail(TALKATU_IS_AUTO_SCROLLER(auto_scroller));

	value = gtk_adjustment_get_value(GTK_ADJUSTMENT(auto_scroller));
	value -= gtk_adjustment_get_page_increment(GTK_ADJUSTMENT(auto_scroller));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(auto_scroller), value);
}

/**
 * talkatu_auto_scroller_increment:
 * @auto_scroller: The #TalkatuAutoScroller instance.
 *
 * Increments the value of @auto_scroller by a page increment.
 */
void
talkatu_auto_scroller_increment(TalkatuAutoScroller *auto_scroller) {
	gdouble value = 0.0;

	g_return_if_fail(TALKATU_IS_AUTO_SCROLLER(auto_scroller));

	value = gtk_adjustment_get_value(GTK_ADJUSTMENT(auto_scroller));
	value += gtk_adjustment_get_page_increment(GTK_ADJUSTMENT(auto_scroller));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(auto_scroller), value);
}
