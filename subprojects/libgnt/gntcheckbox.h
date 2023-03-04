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

#ifndef GNT_CHECK_BOX_H
#define GNT_CHECK_BOX_H

#include "gntbutton.h"
#include "gntcolors.h"
#include "gntkeys.h"

#define GNT_TYPE_CHECK_BOX gnt_check_box_get_type()

/**
 * GntCheckBoxClass:
 * @toggled: The class closure for the #GntCheckBox::toggled signal.
 *
 * The class structure for #GntCheckBox.
 */
struct _GntCheckBoxClass
{
	/*< private >*/
	GntButtonClass parent;

	/*< public >*/
	void (*toggled)(void);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntCheckBox, gnt_check_box, GNT, CHECK_BOX, GntButton)

/**
 * gnt_check_box_new:
 * @text: The text for the checkbox.
 *
 * Create a new checkbox.
 *
 * Returns:  The newly created checkbox.
 */
GntWidget * gnt_check_box_new(const char *text);

/**
 * gnt_check_box_set_checked:
 * @box:   The checkbox.
 * @set:   %TRUE if the checkbox should be selected, %FALSE otherwise.
 *
 * Set whether the checkbox should be checked or not.
 */
void gnt_check_box_set_checked(GntCheckBox *box, gboolean set);

/**
 * gnt_check_box_get_checked:
 * @box:  The checkbox.
 *
 * Return the checked state of the checkbox.
 *
 * Returns:     %TRUE if the checkbox is selected, %FALSE otherwise.
 */
gboolean gnt_check_box_get_checked(GntCheckBox *box);

G_END_DECLS

#endif /* GNT_CHECK_BOX_H */
