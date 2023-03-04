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

#include <gplugin/gplugin-private.h>

/* A GSignalAccumulator that stops emission if a handler returns FALSE */
gboolean
gplugin_boolean_accumulator(
	G_GNUC_UNUSED GSignalInvocationHint *hint,
	GValue *return_accu,
	const GValue *handler_return,
	G_GNUC_UNUSED gpointer data)
{
	gboolean continue_emission;
	gboolean handler_returned;

	handler_returned = g_value_get_boolean(handler_return);
	g_value_set_boolean(return_accu, handler_returned);
	continue_emission = handler_returned;

	return continue_emission;
}
