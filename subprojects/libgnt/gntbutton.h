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

#ifndef GNT_BUTTON_H
#define GNT_BUTTON_H

#include <glib.h>
#include <glib-object.h>
#include "gntwidget.h"

#define GNT_TYPE_BUTTON gnt_button_get_type()

/**
 * GntButtonClass:
 *
 * The class structure for #GntButton.
 */
struct _GntButtonClass
{
	/*< private >*/
	GntWidgetClass parent;

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntButton, gnt_button, GNT, BUTTON, GntWidget)

/**
 * gnt_button_new:
 * @text:   The text for the button.
 *
 * Create a new button.
 *
 * Returns:  The newly created button.
 */
GntWidget *gnt_button_new(const gchar *text);

/**
 * gnt_button_set_text:
 * @button: The button.
 * @text:   The text for the button.
 *
 * Set the text of a button.
 *
 * Since: 2.14.0
 */
void gnt_button_set_text(GntButton *button, const gchar *text);

/**
 * gnt_button_get_text:
 * @button: The button.
 *
 * Get the text of a button.
 *
 * Returns: The text for the button.
 *
 * Since: 2.14.0
 */
const gchar *gnt_button_get_text(GntButton *button);

G_END_DECLS

#endif /* GNT_BUTTON_H */
