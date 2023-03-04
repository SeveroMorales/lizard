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

#ifndef HASL_MECHANISM_PLAIN_H
#define HASL_MECHANISM_PLAIN_H

#include <glib.h>
#include <glib-object.h>

#include <hasl/haslcontext.h>
#include <hasl/haslmechanism.h>

G_BEGIN_DECLS

#define HASL_MECHANISM_PLAIN_DOMAIN (g_quark_from_static_string("hasl-mechanism-plain"))

#define HASL_TYPE_MECHANISM_PLAIN (hasl_mechanism_plain_get_type())
G_DECLARE_FINAL_TYPE(HaslMechanismPlain, hasl_mechanism_plain, HASL,
                     MECHANISM_PLAIN, HaslMechanism)

G_END_DECLS

#endif /* HASL_MECHANISM_PLAIN_H */
