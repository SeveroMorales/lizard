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

#include "purpleperson.h"

#include "util.h"

#include "util.h"

struct _PurplePerson {
	GObject parent;

	gchar *id;

	gchar *alias;
	GdkPixbuf *avatar;
	PurpleTags *tags;

	GPtrArray *contacts;
};

enum {
	PROP_0,
	PROP_ID,
	PROP_ALIAS,
	PROP_AVATAR,
	PROP_AVATAR_FOR_DISPLAY,
	PROP_TAGS,
	PROP_NAME_FOR_DISPLAY,
	PROP_PRIORITY_CONTACT_INFO,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_person_set_id(PurplePerson *person, const gchar *id) {
	g_return_if_fail(PURPLE_IS_PERSON(person));

	g_free(person->id);

	if(id != NULL) {
		person->id = g_strdup(id);
	} else {
		person->id = g_uuid_string_random();
	}

	g_object_notify_by_pspec(G_OBJECT(person), properties[PROP_ID]);
}

static gint
purple_person_contact_compare(gconstpointer a, gconstpointer b) {
	PurpleContactInfo *c1 = *(PurpleContactInfo **)a;
	PurpleContactInfo *c2 = *(PurpleContactInfo **)b;
	PurplePresence *p1 = NULL;
	PurplePresence *p2 = NULL;

	p1 = purple_contact_info_get_presence(c1);
	p2 = purple_contact_info_get_presence(c2);

	return purple_presence_compare(p1, p2);
}

static void
purple_person_sort_contacts(PurplePerson *person) {
	PurpleContactInfo *original_priority = NULL;
	PurpleContactInfo *new_priority = NULL;
	guint n_items = person->contacts->len;

	if(n_items <= 1) {
		GObject *obj = G_OBJECT(person);

		g_object_freeze_notify(obj);
		g_object_notify_by_pspec(obj, properties[PROP_NAME_FOR_DISPLAY]);
		g_object_notify_by_pspec(obj, properties[PROP_PRIORITY_CONTACT_INFO]);
		g_object_thaw_notify(obj);

		g_list_model_items_changed(G_LIST_MODEL(person), 0, n_items, n_items);

		return;
	}

	original_priority = purple_person_get_priority_contact_info(person);

	g_ptr_array_sort(person->contacts, purple_person_contact_compare);

	/* Tell the list we update our stuff. */
	g_list_model_items_changed(G_LIST_MODEL(person), 0, n_items, n_items);

	/* See if the priority contact changed. */
	new_priority = g_ptr_array_index(person->contacts, 0);
	if(original_priority != new_priority) {
		GObject *obj = G_OBJECT(person);

		g_object_freeze_notify(obj);
		g_object_notify_by_pspec(obj, properties[PROP_NAME_FOR_DISPLAY]);
		g_object_notify_by_pspec(obj, properties[PROP_PRIORITY_CONTACT_INFO]);

		/* If the person doesn't have an avatar set, notify that the
		 * avatar-for-display has changed.
		 */
		if(!GDK_IS_PIXBUF(person->avatar)) {
			g_object_notify_by_pspec(obj, properties[PROP_AVATAR_FOR_DISPLAY]);
		}

		g_object_thaw_notify(obj);
	}
}

/* This function is used by purple_person_matches to determine if a contact info
 * matches the needle.
 */
static gboolean
purple_person_matches_find_func(gconstpointer a, gconstpointer b) {
	PurpleContactInfo *info = (gpointer)a;
	const char *needle = b;

	return purple_contact_info_matches(info, needle);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_person_presence_notify_cb(G_GNUC_UNUSED GObject *obj,
                                 G_GNUC_UNUSED GParamSpec *pspec,
                                 gpointer data)
{
	purple_person_sort_contacts(data);
}

/******************************************************************************
 * GListModel Implementation
 *****************************************************************************/
static GType
purple_person_get_item_type(G_GNUC_UNUSED GListModel *list) {
	return PURPLE_TYPE_CONTACT_INFO;
}

static guint
purple_person_get_n_items(GListModel *list) {
	PurplePerson *person = PURPLE_PERSON(list);

	return person->contacts->len;
}

static gpointer
purple_person_get_item(GListModel *list, guint position) {
	PurplePerson *person = PURPLE_PERSON(list);
	PurpleContactInfo *info = NULL;

	if(position < person->contacts->len) {
		info = g_ptr_array_index(person->contacts, position);
		g_object_ref(info);
	}

	return info;
}

static void
purple_person_list_model_init(GListModelInterface *iface) {
	iface->get_item_type = purple_person_get_item_type;
	iface->get_n_items = purple_person_get_n_items;
	iface->get_item = purple_person_get_item;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(PurplePerson, purple_person, G_TYPE_OBJECT, 0,
                       G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL,
                                             purple_person_list_model_init))

static void
purple_person_get_property(GObject *obj, guint param_id, GValue *value,
                           GParamSpec *pspec)
{
	PurplePerson *person = PURPLE_PERSON(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_person_get_id(person));
			break;
		case PROP_ALIAS:
			g_value_set_string(value, purple_person_get_alias(person));
			break;
		case PROP_AVATAR:
			g_value_set_object(value, purple_person_get_avatar(person));
			break;
		case PROP_AVATAR_FOR_DISPLAY:
			g_value_set_object(value, purple_person_get_avatar_for_display(person));
			break;
		case PROP_TAGS:
			g_value_set_object(value, purple_person_get_tags(person));
			break;
		case PROP_NAME_FOR_DISPLAY:
			g_value_set_string(value,
			                   purple_person_get_name_for_display(person));
			break;
		case PROP_PRIORITY_CONTACT_INFO:
			g_value_set_object(value,
			                   purple_person_get_priority_contact_info(person));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_person_set_property(GObject *obj, guint param_id, const GValue *value,
                            GParamSpec *pspec)
{
	PurplePerson *person = PURPLE_PERSON(obj);

	switch(param_id) {
		case PROP_ID:
			purple_person_set_id(person, g_value_get_string(value));
			break;
		case PROP_ALIAS:
			purple_person_set_alias(person, g_value_get_string(value));
			break;
		case PROP_AVATAR:
			purple_person_set_avatar(person, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_person_dispose(GObject *obj) {
	PurplePerson *person = PURPLE_PERSON(obj);

	g_clear_object(&person->avatar);
	g_clear_object(&person->tags);

	if(person->contacts != NULL) {
		g_ptr_array_free(person->contacts, TRUE);
		person->contacts = NULL;
	}

	G_OBJECT_CLASS(purple_person_parent_class)->dispose(obj);
}

static void
purple_person_finalize(GObject *obj) {
	PurplePerson *person = PURPLE_PERSON(obj);

	g_clear_pointer(&person->id, g_free);
	g_clear_pointer(&person->alias, g_free);

	G_OBJECT_CLASS(purple_person_parent_class)->finalize(obj);
}

static void
purple_person_constructed(GObject *obj) {
	PurplePerson *person = NULL;

	G_OBJECT_CLASS(purple_person_parent_class)->constructed(obj);

	person = PURPLE_PERSON(obj);
	if(person->id == NULL) {
		purple_person_set_id(person, NULL);
	}
}

static void
purple_person_init(PurplePerson *person) {
	person->tags = purple_tags_new();
	person->contacts = g_ptr_array_new_full(0, (GDestroyNotify)g_object_unref);
}

static void
purple_person_class_init(PurplePersonClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_person_get_property;
	obj_class->set_property = purple_person_set_property;
	obj_class->constructed = purple_person_constructed;
	obj_class->dispose = purple_person_dispose;
	obj_class->finalize = purple_person_finalize;

	/**
	 * PurplePerson:id:
	 *
	 * The protocol specific id for the contact.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The id of this contact",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePerson:alias:
	 *
	 * The alias for this person. This is controlled by the libpurple user.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ALIAS] = g_param_spec_string(
		"alias", "alias",
		"The alias of this person.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePerson:avatar:
	 *
	 * The avatar for this person. This is controlled by the libpurple user,
	 * which they can use to set a custom avatar.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_AVATAR] = g_param_spec_object(
		"avatar", "avatar",
		"The avatar of this person",
		GDK_TYPE_PIXBUF,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePerson:avatar-for-display
	 *
	 * The avatar to show for the person. If [property@Purple.Person:avatar] is
	 * set, it will be returned. Otherwise the value of
	 * [property@Purple.ContactInfo:avatar] for
	 * [property@Purple.Person:priority-contact-info] will be returned.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_AVATAR_FOR_DISPLAY] = g_param_spec_object(
		"avatar-for-display", "avatar-for-display",
		"The avatar to display for this person",
		GDK_TYPE_PIXBUF,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePerson:tags:
	 *
	 * The [class@Purple.Tags] for this person.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TAGS] = g_param_spec_object(
		"tags", "tags",
		"The tags for this person",
		PURPLE_TYPE_TAGS,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePerson:name-for-display:
	 *
	 * The name that should be displayed for this person.
	 *
	 * If [property@Purple.Person:alias] is set that will be returned. If not
	 * the value of [method@Purple.ContactInfo.get_name_for_display] for
	 * [property@Purple.Person:priority-contact-info] will be used. If
	 * [property@Purple.Person:priority-contact-info] is %NULL, then %NULL will
	 * be returned.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME_FOR_DISPLAY] = g_param_spec_string(
		"name-for-display", "name-for-display",
		"The name that should be displayed for the person",
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePerson:priority-contact-info:
	 *
	 * The [class@Purple.ContactInfo] that currently has the highest priority.
	 *
	 * This is used by user interfaces to determine which
	 * [class@Purple.ContactInfo] to use when messaging and so on.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PRIORITY_CONTACT_INFO] = g_param_spec_object(
		"priority-contact-info", "priority-contact-info",
		"The priority contact info for the person",
		PURPLE_TYPE_CONTACT_INFO,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurplePerson *
purple_person_new(void) {
	return g_object_new(PURPLE_TYPE_PERSON, NULL);
}

const gchar *
purple_person_get_id(PurplePerson *person) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	return person->id;
}

const gchar *
purple_person_get_alias(PurplePerson *person) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	return person->alias;
}

void
purple_person_set_alias(PurplePerson *person, const gchar *alias) {
	g_return_if_fail(PURPLE_IS_PERSON(person));

	if(!purple_strequal(person->alias, alias)) {
		GObject *obj = G_OBJECT(person);

		g_free(person->alias);
		person->alias = g_strdup(alias);

		g_object_freeze_notify(obj);
		g_object_notify_by_pspec(obj, properties[PROP_ALIAS]);
		g_object_notify_by_pspec(obj, properties[PROP_NAME_FOR_DISPLAY]);
		g_object_thaw_notify(obj);
	}
}

GdkPixbuf *
purple_person_get_avatar_for_display(PurplePerson *person) {
	PurpleContactInfo *priority = NULL;

	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	if(GDK_IS_PIXBUF(person->avatar)) {
		return person->avatar;
	}

	priority = purple_person_get_priority_contact_info(person);
	if(PURPLE_IS_CONTACT_INFO(priority)) {
		return purple_contact_info_get_avatar(priority);
	}

	return NULL;
}

GdkPixbuf *
purple_person_get_avatar(PurplePerson *person) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	return person->avatar;
}

void
purple_person_set_avatar(PurplePerson *person, GdkPixbuf *avatar) {
	g_return_if_fail(PURPLE_IS_PERSON(person));

	if(g_set_object(&person->avatar, avatar)) {
		GObject *obj = G_OBJECT(person);

		g_object_freeze_notify(obj);
		g_object_notify_by_pspec(obj, properties[PROP_AVATAR]);
		g_object_notify_by_pspec(obj, properties[PROP_AVATAR_FOR_DISPLAY]);
		g_object_thaw_notify(obj);
	}
}

PurpleTags *
purple_person_get_tags(PurplePerson *person) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	return person->tags;
}

const char *
purple_person_get_name_for_display(PurplePerson *person) {
	PurpleContactInfo *priority = NULL;

	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	if(!purple_strempty(person->alias)) {
		return person->alias;
	}

	priority = purple_person_get_priority_contact_info(person);
	if(PURPLE_IS_CONTACT_INFO(priority)) {
		return purple_contact_info_get_name_for_display(priority);
	}

	return NULL;
}

void
purple_person_add_contact_info(PurplePerson *person,
                               PurpleContactInfo *info)
{
	PurplePresence *presence = NULL;

	g_return_if_fail(PURPLE_IS_PERSON(person));
	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	g_ptr_array_add(person->contacts, g_object_ref(info));

	presence = purple_contact_info_get_presence(info);
	g_signal_connect_object(presence, "notify",
	                        G_CALLBACK(purple_person_presence_notify_cb),
	                        person, 0);

	purple_contact_info_set_person(info, person);

	purple_person_sort_contacts(person);
}

gboolean
purple_person_remove_contact_info(PurplePerson *person,
                                  PurpleContactInfo *info)
{
	gboolean removed = FALSE;

	g_return_val_if_fail(PURPLE_IS_PERSON(person), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), FALSE);

	/* Ref the contact info to avoid a use-after free. */
	g_object_ref(info);

	/* g_ptr_array_remove calls g_object_unref because we passed it in as a
	 * GDestroyNotify.
	 */
	removed = g_ptr_array_remove(person->contacts, info);

	if(removed) {
		PurplePresence *presence = purple_contact_info_get_presence(info);

		g_signal_handlers_disconnect_by_func(presence,
		                                     purple_person_presence_notify_cb,
		                                     person);

		purple_contact_info_set_person(info, NULL);

		purple_person_sort_contacts(person);
	}

	/* Remove our reference. */
	g_object_unref(info);

	return removed;
}

PurpleContactInfo *
purple_person_get_priority_contact_info(PurplePerson *person) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), NULL);

	if(person->contacts->len == 0) {
		return NULL;
	}

	return g_ptr_array_index(person->contacts, 0);
}

gboolean
purple_person_has_contacts(PurplePerson *person) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), FALSE);

	return person->contacts->len > 0;
}

gboolean
purple_person_matches(PurplePerson *person, const char *needle) {
	g_return_val_if_fail(PURPLE_IS_PERSON(person), FALSE);

	if(purple_strempty(needle)) {
		return TRUE;
	}

	/* Check if the person's alias matches. */
	if(!purple_strempty(person->alias)) {
		if(purple_strmatches(needle, person->alias)) {
			return TRUE;
		}
	}

	/* See if any of the contact infos match. */
	return g_ptr_array_find_with_equal_func(person->contacts, needle,
	                                        purple_person_matches_find_func,
	                                        NULL);
}
