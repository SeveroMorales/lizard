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

#if !defined(TALKATU_GLOBAL_HEADER_INSIDE) && !defined(TALKATU_COMPILATION)
#error "only <talkatu.h> may be included directly"
#endif

#ifndef TALKATU_TAG_H
#define TALKATU_TAG_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#define TALKATU_TAG_PREFIX        "talkatu:"
#define TALKATU_TAG_PREFIX_LEN    (sizeof(TALKATU_TAG_PREFIX)-1)

#define TALKATU_TAG_BOLD          TALKATU_TAG_PREFIX "bold"
#define TALKATU_TAG_ITALIC        TALKATU_TAG_PREFIX "italic"
#define TALKATU_TAG_UNDERLINE     TALKATU_TAG_PREFIX "underline"
#define TALKATU_TAG_STRIKETHROUGH TALKATU_TAG_PREFIX "strikethrough"
#define TALKATU_TAG_SUBSCRIPT     TALKATU_TAG_PREFIX "subscript"
#define TALKATU_TAG_SUPERSCRIPT   TALKATU_TAG_PREFIX "superscript"
#define TALKATU_TAG_PRE           TALKATU_TAG_PREFIX "preformatted"
#define TALKATU_TAG_CODE          TALKATU_TAG_PREFIX "code"
#define TALKATU_TAG_SEARCH        TALKATU_TAG_PREFIX "search"
#define TALKATU_TAG_H1            TALKATU_TAG_PREFIX "header1"
#define TALKATU_TAG_H2            TALKATU_TAG_PREFIX "header2"
#define TALKATU_TAG_H3            TALKATU_TAG_PREFIX "header3"
#define TALKATU_TAG_H4            TALKATU_TAG_PREFIX "header4"
#define TALKATU_TAG_H5            TALKATU_TAG_PREFIX "header5"
#define TALKATU_TAG_H6            TALKATU_TAG_PREFIX "header6"
#define TALKATU_TAG_ANCHOR        TALKATU_TAG_PREFIX "anchor"
#define TALKATU_TAG_DL            TALKATU_TAG_PREFIX "description-list"
#define TALKATU_TAG_DT            TALKATU_TAG_PREFIX "description-term"
#define TALKATU_TAG_DD            TALKATU_TAG_PREFIX "description-definition"
#define TALKATU_TAG_MESSAGE       TALKATU_TAG_PREFIX "message"
#define TALKATU_TAG_TIMESTAMP     TALKATU_TAG_PREFIX "timestamp"
#define TALKATU_TAG_AUTHOR        TALKATU_TAG_PREFIX "author"
#define TALKATU_TAG_CONTENTS      TALKATU_TAG_PREFIX "contents"

#define TALKATU_TAG_FORMATTING_START "-start"
#define TALKATU_TAG_FORMATTING_END   "-end"

typedef enum _TalkatuTagDisplay /*< prefix=TALKATU_TAG_DISPLAY,underscore_name=TALKATU_TAG_DISPLAY >*/
{
	TALKATU_TAG_DISPLAY_INLINE = 0,
	TALKATU_TAG_DISPLAY_BLOCK = 1,
} TalkatuTagDisplay;

G_BEGIN_DECLS

#define TALKATU_TYPE_TAG            (talkatu_tag_get_type())

G_DECLARE_FINAL_TYPE(TalkatuTag, talkatu_tag, TALKATU, TAG, GtkTextTag)

GtkTextTag *talkatu_tag_new(const gchar *name, const gchar *first_property, ...) G_GNUC_NULL_TERMINATED;

void talkatu_tag_set_display(TalkatuTag *tag, TalkatuTagDisplay display);
TalkatuTagDisplay talkatu_tag_get_display(TalkatuTag *tag);

const gchar *talkatu_tag_name_for_action_name(const gchar *action_name);
const gchar *talkatu_tag_name_to_html(const gchar *tag_name);

G_END_DECLS

#endif /* TALKATU_TAG_H */
