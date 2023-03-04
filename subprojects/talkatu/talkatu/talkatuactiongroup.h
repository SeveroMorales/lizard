/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2023 Gary Kramlich <grim@reaperworld.com>
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

#ifndef TALKATU_ACTION_GROUP_H
#define TALKATU_ACTION_GROUP_H

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <talkatu/talkatuformat.h>
#include <talkatu/talkatuinput.h>

#define TALKATU_ACTION_FORMAT_BOLD ("format-bold")
#define TALKATU_ACTION_FORMAT_ITALIC ("format-italic")
#define TALKATU_ACTION_FORMAT_UNDERLINE ("format-underline")
#define TALKATU_ACTION_FORMAT_STRIKETHROUGH ("format-strikethrough")
#define TALKATU_ACTION_FORMAT_GROW ("format-grow")
#define TALKATU_ACTION_FORMAT_SHRINK ("format-shrink")
#define TALKATU_ACTION_FORMAT_RESET ("format-reset")

#define TALKATU_ACTION_ATTACH_FILE ("attach-file")
#define TALKATU_ACTION_ATTACH_IMAGE ("attach-image")

#define TALKATU_ACTION_INSERT_LINK ("insert-link")
#define TALKATU_ACTION_INSERT_CODE ("insert-code")

G_BEGIN_DECLS

#define TALKATU_TYPE_ACTION_GROUP           (talkatu_action_group_get_type())

G_DECLARE_DERIVABLE_TYPE(TalkatuActionGroup, talkatu_action_group, TALKATU, ACTION_GROUP, GSimpleActionGroup)

struct _TalkatuActionGroupClass {
	/*< private >*/
	GSimpleActionGroupClass parent;

	/*< public >*/
	void (*action_activated)(TalkatuActionGroup *ag, GAction *action, const gchar *name);

	/*< private >*/
	gpointer reserved[4];
};

GSimpleActionGroup *talkatu_action_group_new(TalkatuFormat format);
GSimpleActionGroup *talkatu_action_group_new_with_buffer(TalkatuFormat format, GtkTextBuffer *buffer);

GtkTextBuffer *talkatu_action_group_get_buffer(TalkatuActionGroup *ag);
void talkatu_action_group_set_buffer(TalkatuActionGroup *ag, GtkTextBuffer *buffer);

void talkatu_action_group_set_input(TalkatuActionGroup *ag, TalkatuInput *input);
TalkatuInput *talkatu_action_group_get_input(TalkatuActionGroup *ag);

const gchar *talkatu_action_name_for_tag_name(const gchar *tag_name);

void talkatu_action_group_activate_format(TalkatuActionGroup *ag, const gchar *format_name);
gboolean talkatu_action_group_get_action_activated(TalkatuActionGroup *ag, const gchar *name);
gchar **talkatu_action_group_get_activated_formats(TalkatuActionGroup *ag);

TalkatuFormat talkatu_action_group_get_format(TalkatuActionGroup *ag);
void talkatu_action_group_set_format(TalkatuActionGroup *ag, TalkatuFormat format);

G_END_DECLS

#endif /* TALKATU_ACTIONGROUP_H */
