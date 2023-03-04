/*
 * Copyright 2021 Elliott Sales de Andrade <quantum.analyst@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GPLUGIN_GTK_VIEWER_WINDOW_H
#define GPLUGIN_GTK_VIEWER_WINDOW_H

#include <gtk/gtk.h>

#include <gplugin-gtk.h>

G_BEGIN_DECLS

#define GPLUGIN_GTK_VIEWER_TYPE_WINDOW (gplugin_gtk_viewer_window_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginGtkViewerWindow,
	gplugin_gtk_viewer_window,
	GPLUGIN_GTK_VIEWER,
	WINDOW,
	GtkApplicationWindow)

GPluginGtkView *gplugin_gtk_viewer_window_get_view(
	GPluginGtkViewerWindow *window);

G_END_DECLS

#endif /* GPLUGIN_GTK_VIEWER_WINDOW_H */
