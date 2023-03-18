/*
 * pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_ATTACHMENT_H
#define PIDGIN_ATTACHMENT_H

#include <purple.h>

#include <talkatu.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_ATTACHMENT (pidgin_attachment_get_type())
G_DECLARE_FINAL_TYPE(PidginAttachment, pidgin_attachment, PIDGIN, ATTACHMENT, GObject)

/**
 * pidgin_attachment_new:
 * @attachment: The #PurpleAttachment to wrap.
 *
 * Wraps @attachment so that it can be used as a #TalkatuAttachment.
 *
 * Returns: (transfer full): The new #PidginAttachment instance.
 */
PidginAttachment *pidgin_attachment_new(PurpleAttachment *attachment);

/**
 * pidgin_attachment_get_attachment:
 * @attachment: The #PidginAttachment instance.
 *
 * Gets the #PurpleAttachment that @attachment is wrapping.
 *
 * Returns: (transfer none): The #PurpleAttachment that @attachment is wrapping.
 */
PurpleAttachment *pidgin_attachment_get_attachment(PidginAttachment *attachment);

G_END_DECLS

#endif /* PIDGIN_ATTACHMENT_H */
