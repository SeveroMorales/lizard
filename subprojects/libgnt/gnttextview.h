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

#ifndef GNT_TEXT_VIEW_H
#define GNT_TEXT_VIEW_H

#include "gntcolors.h"
#include "gntkeys.h"
#include "gntwidget.h"

#define GNT_TYPE_TEXT_VIEW gnt_text_view_get_type()

/**
 * GntTextViewFlag:
 * @GNT_TEXT_VIEW_NO_SCROLL: Do not add a scrollbar to the view.
 * @GNT_TEXT_VIEW_WRAP_CHAR: Wrap on characters (instead of words).
 * @GNT_TEXT_VIEW_TOP_ALIGN: Align text to the top of the view. Since: 2.1.1
 *
 * Flags that determine the behaviour of a #GntTextView.
 *
 * Since: 2.1.0
 */
typedef enum
{
	GNT_TEXT_VIEW_NO_SCROLL     = 1 << 0,
	GNT_TEXT_VIEW_WRAP_CHAR     = 1 << 1,
	GNT_TEXT_VIEW_TOP_ALIGN     = 1 << 2,
} GntTextViewFlag;

/**
 * GntTextFormatFlags:
 * @GNT_TEXT_FLAG_NORMAL: Display the text normally.
 * @GNT_TEXT_FLAG_BOLD: Display the text in bold text.
 * @GNT_TEXT_FLAG_UNDERLINE: Display the text with underlining.
 * @GNT_TEXT_FLAG_BLINK: Display the text with blinking.
 * @GNT_TEXT_FLAG_DIM: Display the text dimmed.
 * @GNT_TEXT_FLAG_HIGHLIGHT: Display the text with highlighting.
 *
 * Flags that determine the display style of portions of text in a
 * #GntTextView.
 */
typedef enum
{
	GNT_TEXT_FLAG_NORMAL      = 0,
	GNT_TEXT_FLAG_BOLD        = 1 << 0,
	GNT_TEXT_FLAG_UNDERLINE   = 1 << 1,
	GNT_TEXT_FLAG_BLINK       = 1 << 2,
	GNT_TEXT_FLAG_DIM         = 1 << 3,
	GNT_TEXT_FLAG_HIGHLIGHT   = 1 << 4,
} GntTextFormatFlags;

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GntTextView, gnt_text_view, GNT, TEXT_VIEW, GntWidget)

/**
 * gnt_text_view_new:
 *
 * Create a new textview.
 *
 * Returns: The newly created textview.
 */
GntWidget *gnt_text_view_new(void);

/**
 * gnt_text_view_scroll:
 * @view:     The textview to scroll.
 * @scroll:   scroll > 0 means scroll up, < 0 means scroll down, == 0 means scroll to the end.
 *
 * Scroll the textview.
 */
void gnt_text_view_scroll(GntTextView *view, int scroll);

/**
 * gnt_text_view_append_text_with_flags:
 * @view:   The textview.
 * @text:   The text to append to the textview.
 * @flags:  The text-flags to apply to the new text.
 *
 * Append new text in a textview.
 */
void gnt_text_view_append_text_with_flags(GntTextView *view, const char *text, GntTextFormatFlags flags);

/**
 * gnt_text_view_append_text_with_tag:
 * @view:   The textview.
 * @text:   The text to append.
 * @flags:  The text-flags to apply to the new text.
 * @tag:    The tag for the appended text, so it can be changed later (see gnt_text_view_tag_change())
 *
 * Append text in the textview, with some identifier (tag) for the added text.
 */
void gnt_text_view_append_text_with_tag(GntTextView *view, const char *text, GntTextFormatFlags flags, const char *tag);

/**
 * gnt_text_view_get_text:
 * @view:   The textview.
 *
 * Get the text of the textview.
 *
 * Returns: The text.
 *
 * Since: 2.14.0
 */
const gchar *gnt_text_view_get_text(GntTextView *view);

/**
 * gnt_text_view_next_line:
 * @view:  The textview.
 *
 * Move the cursor to the beginning of the next line and resets text-attributes.
 * It first completes the current line with the current text-attributes.
 */
void gnt_text_view_next_line(GntTextView *view);

/**
 * gnt_text_view_clear:
 * @view:  The textview.
 *
 * Clear the contents of the textview.
 */
void gnt_text_view_clear(GntTextView *view);

/**
 * gnt_text_view_get_lines_below:
 * @view:  The textview.
 *
 * The number of lines below the bottom-most visible line.
 *
 * Returns:  Number of lines below the bottom-most visible line.
 */
int gnt_text_view_get_lines_below(GntTextView *view);

/**
 * gnt_text_view_get_lines_above:
 * @view:  The textview.
 *
 * The number of lines above the topmost visible line.
 *
 * Returns:  Number of lines above the topmost visible line.
 */
int gnt_text_view_get_lines_above(GntTextView *view);

/**
 * gnt_text_view_tag_change:
 * @view:   The textview.
 * @name:   The name of the tag.
 * @text:   The new text for the text. If 'text' is %NULL, the tag is removed.
 * @all:    %TRUE if all of the instancess of the tag should be changed, %FALSE if
 *               only the first instance should be changed.
 *
 * Change the text of a tag.
 *
 * Returns:  The number of instances changed.
 */
int gnt_text_view_tag_change(GntTextView *view, const char *name, const char *text, gboolean all);

/**
 * gnt_text_view_attach_scroll_widget:
 * @view:    The textview.
 * @widget:  The trigger widget.
 *
 * Setup hooks so that pressing up/down/page-up/page-down keys when 'widget' is
 * in focus scrolls the textview.
 */
void gnt_text_view_attach_scroll_widget(GntTextView *view, GntWidget *widget);

/**
 * gnt_text_view_attach_pager_widget:
 * @view:    The textview.
 * @pager:   The widget to trigger the PAGER.
 *
 * Setup appropriate hooks so that pressing some keys when the 'pager' widget
 * is in focus triggers the PAGER to popup with the contents of the textview
 * in it.
 *
 * The default key-combination to trigger the pager is a-v, and the default
 * PAGER application is $PAGER. Both can be changed in ~/.gntrc like this:
 *
 * <programlisting>
 * [pager]
 * key = a-v
 * path = /path/to/pager
 * </programlisting>
 */
void gnt_text_view_attach_pager_widget(GntTextView *view, GntWidget *pager);

/**
 * gnt_text_view_attach_editor_widget:
 * @view:     The textview.
 * @widget:   The widget to trigger the EDITOR.
 *
 * Setup appropriate hooks so that pressing some keys when 'widget'
 * is in focus triggers the EDITOR to popup with the contents of the textview
 * in it.
 *
 * The default key-combination to trigger the pager is a-e, and the default
 * EDITOR application is $EDITOR. Both can be changed in ~/.gntrc like this:
 *
 * <programlisting>
 * [editor]
 * key = a-e
 * path = /path/to/editor
 * </programlisting>
 */
void gnt_text_view_attach_editor_widget(GntTextView *view, GntWidget *widget);

/**
 * gnt_text_view_set_flag:
 * @view:  The textview widget
 * @flag:  The flag to set
 *
 * Set a #GntTextViewFlag for the textview widget.
 *
 * Since: 2.1.0
 */
void gnt_text_view_set_flag(GntTextView *view, GntTextViewFlag flag);

G_END_DECLS

#endif /* GNT_TEXT_VIEW_H */
