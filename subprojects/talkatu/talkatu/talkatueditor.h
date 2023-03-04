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

#ifndef TALKATU_EDITOR_H
#define TALKATU_EDITOR_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_EDITOR (talkatu_editor_get_type())
G_DECLARE_FINAL_TYPE(TalkatuEditor, talkatu_editor, TALKATU, EDITOR, GtkBox)

GtkWidget *talkatu_editor_new(void);

GtkWidget *talkatu_editor_get_toolbar(TalkatuEditor *editor);
void talkatu_editor_set_toolbar_visible(TalkatuEditor *editor, gboolean visible);
gboolean talkatu_editor_get_toolbar_visible(TalkatuEditor *editor);

GtkWidget *talkatu_editor_get_input(TalkatuEditor *editor);

GtkTextBuffer *talkatu_editor_get_buffer(TalkatuEditor *editor);
void talkatu_editor_set_buffer(TalkatuEditor *editor, GtkTextBuffer *buffer);

gboolean talkatu_editor_get_send_button_visible(TalkatuEditor *editor);
void talkatu_editor_set_send_button_visible(TalkatuEditor *editor, gboolean visible);

G_END_DECLS

#endif /* TALKATU_EDITOR_H */
