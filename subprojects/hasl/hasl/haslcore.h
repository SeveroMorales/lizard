/*
 * Copyright (C) 2023 Hasl Developers
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

#ifndef HASL_CORE_H
#define HASL_CORE_H

#include <glib.h>

/**
 * HASL_DOMAIN: (skip)
 *
 * The #GError domain used internally by Hasl
 */
#define HASL_DOMAIN (g_quark_from_static_string("hasl"))

#endif /* HASL_CORE_H */
