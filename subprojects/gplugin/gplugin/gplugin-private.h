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
#ifndef GPLUGIN_PRIVATE_H
#define GPLUGIN_PRIVATE_H

#include <glib.h>
#include <glib-object.h>

/* this gets included by some tests so we need to trick the headers to accept
 * it.
 */
#define GPLUGIN_GLOBAL_HEADER_INSIDE
#include <gplugin/gplugin-manager.h>
#include <gplugin/gplugin-plugin-info.h>
#include <gplugin/gplugin-plugin.h>
#undef GPLUGIN_GLOBAL_HEADER_INSIDE

G_BEGIN_DECLS

void gplugin_manager_private_init(gboolean register_native_loader);
void gplugin_manager_private_uninit(void);

gboolean gplugin_boolean_accumulator(
	GSignalInvocationHint *hint,
	GValue *return_accu,
	const GValue *handler_return,
	gpointer data);

G_GNUC_INTERNAL
void gplugin_manager_add_plugin(
	GPluginManager *manager,
	const gchar *id,
	GPluginPlugin *plugin);

G_END_DECLS

#endif /* GPLUGIN_PRIVATE_H */
