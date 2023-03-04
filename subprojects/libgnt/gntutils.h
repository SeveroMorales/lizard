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

#ifndef GNT_UTILS_H
#define GNT_UTILS_H

#include <glib.h>

#include "gnttextview.h"
#include "gntwidget.h"

/**
 * GntDuplicateFunc:
 * @data: The data to be copied.
 *
 * A function used to duplicate some data in a hash table. See
 * gnt_hash_table_duplicate().
 */
typedef gpointer (*GntDuplicateFunc)(gconstpointer data);

/**
 * gnt_util_get_text_bound:
 * @text:     The text to be displayed.
 * @width:    The width required is set here, if not %NULL.
 * @height:   The height required is set here, if not %NULL.
 *
 * Compute the width and height required to view the text on the screen.
 */
void gnt_util_get_text_bound(const char *text, int *width, int *height);

/* excluding *end */
/**
 * gnt_util_onscreen_width:
 * @start: The beginning of the string.
 * @end: The end of the string. The width returned is the width up to (but not
 *       including) end. If end is %NULL, then start is considered as a
 *       %NULL-terminated string.
 *
 * Get the onscreen width of a string, or a substring.
 *
 * Returns:       The on-screen width of the string.
 */
int gnt_util_onscreen_width(const char *start, const char *end);

/**
 * gnt_util_onscreen_width_to_pointer:
 * @str: The string.
 * @len: The length to consider. If non-positive, the entire screenlength is used.
 * @w: The actual width of the string up to the returned offset, if not %NULL.
 *
 * Computes and returns the string after a specific number of onscreen characters.
 *
 * Returns:     The string after len offset.
 */
const char *gnt_util_onscreen_width_to_pointer(const char *str, int len, int *w);

/**
 * gnt_util_onscreen_fit_string:
 * @string:  The string.
 * @maxw:    The width that the string should fit into. If maxw is <= 0,
 *                then the available maximum width is used.
 *
 * Inserts newlines in 'string' where necessary so that its onscreen width is
 * no more than 'maxw'.
 *
 * Returns:  A newly allocated string that needs to be freed by the caller.
 */
char * gnt_util_onscreen_fit_string(const char *string, int maxw);

/**
 * gnt_hash_table_duplicate:
 * @src:                     The source hashtable.
 * @hash: (scope call):      The hash-function to use.
 * @equal:                   The hash-equal function to use.
 * @key_d:                   The key-destroy function to use.
 * @value_d:                 The value-destroy function to use.
 * @key_dup: (scope call):   The function to use to duplicate the key.
 * @value_dup: (scope call): The function to use to duplicate the value.
 *
 * Duplicate the contents of a hastable.
 *
 * Returns: (transfer full): The new hashtable.
 */
GHashTable * gnt_hash_table_duplicate(GHashTable *src, GHashFunc hash, GEqualFunc equal, GDestroyNotify key_d, GDestroyNotify value_d, GntDuplicateFunc key_dup, GntDuplicateFunc value_dup);

/**
 * gnt_util_parse_widgets:
 * @string: The XML string.
 * @num:    The number of widgets to return.
 * @...:    A list of #GntWidget** pointers to place the result.
 *
 * Parse widgets from an XML description. For example,
 *
 * <programlisting>
 * GntWidget *win, *button;
 * gnt_util_parse_widgets("\
 *      &lt;vwindow id='0' fill='0' align='2'&gt;     \
 *          &lt;label&gt;This is a test&lt;/label&gt;       \
 *          &lt;button id='1'&gt;OK&lt;/button&gt;          \
 *      &lt;/vwindow&gt;",
 *   2, &win, &button);
 * </programlisting>
 */
void gnt_util_parse_widgets(const char *string, int num, ...);

/**
 * gnt_util_parse_xhtml_to_textview:
 * @string:   The XHTML string
 * @tv:       The GntTextView
 *
 * Parse an XHTML string and add it in a GntTextView with
 * appropriate text flags.
 *
 * Returns:  %TRUE if the string was added to the textview properly, %FALSE otherwise.
 *
 * Since: 2.2.0
 */
gboolean gnt_util_parse_xhtml_to_textview(const char *string, GntTextView *tv);

/**
 * gnt_util_set_trigger_widget:
 * @widget:  The widget
 * @key:     The key to trigger the button
 * @button:  The button to trigger
 *
 * Make some keypress activate a button when some key is pressed with 'wid' in focus.
 *
 * Since: 2.1.0
 */
void gnt_util_set_trigger_widget(GntWidget *widget, const char *key, GntWidget *button);

#endif /* GNT_UTILS_H */
