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

#ifndef TALKATU_ATTACHMENT_PREVIEW_H
#define TALKATU_ATTACHMENT_PREVIEW_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <talkatu/talkatuattachment.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_ATTACHMENT_PREVIEW (talkatu_attachment_preview_get_type())
G_DECLARE_FINAL_TYPE(TalkatuAttachmentPreview, talkatu_attachment_preview, TALKATU, ATTACHMENT_PREVIEW, GtkWidget)

GtkWidget *talkatu_attachment_preview_new(TalkatuAttachment *attachment);

TalkatuAttachment *talkatu_attachment_preview_get_attachment(TalkatuAttachmentPreview *preview);

G_END_DECLS

#endif /* TALKATU_ATTACHMENT_PREVIEW_H */
