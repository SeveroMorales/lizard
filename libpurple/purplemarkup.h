/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_MARKUP_H
#define PURPLE_MARKUP_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * purple_markup_html_to_xhtml:
 * @html:       The HTML markup.
 * @dest_xhtml: The destination XHTML output.
 * @dest_plain: The destination plain-text output.
 *
 * Converts HTML markup to XHTML.
 */
void purple_markup_html_to_xhtml(const char *html, char **dest_xhtml,
							   char **dest_plain);

/**
 * purple_markup_strip_html:
 * @str: The string to strip HTML from.
 *
 * Strips HTML tags from a string.
 *
 * Returns: The new string without HTML.  You must g_free this string
 *         when finished with it.
 */
char *purple_markup_strip_html(const char *str);

/**
 * purple_markup_linkify:
 * @str: The string to linkify.
 *
 * Adds the necessary HTML code to turn URIs into HTML links in a string.
 *
 * Returns: The new string with all URIs surrounded in standard
 *          HTML &lt;a href="whatever"&gt;&lt;/a&gt; tags. You must g_free()
 *          this string when finished with it.
 */
char *purple_markup_linkify(const char *str);

/**
 * purple_unescape_text:
 * @text: The string in which to unescape any HTML entities
 *
 * Unescapes HTML entities to their literal characters in the text.
 * For example "&amp;amp;" is replaced by '&amp;' and so on.  Also converts
 * numerical entities (e.g. "&amp;\#38;" is also '&amp;').
 *
 * This function currently supports the following named entities:
 *     "&amp;amp;", "&amp;lt;", "&amp;gt;", "&amp;copy;", "&amp;quot;",
 *     "&amp;reg;", "&amp;apos;"
 *
 * purple_unescape_html() is similar, but also converts "&lt;br&gt;" into "\n".
 *
 *  See purple_unescape_html()
 *
 * Returns: The text with HTML entities literalized.  You must g_free
 *         this string when finished with it.
 */
char *purple_unescape_text(const char *text);

/**
 * purple_unescape_html:
 * @html: The string in which to unescape any HTML entities
 *
 * Unescapes HTML entities to their literal characters and converts
 * "&lt;br&gt;" to "\n".  See purple_unescape_text() for more details.
 *
 *  See purple_unescape_text()
 *
 * Returns: The text with HTML entities literalized.  You must g_free
 *         this string when finished with it.
 */
char *purple_unescape_html(const char *html);

/**
 * purple_markup_slice:
 * @str: The input NUL terminated, HTML, UTF-8 (or ASCII) string.
 * @x: The character offset into an unformatted version of str to begin at.
 * @y: The character offset (into an unformatted version of str) of one past
 *     the last character to include in the slice.
 *
 * Returns a newly allocated substring of the HTML UTF-8 string "str".
 * The markup is preserved such that the substring will have the same
 * formatting as original string, even though some tags may have been
 * opened before "x", or may close after "y". All open tags are closed
 * at the end of the returned string, in the proper order.
 *
 * Note that x and y are in character offsets, not byte offsets, and
 * are offsets into an unformatted version of str. Because of this,
 * this function may be sensitive to changes in GtkIMHtml and may break
 * when used with other UI's. libpurple users are encouraged to report and
 * work out any problems encountered.
 *
 * Returns: The HTML slice of string, with all formatting retained.
 */
char *purple_markup_slice(const char *str, guint x, guint y);

/**
 * purple_markup_get_tag_name:
 * @tag: The string starting a HTML tag.
 *
 * Returns a newly allocated string containing the name of the tag
 * located at "tag". Tag is expected to point to a '<', and contain
 * a '>' sometime after that. If there is no '>' and the string is
 * not NUL terminated, this function can be expected to segfault.
 *
 * Returns: A string containing the name of the tag.
 */
char *purple_markup_get_tag_name(const char *tag);

/**
 * purple_markup_unescape_entity:
 * @text:   A string containing an HTML entity.
 * @length: If not %NULL, the string length of the entity is stored in this location.
 *
 * Returns a constant string of the character representation of the HTML
 * entity pointed to by @text. For example, purple_markup_unescape_entity("&amp;amp;")
 * will return "&amp;". The @text variable is expected to point to an '&amp;',
 * the first character of the entity. If given an unrecognized entity, the function
 * returns %NULL.
 *
 * Note that this function, unlike purple_unescape_html(), does not search
 * the string for the entity, does not replace the entity, and does not
 * return a newly allocated string.
 *
 * Returns: A constant string containing the character representation of the given entity.
 */
const char * purple_markup_unescape_entity(const char *text, int *length);

G_END_DECLS

#endif /* PURPLE_MARKUP_H */
