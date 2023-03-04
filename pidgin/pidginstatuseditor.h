/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifndef PIDGIN_STATUS_EDITOR_H
#define PIDGIN_STATUS_EDITOR_H

#include <gtk/gtk.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginStatusEditor:
 *
 * A dialog for editing statuses.
 */

#define PIDGIN_TYPE_STATUS_EDITOR (pidgin_status_editor_get_type())
G_DECLARE_FINAL_TYPE(PidginStatusEditor, pidgin_status_editor, PIDGIN,
                     STATUS_EDITOR, GtkDialog)

/**
 * pidgin_status_editor_new:
 * @status: (transfer none) (nullable): The [type@Purple.SavedStatus] to edit.
 *
 * Creates a new instance of the dialog. You can pass %NULL for @status to
 * create a new status with the dialog.
 *
 * Returns: (transfer full): The new #PidginStatusEditor instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_status_editor_new(PurpleSavedStatus *status);

/**
 * pidgin_status_editor_get_status:
 * @editor: The editor instance.
 *
 * Gets the [type@Purple.SavedStatus] that @editor was created for. This can
 * be %NULL if a new status is being created.
 *
 * Returns: (transfer none): The status.
 *
 * Since: 3.0.0
 */
PurpleSavedStatus *pidgin_status_editor_get_status(PidginStatusEditor *editor);

G_END_DECLS

#endif /* PIDGIN_STATUS_EDITOR_H */
