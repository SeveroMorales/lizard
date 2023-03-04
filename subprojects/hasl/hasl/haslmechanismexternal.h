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

#ifndef HASL_MECHANISM_EXTERNAL_H
#define HASL_MECHANISM_EXTERNAL_H

#include <glib.h>
#include <glib-object.h>

#include <hasl/haslcontext.h>
#include <hasl/haslmechanism.h>

G_BEGIN_DECLS

#define HASL_TYPE_MECHANISM_EXTERNAL (hasl_mechanism_external_get_type())
G_DECLARE_FINAL_TYPE(HaslMechanismExternal, hasl_mechanism_external, HASL,
                     MECHANISM_EXTERNAL, HaslMechanism)

G_END_DECLS

#endif /* HASL_MECHANISM_EXTERNAL_H */

