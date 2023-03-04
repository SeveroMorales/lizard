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

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_SLIDER_H
#define GNT_SLIDER_H

#include "gntlabel.h"
#include "gntwidget.h"

G_BEGIN_DECLS

#define GNT_TYPE_SLIDER gnt_slider_get_type()

G_DECLARE_DERIVABLE_TYPE(GntSlider, gnt_slider, GNT, SLIDER, GntWidget)

/**
 * GntSliderClass:
 * @changed: The class closure for the #GntSlider::changed signal.
 *
 * The class structure for #GntSlider.
 *
 * Since: 2.1.0
 */
struct _GntSliderClass
{
	/*< private >*/
	GntWidgetClass parent;

	/*< public >*/
	void (*changed)(GntSlider *slider, int value);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * gnt_hslider_new:
 * @max: The maximum value for the slider.
 * @min: The minimum value for the slider.
 *
 * Create a new horizontal slider.
 *
 * Returns: The newly created slider.
 *
 * Since: 2.1.0
 */
#define gnt_hslider_new(max, min) gnt_slider_new(FALSE, max, min)

/**
 * gnt_vslider_new:
 * @max: The maximum value for the slider.
 * @min: The minimum value for the slider.
 *
 * Create a new vertical slider.
 *
 * Returns: The newly created slider.
 *
 * Since: 2.1.0
 */
#define gnt_vslider_new(max, min) gnt_slider_new(TRUE, max, min)

/**
 * gnt_slider_new:
 * @orient: A vertical slider is created if %TRUE, otherwise the slider is
 *          horizontal.
 * @max: The maximum value for the slider.
 * @min: The minimum value for the slider.
 *
 * Create a new slider.
 *
 * Returns: The newly created slider.
 *
 * Since: 2.1.0
 */
GntWidget * gnt_slider_new(gboolean orient, int max, int min);

/**
 * gnt_slider_get_vertical:
 * @slider:  The slider
 *
 * Get whether the slider is vertical or not.
 *
 * Returns:  Whether the slider is vertical.
 *
 * Since: 2.14.0
 */
gboolean gnt_slider_get_vertical(GntSlider *slider);

/**
 * gnt_slider_set_range:
 * @slider:  The slider
 * @max:     The maximum value
 * @min:     The minimum value
 *
 * Set the range of the slider.
 *
 * Since: 2.1.0
 */
void gnt_slider_set_range(GntSlider *slider, int max, int min);

/**
 * gnt_slider_get_range:
 * @slider:     The slider
 * @max: (out): The maximum value
 * @min: (out): The minimum value
 *
 * Get the range of the slider.
 *
 * Since: 2.14.0
 */
void gnt_slider_get_range(GntSlider *slider, int *max, int *min);

/**
 * gnt_slider_set_step:
 * @slider:  The slider
 * @step:    The amount for each step
 *
 * Sets the amount of change at each step.
 *
 * Since: 2.1.0
 */
void gnt_slider_set_step(GntSlider *slider, int step);

/**
 * gnt_slider_get_step:
 * @slider:  The slider
 *
 * Gets the amount of change at each step.
 *
 * Returns:  The amount for each step
 *
 * Since: 2.14.0
 */
int gnt_slider_get_step(GntSlider *slider);

/**
 * gnt_slider_set_small_step:
 * @slider:  The slider
 * @step:    The amount for a small step (for the slider)
 *
 * Sets the amount of change a small step.
 *
 * Since: 2.2.0
 */
void gnt_slider_set_small_step(GntSlider *slider, int step);

/**
 * gnt_slider_get_small_step:
 * @slider:  The slider
 *
 * Gets the amount of change for a small step.
 *
 * Returns:  The amount for a small step (of the slider)
 *
 * Since: 2.14.0
 */
int gnt_slider_get_small_step(GntSlider *slider);

/**
 * gnt_slider_set_large_step:
 * @slider:  The slider
 * @step:    The amount for a large step (for the slider)
 *
 * Sets the amount of change a large step.
 *
 * Since: 2.2.0
 */
void gnt_slider_set_large_step(GntSlider *slider, int step);

/**
 * gnt_slider_get_large_step:
 * @slider:  The slider
 *
 * Gets the amount of change for a large step.
 *
 * Returns:  The amount for a large step (of the slider)
 *
 * Since: 2.14.0
 */
int gnt_slider_get_large_step(GntSlider *slider);

/**
 * gnt_slider_advance_step:
 * @slider: The slider
 * @steps:  The number of amounts to change, positive to change forward,
 *          negative to change backward
 *
 * Advance the slider forward or backward.
 *
 * Returns:   The value of the slider after the change
 *
 * Since: 2.1.0
 */
int gnt_slider_advance_step(GntSlider *slider, int steps);

/**
 * gnt_slider_set_value:
 * @slider:  The slider
 * @value:   The current value
 *
 * Set the current value for the slider.
 *
 * Since: 2.1.0
 */
void gnt_slider_set_value(GntSlider *slider, int value);

/**
 * gnt_slider_get_value:
 * @slider: The slider
 *
 * Get the current value for the slider.
 *
 * Since: 2.1.0
 */
int gnt_slider_get_value(GntSlider *slider);

/**
 * gnt_slider_reflect_label:
 * @slider:   The slider
 * @label:    The label to update
 *
 * Update a label with the value of the slider whenever the value changes.
 *
 * Since: 2.1.0
 */
void gnt_slider_reflect_label(GntSlider *slider, GntLabel *label);

G_END_DECLS

#endif /* GNT_SLIDER_H */
