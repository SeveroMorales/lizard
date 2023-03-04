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

#ifndef GNT_LABEL_H
#define GNT_LABEL_H

#include "gntncurses.h"
#include "gnttextview.h"
#include "gntwidget.h"

#define GNT_TYPE_LABEL gnt_label_get_type()

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GntLabel, gnt_label, GNT, LABEL, GntWidget)

/**
 * gnt_label_new:
 * @text:  The text of the label.
 *
 * Create a new GntLabel.
 *
 * Returns:  The newly created label.
 */
GntWidget * gnt_label_new(const char *text);

/**
 * gnt_label_new_with_format:
 * @text:    The text.
 * @flags:   Text attributes for the text.
 *
 * Create a new label with specified text attributes.
 *
 * Returns:  The newly created label.
 */
GntWidget * gnt_label_new_with_format(const char *text, GntTextFormatFlags flags);

/**
 * gnt_label_set_text:
 * @label:  The label.
 * @text:   The new text to set in the label.
 *
 * Change the text of a label.
 */
void gnt_label_set_text(GntLabel *label, const char *text);

G_END_DECLS

#endif /* GNT_LABEL_H */

