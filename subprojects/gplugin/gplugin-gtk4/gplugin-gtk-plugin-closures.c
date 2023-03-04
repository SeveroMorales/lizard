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

#include <glib/gi18n-lib.h>

#include <gplugin.h>

#include <gplugin-gtk-plugin-closures.h>

/******************************************************************************
 * Private API
 *****************************************************************************/

gchar *
gplugin_gtk_lookup_plugin_name(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	GPluginPluginInfo *info,
	const gchar *filename,
	G_GNUC_UNUSED gpointer data)
{
	const gchar *name = NULL;
	gchar *basename = NULL;
	gchar *unnamed = NULL;

	name = gplugin_plugin_info_get_name(info);
	if(name != NULL) {
		return g_strdup(name);
	}

	/* Add a default name if unavailable. */
	basename = g_path_get_basename(filename);
	unnamed = g_strdup_printf(_("Unnamed Plugin: %s"), basename);
	g_free(basename);

	return unnamed;
}

gboolean
gplugin_gtk_lookup_plugin_state_sensitivity(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	GPluginPluginState state,
	G_GNUC_UNUSED gpointer data)
{
	gboolean result = FALSE;

	switch(state) {
		case GPLUGIN_PLUGIN_STATE_QUERIED:
		case GPLUGIN_PLUGIN_STATE_REQUERY:
		case GPLUGIN_PLUGIN_STATE_LOADED:
			result = TRUE;
			break;

		case GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED:
		case GPLUGIN_PLUGIN_STATE_ERROR:
		case GPLUGIN_PLUGIN_STATE_LOAD_FAILED:
		case GPLUGIN_PLUGIN_STATE_UNKNOWN:
		default:
			result = FALSE;
			break;
	}

	return result;
}

gboolean
gplugin_gtk_lookup_plugin_state(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	GPluginPluginState state,
	G_GNUC_UNUSED gpointer data)
{
	gboolean result = FALSE;

	switch(state) {
		case GPLUGIN_PLUGIN_STATE_LOADED:
		case GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED:
			result = TRUE;
			break;

		case GPLUGIN_PLUGIN_STATE_QUERIED:
		case GPLUGIN_PLUGIN_STATE_REQUERY:
		case GPLUGIN_PLUGIN_STATE_ERROR:
		case GPLUGIN_PLUGIN_STATE_LOAD_FAILED:
		case GPLUGIN_PLUGIN_STATE_UNKNOWN:
		default:
			result = FALSE;
			break;
	}

	return result;
}
