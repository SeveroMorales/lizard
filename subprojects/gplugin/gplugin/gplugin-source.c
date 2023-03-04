/*
 * Copyright (C) 2011-2022 Gary Kramlich <grim@reaperworld.com>
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

#include "gplugin-source.h"

/**
 * GPluginSource:
 *
 * An interface that the manager will call during refresh to query plugins.
 *
 * Since: 0.39.0
 */

G_DEFINE_INTERFACE(GPluginSource, gplugin_source, G_TYPE_OBJECT)

/******************************************************************************
 * GInterface implementation
 *****************************************************************************/
static void
gplugin_source_default_init(G_GNUC_UNUSED GPluginSourceInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * gplugin_source_scan:
 * @source: The instance.
 *
 * This method is called when [method@GPlugin.Manager.refresh] is running. The
 * source should scan its available sources for plugins. For the filesystem
 * source, this is paths that have been registered with the manager.
 *
 * The implementation should return TRUE if it found a new unqueried plugin,
 * which will tell the manager to continue scanning.
 *
 * Returns: %TRUE if an unqueried plugin was found, %FALSE otherwise.
 *
 * Since: 0.39.0
 */
gboolean
gplugin_source_scan(GPluginSource *source)
{
	GPluginSourceInterface *iface = NULL;

	g_return_val_if_fail(GPLUGIN_IS_SOURCE(source), FALSE);

	iface = GPLUGIN_SOURCE_GET_IFACE(source);
	if(iface && iface->scan) {
		return iface->scan(source);
	}

	return FALSE;
}
