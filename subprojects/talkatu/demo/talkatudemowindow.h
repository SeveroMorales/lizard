/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TALKATU_DEMO_WINDOW_H
#define TALKATU_DEMO_WINDOW_H

#include <glib.h>
#include <glib-object.h>

#include <talkatu/talkatu.h>

G_BEGIN_DECLS

#define TALKATU_DEMO_TYPE_WINDOW (talkatu_demo_window_get_type())

G_DECLARE_FINAL_TYPE(TalkatuDemoWindow, talkatu_demo_window, TALKATU_DEMO, WINDOW, GtkApplicationWindow)

GtkWidget *talkatu_demo_window_new(void);

G_END_DECLS

#endif /* TALKATU_DEMO_WINDOW_H */
