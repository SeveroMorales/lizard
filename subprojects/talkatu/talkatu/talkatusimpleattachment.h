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

#ifndef TALKATU_SIMPLE_ATTACHMENT_H
#define TALKATU_SIMPLE_ATTACHMENT_H

#include <talkatu/talkatuattachment.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_SIMPLE_ATTACHMENT (talkatu_simple_attachment_get_type())
G_DECLARE_FINAL_TYPE(TalkatuSimpleAttachment, talkatu_simple_attachment, TALKATU, SIMPLE_ATTACHMENT, GObject)

TalkatuAttachment *talkatu_simple_attachment_new(guint64 id, const gchar *content_type);

G_END_DECLS

#endif /* TALKATU_SIMPLE_ATTACHMENT_H */
