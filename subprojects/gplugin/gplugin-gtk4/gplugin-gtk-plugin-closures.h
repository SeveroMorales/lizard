/*
 * Copyright (C) 2022 Elliott Sales de Andrade <quantum.analyst@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GPLUGIN_GTK_GLOBAL_HEADER_INSIDE) && \
	!defined(GPLUGIN_GTK_COMPILATION)
#error "only <gplugin-gtk.h> may be included directly"
#endif

#ifndef GPLUGIN_GTK_PLUGIN_CLOSURES_H
#define GPLUGIN_GTK_PLUGIN_CLOSURES_H

#include <glib.h>

#include <gtk/gtk.h>

#include <gplugin.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
gchar *gplugin_gtk_lookup_plugin_name(
	GtkClosureExpression *expression,
	GPluginPluginInfo *info,
	const gchar *filename,
	gpointer data);

G_GNUC_INTERNAL
gboolean gplugin_gtk_lookup_plugin_state_sensitivity(
	GtkClosureExpression *expression,
	GPluginPluginState state,
	gpointer data);

G_GNUC_INTERNAL
gboolean gplugin_gtk_lookup_plugin_state(
	GtkClosureExpression *expression,
	GPluginPluginState state,
	gpointer data);

G_END_DECLS

#endif /* GPLUGIN_GTK_PLUGIN_CLOSURES_H */
