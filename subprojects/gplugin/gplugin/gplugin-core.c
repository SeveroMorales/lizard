/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
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

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-plugin.h>
#include <gplugin/gplugin-private.h>

/**
 * GPluginCoreFlags:
 * @GPLUGIN_CORE_FLAGS_NONE: No flags.
 * @GPLUGIN_CORE_FLAGS_DISABLE_NATIVE_LOADER: Disable the native plugin loader.
 * @GPLUGIN_CORE_FLAGS_LOG_PLUGIN_STATE_CHANGES: Log plugin state changes with
 *                                               g_message. Since: 0.34.0
 *
 * Flags to configure behaviors in GPlugin.
 *
 * Since: 0.31.0
 */

static GPluginCoreFlags core_flags = 0;

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * GPLUGIN_DOMAIN: (skip)
 *
 * The #GError domain used internally by GPlugin
 */

/**
 * GPLUGIN_GLOBAL_HEADER_INSIDE: (skip)
 *
 * This define is used to determine if we're inside the gplugin global header
 * file or not.
 */

/**
 * gplugin_init:
 * @flags: The core flags to set.
 *
 * Initializes the GPlugin library.
 *
 * This function *MUST* be called before interacting with any other GPlugin
 * API. The one exception is [func@GPlugin.get_option_group]. Parsing options
 * with the [struct@GLib.OptionGroup] from [func@GPlugin.get_option_group]
 * internally calls [func@GPlugin.init].
 */
void
gplugin_init(GPluginCoreFlags flags)
{
	gboolean register_native_loader = TRUE;

	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	core_flags = flags;

	if(flags & GPLUGIN_CORE_FLAGS_DISABLE_NATIVE_LOADER) {
		register_native_loader = FALSE;
	}

	gplugin_manager_private_init(register_native_loader);
}

/**
 * gplugin_uninit:
 *
 * Uninitializes the GPlugin library
 */
void
gplugin_uninit(void)
{
	gplugin_manager_private_uninit();
}

/**
 * gplugin_get_flags:
 *
 * Gets the core flags that were passed to [func@GPlugin.init].
 *
 * Returns: The core flags that GPlugin was initialized with.
 *
 * Since: 0.34.0
 */
GPluginCoreFlags
gplugin_get_flags(void)
{
	return core_flags;
}
