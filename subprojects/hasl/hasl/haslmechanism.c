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

#include "haslmechanism.h"

G_DEFINE_TYPE(HaslMechanism, hasl_mechanism, G_TYPE_OBJECT)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
hasl_mechanism_init(G_GNUC_UNUSED HaslMechanism *mechanism) {
}

static void
hasl_mechanism_class_init(G_GNUC_UNUSED HaslMechanismClass *klass) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/
gboolean
hasl_mechanism_possible(HaslMechanism *mechanism, HaslContext *ctx,
                        GError **error)
{
	HaslMechanismClass *klass = NULL;

	g_return_val_if_fail(HASL_IS_MECHANISM(mechanism), FALSE);
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), FALSE);
	g_return_val_if_fail(error != NULL && *error == NULL, FALSE);

	klass = HASL_MECHANISM_GET_CLASS(mechanism);
	if(klass != NULL && klass->possible != NULL) {
		return klass->possible(mechanism, ctx, error);
	}

	return TRUE;
}

HaslMechanismResult
hasl_mechanism_step(HaslMechanism *mechanism, HaslContext *ctx,
                    const guint8 *server_in, gsize server_in_length,
                    guint8 **client_out, gsize *client_out_length,
                    GError **error)
{
	HaslMechanismClass *klass = NULL;

	g_return_val_if_fail(HASL_IS_MECHANISM(mechanism),
	                     HASL_MECHANISM_RESULT_ERROR);
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), HASL_MECHANISM_RESULT_ERROR);
	g_return_val_if_fail(client_out != NULL, HASL_MECHANISM_RESULT_ERROR);
	g_return_val_if_fail(error != NULL && *error == NULL,
	                     HASL_MECHANISM_RESULT_ERROR);

	klass = HASL_MECHANISM_GET_CLASS(mechanism);
	if(klass != NULL && klass->step != NULL) {
		return klass->step(mechanism, ctx, server_in, server_in_length,
		                   client_out, client_out_length, error);
	}

	return HASL_MECHANISM_RESULT_ERROR;
}
