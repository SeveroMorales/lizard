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

#include <stdarg.h>

#include <gtk/gtk.h>
#include <pango/pango.h>

#include <talkatu/talkatutag.h>
#include <talkatu/talkatutagtable.h>

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
talkatu_tag_table_populate(GtkTextTagTable *tag_table) {
	GdkRGBA color = {0.0, 0.0, 0.0, 0.0};

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_BOLD,
			"weight", PANGO_WEIGHT_BOLD,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_ITALIC,
			"style", PANGO_STYLE_ITALIC,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_UNDERLINE,
			"underline", PANGO_UNDERLINE_SINGLE,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_STRIKETHROUGH,
			"strikethrough", TRUE,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_SUBSCRIPT,
			"rise", -5000,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_SUPERSCRIPT,
			"rise", 5000,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_PRE,
			"family", "Monospace",
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_CODE,
			"family", "Monospace",
			"paragraph-background", "#C0C0C0",
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_SEARCH,
			"background", "#22FF00",
			"weight", PANGO_WEIGHT_BOLD,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_H1,
			"weight", PANGO_WEIGHT_BOLD,
			"pixels-above-lines", 11,
			"pixels-below-lines", 11,
			"scale", 2.0,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_H2,
			"weight", PANGO_WEIGHT_BOLD,
			"scale", 1.5,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_H3,
			"weight", PANGO_WEIGHT_BOLD,
			"scale", 1.1699,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_H4,
			"weight", PANGO_WEIGHT_BOLD,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_H5,
			"weight", PANGO_WEIGHT_BOLD,
			"scale", 0.8299,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_H6,
			"weight", PANGO_WEIGHT_BOLD,
			"scale", 0.67,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gdk_rgba_parse(&color, "#0000FF");
	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_ANCHOR,
			"foreground-rgba", &color,
			"underline", PANGO_UNDERLINE_SINGLE,
			"underline-rgba", &color,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_DL,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			"pixels-inside-wrap", 12,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_DL TALKATU_TAG_FORMATTING_START,
			"pixels-above-lines", 12, /* needs to be dynamic the font's point size */
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_DL TALKATU_TAG_FORMATTING_END,
			"pixels-below-lines", 12, /* needs to be dynamic the font's point size */
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_DT,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	gtk_text_tag_table_add(
		tag_table,
		talkatu_tag_new(
			TALKATU_TAG_DD,
			"left-margin", 40,
			"display", TALKATU_TAG_DISPLAY_BLOCK,
			NULL
		)
	);

	/* these tags are just used as markers and don't provide any formatting */
	gtk_text_tag_table_add(
		tag_table,
		gtk_text_tag_new(TALKATU_TAG_MESSAGE)
	);

	gtk_text_tag_table_add(
		tag_table,
		gtk_text_tag_new(TALKATU_TAG_TIMESTAMP)
	);

	gtk_text_tag_table_add(
		tag_table,
		gtk_text_tag_new(TALKATU_TAG_AUTHOR)
	);

	gtk_text_tag_table_add(
		tag_table,
		gtk_text_tag_new(TALKATU_TAG_CONTENTS)
	);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_tag_table_new:
 *
 * Creates a new [class@Gtk.TextTagTable] that is populated with all of the
 * tags that Talkatu uses.
 *
 * Returns: (transfer full): The new instance.
 */
GtkTextTagTable *talkatu_tag_table_new(void) {
	GtkTextTagTable *table = gtk_text_tag_table_new();

	talkatu_tag_table_populate(table);

	return table;
}
