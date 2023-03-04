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

#ifndef GNT_PROGRESS_BAR_H
#define GNT_PROGRESS_BAR_H

#include "gntwidget.h"

#define GNT_TYPE_PROGRESS_BAR          (gnt_progress_bar_get_type ())

/**
 * GntProgressBarOrientation:
 * @GNT_PROGRESS_LEFT_TO_RIGHT: The progress bar fills from left to right.
 * @GNT_PROGRESS_RIGHT_TO_LEFT: The progress bar fills from right to left.
 * @GNT_PROGRESS_BOTTOM_TO_TOP: The progress bar fills from bottom to top.
 * @GNT_PROGRESS_TOP_TO_BOTTOM: The progress bar fills from top to bottom.
 *
 * The orientation of a #GntProgressBar.
 */
typedef enum
{
   GNT_PROGRESS_LEFT_TO_RIGHT,
   GNT_PROGRESS_RIGHT_TO_LEFT,
   GNT_PROGRESS_BOTTOM_TO_TOP,
   GNT_PROGRESS_TOP_TO_BOTTOM,
} GntProgressBarOrientation;

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GntProgressBar, gnt_progress_bar, GNT, PROGRESS_BAR,
		GntWidget)

/**
 * gnt_progress_bar_new:
 *
 * Create a new GntProgressBar
 *
 * Returns: The new GntProgressBar
 *
 * Since: 2.6.0
 */
GntWidget *
gnt_progress_bar_new (void);

/**
 * gnt_progress_bar_set_fraction:
 * @pbar: The GntProgressBar
 * @fraction: The value between 0 and 1 to display
 *
 * Set the progress for a progress bar
 *
 * Since: 2.6.0
 */
void
gnt_progress_bar_set_fraction (GntProgressBar *pbar, gdouble fraction);

/**
 * gnt_progress_bar_set_orientation:
 * @pbar: The GntProgressBar
 * @orientation: The orientation to use
 *
 * Set the orientation for a progress bar
 *
 * Since: 2.6.0
 */
void
gnt_progress_bar_set_orientation (GntProgressBar *pbar, GntProgressBarOrientation orientation);

/**
 * gnt_progress_bar_set_show_progress:
 * @pbar: The GntProgressBar
 * @show: A boolean indicating if the value is shown
 *
 * Controls whether the progress value is shown
 *
 * Since: 2.6.0
 */
void
gnt_progress_bar_set_show_progress (GntProgressBar *pbar, gboolean show);

/**
 * gnt_progress_bar_get_fraction:
 * @pbar: The GntProgressBar
 *
 * Get the progress that is displayed
 *
 * Returns: The progress displayed as a value between 0 and 1
 *
 * Since: 2.6.0
 */
gdouble
gnt_progress_bar_get_fraction (GntProgressBar *pbar);

/**
 * gnt_progress_bar_get_orientation:
 * @pbar: The GntProgressBar
 *
 * Get the orientation for the progress bar
 *
 * Returns: The current orientation of the progress bar
 *
 * Since: 2.6.0
 */
GntProgressBarOrientation
gnt_progress_bar_get_orientation (GntProgressBar *pbar);

/**
 * gnt_progress_bar_get_show_progress:
 * @pbar: The GntProgressBar
 *
 * Get a boolean describing if the progress value is shown
 *
 * Returns: %TRUE if the progress value is shown, %FALSE otherwise.
 *
 * Since: 2.6.0
 */
gboolean
gnt_progress_bar_get_show_progress (GntProgressBar *pbar);

G_END_DECLS

#endif /* GNT_PROGRESS_BAR_H */
