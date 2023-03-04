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

#include <gplugin-gtk.h>

#include "gplugin-gtk-viewer-window.h"

struct _GPluginGtkViewerWindow {
	GtkApplicationWindow parent;

	GPluginGtkView *view;
};

G_DEFINE_TYPE(
	GPluginGtkViewerWindow,
	gplugin_gtk_viewer_window,
	GTK_TYPE_APPLICATION_WINDOW)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/

static void
gplugin_gtk_viewer_window_class_init(GPluginGtkViewerWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/gplugin/gplugin/viewer/window.ui");

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkViewerWindow,
		view);

	gtk_widget_class_add_binding_action(
		widget_class,
		GDK_KEY_w,
		GDK_CONTROL_MASK,
		"window.close",
		NULL);
}

static void
gplugin_gtk_viewer_window_init(GPluginGtkViewerWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GPluginGtkView *
gplugin_gtk_viewer_window_get_view(GPluginGtkViewerWindow *window)
{
	return window->view;
}
