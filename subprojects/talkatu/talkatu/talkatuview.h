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

#ifndef TALKATU_VIEW_H
#define TALKATU_VIEW_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_VIEW (talkatu_view_get_type())
G_DECLARE_DERIVABLE_TYPE(TalkatuView, talkatu_view, TALKATU, VIEW, GtkTextView)

struct _TalkatuViewClass {
	/*< private >*/
	GtkTextViewClass parent;

	/*< public >*/
	void (*format_activate)(TalkatuView *view, const gchar *action_name);

	void (*open_url)(TalkatuView *view, const gchar *url);

	/*< private >*/
	gpointer reserved[4];
};

GtkWidget *talkatu_view_new(void);
GtkWidget *talkatu_view_new_with_buffer(GtkTextBuffer *buffer);

G_END_DECLS

#endif /* TALKATU_VIEW_H */
