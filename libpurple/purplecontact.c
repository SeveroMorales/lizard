/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "purplecontact.h"

#include "purpleenums.h"
#include "util.h"

struct _PurpleContact {
	PurpleContactInfo parent;

	PurpleAccount *account;
};

enum {
	PROP_0,
	PROP_ACCOUNT,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PurpleContact, purple_contact, PURPLE_TYPE_CONTACT_INFO)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_contact_set_account(PurpleContact *contact, PurpleAccount *account) {
	g_return_if_fail(PURPLE_IS_CONTACT(contact));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	if(g_set_object(&contact->account, account)) {
		g_object_notify_by_pspec(G_OBJECT(contact), properties[PROP_ACCOUNT]);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_contact_get_property(GObject *obj, guint param_id, GValue *value,
                            GParamSpec *pspec)
{
	PurpleContact *contact = PURPLE_CONTACT(obj);

	switch(param_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value, purple_contact_get_account(contact));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_contact_set_property(GObject *obj, guint param_id, const GValue *value,
                            GParamSpec *pspec)
{
	PurpleContact *contact = PURPLE_CONTACT(obj);

	switch(param_id) {
		case PROP_ACCOUNT:
			purple_contact_set_account(contact, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_contact_dispose(GObject *obj) {
	PurpleContact *contact = PURPLE_CONTACT(obj);

	g_clear_object(&contact->account);

	G_OBJECT_CLASS(purple_contact_parent_class)->dispose(obj);
}

static void
purple_contact_init(G_GNUC_UNUSED PurpleContact *contact) {
}

static void
purple_contact_class_init(PurpleContactClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = purple_contact_dispose;
	obj_class->get_property = purple_contact_get_property;
	obj_class->set_property = purple_contact_set_property;

	/**
	 * PurpleContact:account:
	 *
	 * The account that this contact belongs to.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "account",
		"The account this contact belongs to",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleContact *
purple_contact_new(PurpleAccount *account, const gchar *id) {
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	return g_object_new(
		PURPLE_TYPE_CONTACT,
		"account", account,
		"id", id,
		NULL);
}

PurpleAccount *
purple_contact_get_account(PurpleContact *contact) {
	g_return_val_if_fail(PURPLE_IS_CONTACT(contact), NULL);

	return contact->account;
}
