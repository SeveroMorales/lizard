/*
 * purple - Jabber Protocol Plugin
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "caps.h"
#include "iq.h"
#include "presence.h"
#include "xdata.h"

#define JABBER_CAPS_FILENAME "xmpp-caps.xml"

typedef struct {
	gchar *var;
	GList *values;
} JabberDataFormField;

static GHashTable *capstable = NULL; /* JabberCapsTuple -> JabberCapsClientInfo */
static guint       save_timer = 0;

static guint jabber_caps_hash(gconstpointer data) {
	const JabberCapsTuple *key = data;
	guint nodehash = g_str_hash(key->node);
	guint verhash  = g_str_hash(key->ver);
	guint hashhash = g_str_hash(key->hash);
	return nodehash ^ verhash ^ hashhash;
}

static gboolean jabber_caps_compare(gconstpointer v1, gconstpointer v2) {
	const JabberCapsTuple *name1 = v1;
	const JabberCapsTuple *name2 = v2;

	return purple_strequal(name1->node, name2->node) &&
	       purple_strequal(name1->ver, name2->ver) &&
	       purple_strequal(name1->hash, name2->hash);
}

void
jabber_caps_client_info_destroy(JabberCapsClientInfo *info)
{
	if (info == NULL)
		return;

	g_list_free_full(info->identities, (GDestroyNotify)jabber_identity_free);

	g_list_free_full(info->features, g_free);

	g_list_free_full(info->forms, (GDestroyNotify)purple_xmlnode_free);

	g_free((char *)info->tuple.node);
	g_free((char *)info->tuple.ver);
	g_free((char *)info->tuple.hash);

	g_free(info);
}

static void jabber_caps_store_client(gpointer key, gpointer value, gpointer user_data) {
	const JabberCapsTuple *tuple = key;
	const JabberCapsClientInfo *props = value;
	PurpleXmlNode *root = user_data;
	PurpleXmlNode *client = purple_xmlnode_new_child(root, "client");
	GList *iter;

	purple_xmlnode_set_attrib(client, "node", tuple->node);
	purple_xmlnode_set_attrib(client, "ver", tuple->ver);
	purple_xmlnode_set_attrib(client, "hash", tuple->hash);
	for(iter = props->identities; iter; iter = g_list_next(iter)) {
		JabberIdentity *id = iter->data;
		PurpleXmlNode *identity = purple_xmlnode_new_child(client, "identity");
		purple_xmlnode_set_attrib(identity, "category", id->category);
		purple_xmlnode_set_attrib(identity, "type", id->type);
		if (id->name)
			purple_xmlnode_set_attrib(identity, "name", id->name);
		if (id->lang)
			purple_xmlnode_set_attrib(identity, "lang", id->lang);
	}

	for(iter = props->features; iter; iter = g_list_next(iter)) {
		const char *feat = iter->data;
		PurpleXmlNode *feature = purple_xmlnode_new_child(client, "feature");
		purple_xmlnode_set_attrib(feature, "var", feat);
	}

	for(iter = props->forms; iter; iter = g_list_next(iter)) {
		/* FIXME: See #7814 */
		PurpleXmlNode *xdata = iter->data;
		purple_xmlnode_insert_child(client, purple_xmlnode_copy(xdata));
	}
}

static gboolean
do_jabber_caps_store(G_GNUC_UNUSED gpointer data)
{
	char *str;
	int length = 0;
	PurpleXmlNode *root = purple_xmlnode_new("capabilities");

	g_hash_table_foreach(capstable, jabber_caps_store_client, root);
	str = purple_xmlnode_to_formatted_str(root, &length);
	purple_xmlnode_free(root);
	purple_util_write_data_to_cache_file(JABBER_CAPS_FILENAME, str, length);
	g_free(str);

	save_timer = 0;
	return FALSE;
}

static void
schedule_caps_save(void)
{
	if (save_timer == 0)
		save_timer = g_timeout_add_seconds(5, do_jabber_caps_store, NULL);
}

static void
jabber_caps_load(void)
{
	PurpleXmlNode *capsdata = purple_util_read_xml_from_cache_file(JABBER_CAPS_FILENAME, "XMPP capabilities cache");
	PurpleXmlNode *client;

	if(!capsdata)
		return;

	if (!purple_strequal(capsdata->name, "capabilities")) {
		purple_xmlnode_free(capsdata);
		return;
	}

	for (client = capsdata->child; client; client = client->next) {
		if (client->type != PURPLE_XMLNODE_TYPE_TAG)
			continue;
		if (purple_strequal(client->name, "client")) {
			JabberCapsClientInfo *value = g_new0(JabberCapsClientInfo, 1);
			JabberCapsTuple *key = (JabberCapsTuple*)&value->tuple;
			PurpleXmlNode *child;
			key->node = g_strdup(purple_xmlnode_get_attrib(client,"node"));
			key->ver  = g_strdup(purple_xmlnode_get_attrib(client,"ver"));
			key->hash = g_strdup(purple_xmlnode_get_attrib(client,"hash"));

			for (child = client->child; child; child = child->next) {
				if (child->type != PURPLE_XMLNODE_TYPE_TAG)
					continue;
				if (purple_strequal(child->name, "feature")) {
					const char *var = purple_xmlnode_get_attrib(child, "var");
					if(!var)
						continue;
					value->features = g_list_append(value->features,g_strdup(var));
				} else if (purple_strequal(child->name, "identity")) {
					const char *category = purple_xmlnode_get_attrib(child, "category");
					const char *type = purple_xmlnode_get_attrib(child, "type");
					const char *name = purple_xmlnode_get_attrib(child, "name");
					const char *lang = purple_xmlnode_get_attrib(child, "lang");
					JabberIdentity *id;

					if (!category || !type)
						continue;

					id = jabber_identity_new(category, type, lang, name);
					value->identities = g_list_append(value->identities,id);
				} else if (purple_strequal(child->name, "x")) {
					/* TODO: See #7814 -- this might cause problems if anyone
					 * ever actually specifies forms. In fact, for this to
					 * work properly, that bug needs to be fixed in
					 * purple_xmlnode_from_str, not the output version... */
					value->forms = g_list_append(value->forms, purple_xmlnode_copy(child));
				}
			}

			g_hash_table_replace(capstable, key, value);
		}
	}
	purple_xmlnode_free(capsdata);
}

void jabber_caps_init(void)
{
	capstable = g_hash_table_new_full(jabber_caps_hash, jabber_caps_compare, NULL, (GDestroyNotify)jabber_caps_client_info_destroy);
	jabber_caps_load();
}

void jabber_caps_uninit(void)
{
	if (save_timer != 0) {
		g_source_remove(save_timer);
		save_timer = 0;
		do_jabber_caps_store(NULL);
	}
	g_hash_table_destroy(capstable);
	capstable = NULL;
}

typedef struct {
	jabber_caps_get_info_cb cb;
	gpointer cb_data;

	char *who;
	char *node;
	char *ver;
	char *hash;

	JabberCapsClientInfo *info;
} jabber_caps_cbplususerdata;

static void
cbplususerdata_destroy(jabber_caps_cbplususerdata *data)
{
	if (data == NULL)
		return;

	g_free(data->who);
	g_free(data->node);
	g_free(data->ver);
	g_free(data->hash);

	/* If we have info here, it's already in the capstable, so don't free it */

	g_free(data);
}

static void
jabber_caps_get_info_complete(jabber_caps_cbplususerdata *userdata)
{
	if (userdata->cb) {
		userdata->cb(userdata->info, userdata->cb_data);
		userdata->info = NULL;
	}
}

static void
jabber_caps_client_iqcb(G_GNUC_UNUSED JabberStream *js,
                        G_GNUC_UNUSED const char *from, JabberIqType type,
                        G_GNUC_UNUSED const char *id, PurpleXmlNode *packet,
                        gpointer data)
{
	jabber_caps_cbplususerdata *userdata = data;
	PurpleXmlNode *query = NULL;
	JabberCapsClientInfo *info = NULL, *value;
	JabberCapsTuple key;
	gchar *hash = NULL;
	GChecksumType hash_type;
	gboolean supported_hash = TRUE;

	query = purple_xmlnode_get_child_with_namespace(packet, "query",
	                                                NS_DISCO_INFO);
	if(query == NULL || type == JABBER_IQ_ERROR) {
		userdata->cb(NULL, userdata->cb_data);
		cbplususerdata_destroy(userdata);
		return;
	}

	/* check hash */
	info = jabber_caps_parse_client_info(query);

	if(purple_strequal(userdata->hash, "sha-1")) {
		hash_type = G_CHECKSUM_SHA1;
	} else if(purple_strequal(userdata->hash, "md5")) {
		hash_type = G_CHECKSUM_MD5;
	} else {
		supported_hash = FALSE;
	}

	if (supported_hash) {
		hash = jabber_caps_calculate_hash(info, hash_type);
	}

	if (hash == NULL || !purple_strequal(hash, userdata->ver)) {
		purple_debug_warning("jabber",
		                     "Could not validate caps info from %s. "
		                     "Expected %s, got %s",
		                     purple_xmlnode_get_attrib(packet, "from"),
		                     userdata->ver, hash ? hash : "(null)");

		userdata->cb(NULL, userdata->cb_data);
		jabber_caps_client_info_destroy(info);
		cbplususerdata_destroy(userdata);
		g_free(hash);
		return;
	}

	g_free(hash);

	key.node = userdata->node;
	key.ver  = userdata->ver;
	key.hash = userdata->hash;

	/* Use the copy of this data already in the table if it exists or insert
	 * a new one if we need to */
	if ((value = g_hash_table_lookup(capstable, &key))) {
		jabber_caps_client_info_destroy(info);
		info = value;
	} else {
		JabberCapsTuple *n_key = NULL;

		if (G_UNLIKELY(info == NULL)) {
			g_warn_if_reached();
			return;
		}

		n_key = (JabberCapsTuple *)&info->tuple;
		n_key->node = userdata->node;
		n_key->ver  = userdata->ver;
		n_key->hash = userdata->hash;
		userdata->node = userdata->ver = userdata->hash = NULL;

		/* The capstable gets a reference */
		g_hash_table_insert(capstable, n_key, info);
		schedule_caps_save();
	}

	userdata->info = info;

	jabber_caps_get_info_complete(userdata);

	cbplususerdata_destroy(userdata);
}

void
jabber_caps_get_info(JabberStream *js, const char *who, const char *node,
                     const char *ver, const char *hash,
                     jabber_caps_get_info_cb cb, gpointer user_data)
{
	JabberCapsClientInfo *info = NULL;
	JabberCapsTuple key;
	jabber_caps_cbplususerdata *userdata = NULL;
	JabberIq *iq = NULL;
	PurpleXmlNode *query = NULL;
	char *nodever = NULL;

	/* Using this in a read-only fashion, so the cast is OK */
	key.node = (char *)node;
	key.ver = (char *)ver;
	key.hash = (char *)hash;

	info = g_hash_table_lookup(capstable, &key);
	if (info != NULL) {
		/* We already have all the information we care about */
		if (cb) {
			cb(info, user_data);
		}
		return;
	}

	userdata = g_new0(jabber_caps_cbplususerdata, 1);
	/* We start out with 0 references. Every query takes one */
	userdata->cb = cb;
	userdata->cb_data = user_data;
	userdata->who = g_strdup(who);
	userdata->node = g_strdup(node);
	userdata->ver = g_strdup(ver);
	userdata->hash = g_strdup(hash);

	/* If we don't have the basic information about the client, we need to
	 * fetch it. */
	iq = jabber_iq_new_query(js, JABBER_IQ_GET, NS_DISCO_INFO);
	query = purple_xmlnode_get_child_with_namespace(iq->node, "query",
	                                                NS_DISCO_INFO);
	nodever = g_strdup_printf("%s#%s", node, ver);
	purple_xmlnode_set_attrib(query, "node", nodever);
	g_free(nodever);
	purple_xmlnode_set_attrib(iq->node, "to", who);

	jabber_iq_set_callback(iq, jabber_caps_client_iqcb, userdata);
	jabber_iq_send(iq);
}

static gint
jabber_xdata_compare(gconstpointer a, gconstpointer b)
{
	const PurpleXmlNode *aformtypefield = a;
	const PurpleXmlNode *bformtypefield = b;
	char *aformtype;
	char *bformtype;
	int result;

	aformtype = jabber_x_data_get_formtype(aformtypefield);
	bformtype = jabber_x_data_get_formtype(bformtypefield);

	result = strcmp(aformtype, bformtype);
	g_free(aformtype);
	g_free(bformtype);
	return result;
}

JabberCapsClientInfo *jabber_caps_parse_client_info(PurpleXmlNode *query)
{
	PurpleXmlNode *child;
	JabberCapsClientInfo *info;

	if (!query || !purple_strequal(query->name, "query") ||
			!purple_strequal(query->xmlns, NS_DISCO_INFO))
		return NULL;

	info = g_new0(JabberCapsClientInfo, 1);

	for(child = query->child; child; child = child->next) {
		if (child->type != PURPLE_XMLNODE_TYPE_TAG)
			continue;
		if (purple_strequal(child->name, "identity")) {
			/* parse identity */
			const char *category = purple_xmlnode_get_attrib(child, "category");
			const char *type = purple_xmlnode_get_attrib(child, "type");
			const char *name = purple_xmlnode_get_attrib(child, "name");
			const char *lang = purple_xmlnode_get_attrib(child, "lang");
			JabberIdentity *id;

			if (!category || !type)
				continue;

			id = jabber_identity_new(category, type, lang, name);
			info->identities = g_list_append(info->identities, id);
		} else if (purple_strequal(child->name, "feature")) {
			/* parse feature */
			const char *var = purple_xmlnode_get_attrib(child, "var");
			if (var)
				info->features = g_list_prepend(info->features, g_strdup(var));
		} else if (purple_strequal(child->name, "x")) {
			if (purple_strequal(child->xmlns, "jabber:x:data")) {
				/* x-data form */
				PurpleXmlNode *dataform = purple_xmlnode_copy(child);
				info->forms = g_list_append(info->forms, dataform);
			}
		}
	}
	return info;
}

static gint jabber_caps_xdata_field_compare(gconstpointer a, gconstpointer b)
{
	const JabberDataFormField *ac = a;
	const JabberDataFormField *bc = b;

	return strcmp(ac->var, bc->var);
}

static GList* jabber_caps_xdata_get_fields(const PurpleXmlNode *x)
{
	GList *fields = NULL;
	PurpleXmlNode *field;

	if (!x)
		return NULL;

	for (field = purple_xmlnode_get_child(x, "field"); field; field = purple_xmlnode_get_next_twin(field)) {
		PurpleXmlNode *value;
		JabberDataFormField *xdatafield = g_new0(JabberDataFormField, 1);
		xdatafield->var = g_strdup(purple_xmlnode_get_attrib(field, "var"));

		for (value = purple_xmlnode_get_child(field, "value"); value; value = purple_xmlnode_get_next_twin(value)) {
			gchar *val = purple_xmlnode_get_data(value);
			xdatafield->values = g_list_prepend(xdatafield->values, val);
		}

		xdatafield->values = g_list_sort(xdatafield->values, (GCompareFunc)strcmp);
		fields = g_list_prepend(fields, xdatafield);
	}

	fields = g_list_sort(fields, jabber_caps_xdata_field_compare);
	return fields;
}

static void
append_escaped_string(GChecksum *hash, const gchar *str)
{
	g_return_if_fail(hash != NULL);

	if (str && *str) {
		char *tmp = g_markup_escape_text(str, -1);
		g_checksum_update(hash, (const guchar *)tmp, -1);
		g_free(tmp);
	}

	g_checksum_update(hash, (const guchar *)"<", -1);
}

gchar *jabber_caps_calculate_hash(JabberCapsClientInfo *info,
	GChecksumType hash_type)
{
	GChecksum *hash;
	GList *node;
	guint8 *checksum;
	gsize checksum_size;
	gchar *ret;

	if (!info)
		return NULL;

	/* sort identities, features and x-data forms */
	info->identities = g_list_sort(info->identities, jabber_identity_compare);
	info->features = g_list_sort(info->features, (GCompareFunc)strcmp);
	info->forms = g_list_sort(info->forms, jabber_xdata_compare);

	hash = g_checksum_new(hash_type);

	if (hash == NULL) {
		return NULL;
	}

	/* Add identities to the hash data */
	for (node = info->identities; node; node = node->next) {
		JabberIdentity *id = (JabberIdentity*)node->data;
		char *category = g_markup_escape_text(id->category, -1);
		char *type = g_markup_escape_text(id->type, -1);
		char *lang = NULL;
		char *name = NULL;
		char *tmp;

		if (id->lang)
			lang = g_markup_escape_text(id->lang, -1);
		if (id->name)
			name = g_markup_escape_text(id->name, -1);

		tmp = g_strconcat(category, "/", type, "/", lang ? lang : "",
		                  "/", name ? name : "", "<", NULL);

		g_checksum_update(hash, (const guchar *)tmp, -1);

		g_free(tmp);
		g_free(category);
		g_free(type);
		g_free(lang);
		g_free(name);
	}

	/* concat features to the verification string */
	for (node = info->features; node; node = node->next) {
		append_escaped_string(hash, node->data);
	}

	/* concat x-data forms to the verification string */
	for(node = info->forms; node; node = node->next) {
		PurpleXmlNode *data = (PurpleXmlNode *)node->data;
		gchar *formtype = jabber_x_data_get_formtype(data);
		GList *fields = jabber_caps_xdata_get_fields(data);

		/* append FORM_TYPE's field value to the verification string */
		append_escaped_string(hash, formtype);
		g_free(formtype);

		while (fields) {
			JabberDataFormField *field = (JabberDataFormField*)fields->data;

			if (!purple_strequal(field->var, "FORM_TYPE")) {
				/* Append the "var" attribute */
				append_escaped_string(hash, field->var);
				/* Append <value/> elements' cdata */
				while (field->values) {
					append_escaped_string(hash, field->values->data);
					g_free(field->values->data);
					field->values = g_list_delete_link(field->values,
					                                   field->values);
				}
			} else {
				g_list_free_full(field->values, g_free);
			}

			g_free(field->var);
			g_free(field);

			fields = g_list_delete_link(fields, fields);
		}
	}

	checksum_size = g_checksum_type_get_length(hash_type);
	checksum = g_new(guint8, checksum_size);

	/* generate hash */
	g_checksum_get_digest(hash, checksum, &checksum_size);

	ret = g_base64_encode(checksum, checksum_size);
	g_free(checksum);
	g_checksum_free(hash);

	return ret;
}

void jabber_caps_calculate_own_hash(JabberStream *js) {
	JabberCapsClientInfo info;
	GList *iter = NULL;
	GList *features = NULL;

	if (!jabber_identities && !jabber_features) {
		/* This really shouldn't ever happen */
		purple_debug_warning("jabber", "No features or identities, cannot calculate own caps hash.\n");
		g_free(js->caps_hash);
		js->caps_hash = NULL;
		return;
	}

	/* build the currently-supported list of features */
	if (jabber_features) {
		for (iter = jabber_features; iter; iter = iter->next) {
			JabberFeature *feat = iter->data;
			if(!feat->is_enabled || feat->is_enabled(js, feat->namespace)) {
				features = g_list_append(features, feat->namespace);
			}
		}
	}

	info.features = features;
	/* TODO: This copy can go away, I think, since jabber_identities
	 * is pre-sorted, so the sort in calculate_hash should be idempotent.
	 * However, I want to test that. --darkrain
	 */
	info.identities = g_list_copy(jabber_identities);
	info.forms = NULL;

	g_free(js->caps_hash);
	js->caps_hash = jabber_caps_calculate_hash(&info, G_CHECKSUM_SHA1);
	g_list_free(info.identities);
	g_list_free(info.features);
}

const gchar* jabber_caps_get_own_hash(JabberStream *js)
{
	if (!js->caps_hash)
		jabber_caps_calculate_own_hash(js);

	return js->caps_hash;
}

void
jabber_caps_broadcast_change(void)
{
	PurpleAccountManager *manager = NULL;
	GList *node, *accounts;

	manager = purple_account_manager_get_default();
	accounts = purple_account_manager_get_enabled(manager);

	for (node = accounts; node; node = node->next) {
		PurpleAccount *account = node->data;
		const char *protocol_id = purple_account_get_protocol_id(account);
		if (purple_strequal("prpl-jabber", protocol_id) && purple_account_is_connected(account)) {
			PurpleConnection *gc = purple_account_get_connection(account);
			jabber_presence_send(purple_connection_get_protocol_data(gc), TRUE);
		}
	}

	g_list_free(accounts);
}

