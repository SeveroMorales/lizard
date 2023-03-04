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

#include "haslmechanismexternal.h"

#include "haslcore.h"

struct _HaslMechanismExternal {
	HaslMechanism parent;
};

G_DEFINE_TYPE(HaslMechanismExternal, hasl_mechanism_external,
              HASL_TYPE_MECHANISM)

/******************************************************************************
 * HaslMechanism Implementation
 *****************************************************************************/
HaslMechanismResult
hasl_mechanism_external_step(G_GNUC_UNUSED HaslMechanism *mechanism,
                             HaslContext *ctx,
                             G_GNUC_UNUSED const guint8 *server_in,
                             G_GNUC_UNUSED gsize server_in_length,
                             guint8 **client_out,
                             gsize *client_out_length,
                             G_GNUC_UNUSED GError **error)
{
	const char *authzid = NULL;
	gsize length = 0;

	authzid = hasl_context_get_authzid(ctx);
	if(authzid != NULL && authzid[0] != '\0') {
		length = strlen(authzid);
		*client_out = (guint8 *)g_strdup(authzid);
	}

	if(client_out_length != NULL) {
		*client_out_length = length;
	}

	return HASL_MECHANISM_RESULT_SUCCESS;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
hasl_mechanism_external_init(G_GNUC_UNUSED HaslMechanismExternal *external) {
}

static void
hasl_mechanism_external_class_init(HaslMechanismExternalClass *klass) {
	HaslMechanismClass *mechanism_class = HASL_MECHANISM_CLASS(klass);

	mechanism_class->step = hasl_mechanism_external_step;
}
