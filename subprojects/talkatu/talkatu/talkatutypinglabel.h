/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2022 Gary Kramlich <grim@reaperworld.com>
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

#ifndef TALKATU_TYPING_LABEL_H
#define TALKATU_TYPING_LABEL_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_TYPING_LABEL (talkatu_typing_label_get_type())
G_DECLARE_FINAL_TYPE(TalkatuTypingLabel, talkatu_typing_label, TALKATU, TYPING_LABEL, GtkWidget)

GtkWidget *talkatu_typing_label_new(void);

void talkatu_typing_label_start_typing(TalkatuTypingLabel *label, const gchar *who);
void talkatu_typing_label_finish_typing(TalkatuTypingLabel *label, const gchar *who);

G_END_DECLS

#endif /* TALKATU_TYPING_LABEL_H */

