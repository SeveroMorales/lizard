/* purple
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <libsoup/soup.h>
#include <string.h>

#include <purple.h>
#include "libpurple/glibcompat.h"

#include "api.h"
#include "data.h"

/**
 * FbData:
 *
 * Represents the connection data used by #FacebookProtocol.
 */
struct _FbData {
	GObject parent;

	FbApi *api;
	SoupSession *cons;
	PurpleConnection *gc;
	PurpleRoomlist *roomlist;
	GQueue *msgs;
	GHashTable *imgs;
	GHashTable *unread;
	GHashTable *evs;
};

/**
 * FbDataImage:
 *
 * Represents the data used for fetching images.
 */
struct _FbDataImage {
	GObject parent;

	FbData *fata;
	gchar *url;
	FbDataImageFunc func;
	gpointer data;
	GDestroyNotify dunc;

	gboolean active;
	const guint8 *image;
	gsize size;
};

static const gchar *fb_props_strs[] = {
	"cid",
	"did",
	"stoken",
	"token"
};

G_DEFINE_TYPE(FbData, fb_data, G_TYPE_OBJECT);
G_DEFINE_TYPE(FbDataImage, fb_data_image, G_TYPE_OBJECT);

static void
fb_data_dispose(GObject *obj)
{
	FbData *fata = FB_DATA(obj);

	if(fata->cons != NULL) {
		soup_session_abort(fata->cons);
	}

	if(fata->evs != NULL) {
		GHashTableIter iter;
		gpointer ptr = NULL;

		g_hash_table_iter_init(&iter, fata->evs);
		while (g_hash_table_iter_next(&iter, NULL, &ptr)) {
			g_source_remove(GPOINTER_TO_UINT(ptr));
		}
	}

	g_clear_object(&fata->api);

	g_clear_object(&fata->cons);
	if(fata->msgs != NULL) {
		g_queue_free_full(fata->msgs, (GDestroyNotify)fb_api_message_free);
		fata->msgs = NULL;
	}

	g_clear_pointer(&fata->imgs, g_hash_table_destroy);
	g_clear_pointer(&fata->unread, g_hash_table_destroy);
	g_clear_pointer(&fata->evs, g_hash_table_destroy);
}

static void
fb_data_class_init(FbDataClass *klass)
{
	GObjectClass *gklass = G_OBJECT_CLASS(klass);

	gklass->dispose = fb_data_dispose;
}

static void
fb_data_init(FbData *fata)
{
	fata->msgs = g_queue_new();

	fata->imgs = g_hash_table_new_full(g_direct_hash, g_direct_equal,
	                                   g_object_unref, NULL);
	fata->unread = g_hash_table_new_full(fb_id_hash, fb_id_equal,
	                                     g_free, NULL);
	fata->evs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static void
fb_data_image_dispose(GObject *obj)
{
	FbDataImage *img = FB_DATA_IMAGE(obj);
	FbData *fata = img->fata;

	if (img->dunc != NULL && img->data != NULL) {
		img->dunc(img->data);
		img->dunc = NULL;
	}

	g_clear_pointer(&img->url, g_free);
	g_hash_table_steal(fata->imgs, img);
}

static void
fb_data_image_class_init(FbDataImageClass *klass)
{
	GObjectClass *gklass = G_OBJECT_CLASS(klass);

	gklass->dispose = fb_data_image_dispose;
}

static void
fb_data_image_init(G_GNUC_UNUSED FbDataImage *img)
{
}

FbData *
fb_data_new(PurpleConnection *gc, GProxyResolver *resolver)
{
	FbData *fata;

	fata = g_object_new(FB_TYPE_DATA, NULL);

	fata->cons = soup_session_new_with_options("proxy-resolver", resolver,
	                                           NULL);
	fata->api = fb_api_new(gc, resolver);
	fata->gc = gc;

	return fata;
}

gboolean
fb_data_load(FbData *fata)
{
	const gchar *str;
	FbId id;
	gboolean ret = TRUE;
	guint i;
	guint64 uint;
	GValue val = G_VALUE_INIT;
	PurpleAccount *acct;

	g_return_val_if_fail(FB_IS_DATA(fata), FALSE);
	acct = purple_connection_get_account(fata->gc);

	for (i = 0; i < G_N_ELEMENTS(fb_props_strs); i++) {
		str = purple_account_get_string(acct, fb_props_strs[i], NULL);

		if (str == NULL) {
			ret = FALSE;
		}

		g_value_init(&val, G_TYPE_STRING);
		g_value_set_string(&val, str);
		g_object_set_property(G_OBJECT(fata->api), fb_props_strs[i],
		                      &val);
		g_value_unset(&val);
	}

	str = purple_account_get_string(acct, "mid", NULL);

	if (str != NULL) {
		uint = g_ascii_strtoull(str, NULL, 10);
		g_value_init(&val, G_TYPE_UINT64);
		g_value_set_uint64(&val, uint);
		g_object_set_property(G_OBJECT(fata->api), "mid", &val);
		g_value_unset(&val);
	} else {
		ret = FALSE;
	}

	str = purple_account_get_string(acct, "uid", NULL);

	if (str != NULL) {
		id = FB_ID_FROM_STR(str);
		g_value_init(&val, FB_TYPE_ID);
		g_value_set_int64(&val, id);
		g_object_set_property(G_OBJECT(fata->api), "uid", &val);
		g_value_unset(&val);
	} else {
		ret = FALSE;
	}

	fb_api_rehash(fata->api);
	return ret;
}

void
fb_data_save(FbData *fata)
{
	const gchar *str;
	gchar *dup;
	guint i;
	guint64 uint;
	GValue val = G_VALUE_INIT;
	PurpleAccount *acct;

	g_return_if_fail(FB_IS_DATA(fata));
	acct = purple_connection_get_account(fata->gc);

	for (i = 0; i < G_N_ELEMENTS(fb_props_strs); i++) {
		g_value_init(&val, G_TYPE_STRING);
		g_object_get_property(G_OBJECT(fata->api), fb_props_strs[i],
		                      &val);
		str = g_value_get_string(&val);

		if (purple_strequal(fb_props_strs[i], "token") && !purple_account_get_remember_password(acct)) {
			str = "";
		}
		purple_account_set_string(acct, fb_props_strs[i], str);
		g_value_unset(&val);
	}

	g_value_init(&val, G_TYPE_UINT64);
	g_object_get_property(G_OBJECT(fata->api), "mid", &val);
	uint = g_value_get_uint64(&val);
	g_value_unset(&val);

	dup = g_strdup_printf("%" G_GINT64_FORMAT, uint);
	purple_account_set_string(acct, "mid", dup);
	g_free(dup);

	g_value_init(&val, G_TYPE_INT64);
	g_object_get_property(G_OBJECT(fata->api), "uid", &val);
	uint = g_value_get_int64(&val);
	g_value_unset(&val);

	dup = g_strdup_printf("%" FB_ID_FORMAT, uint);
	purple_account_set_string(acct, "uid", dup);
	g_free(dup);
}

void
fb_data_save_timeout(FbData *fata, const gchar *name, guint id)
{
	g_return_if_fail(FB_IS_DATA(fata));

	fb_data_clear_timeout(fata, name, TRUE);

	g_hash_table_replace(fata->evs, g_strdup(name), GUINT_TO_POINTER(id));
}

void
fb_data_clear_timeout(FbData *fata, const gchar *name, gboolean remove)
{
	gpointer ptr;
	guint id;

	g_return_if_fail(FB_IS_DATA(fata));

	ptr = g_hash_table_lookup(fata->evs, name);
	id = GPOINTER_TO_UINT(ptr);

	if ((id > 0) && remove) {
		g_source_remove(id);
	}

	g_hash_table_remove(fata->evs, name);
}

FbApi *
fb_data_get_api(FbData *fata)
{

	g_return_val_if_fail(FB_IS_DATA(fata), NULL);

	return fata->api;
}

PurpleConnection *
fb_data_get_connection(FbData *fata)
{
	g_return_val_if_fail(FB_IS_DATA(fata), NULL);

	return fata->gc;
}

PurpleRoomlist *
fb_data_get_roomlist(FbData *fata)
{
	g_return_val_if_fail(FB_IS_DATA(fata), NULL);

	return fata->roomlist;
}

gboolean
fb_data_get_unread(FbData *fata, FbId id)
{
	gpointer *ptr;

	g_return_val_if_fail(FB_IS_DATA(fata), FALSE);
	g_return_val_if_fail(id != 0, FALSE);

	ptr = g_hash_table_lookup(fata->unread, &id);
	return GPOINTER_TO_INT(ptr);
}

void
fb_data_set_roomlist(FbData *fata, PurpleRoomlist *list)
{
	g_return_if_fail(FB_IS_DATA(fata));

	fata->roomlist = list;
}

void
fb_data_set_unread(FbData *fata, FbId id, gboolean unread)
{
	gpointer key;

	g_return_if_fail(FB_IS_DATA(fata));
	g_return_if_fail(id != 0);

	if (!unread) {
		g_hash_table_remove(fata->unread, &id);
		return;
	}

	key = g_memdup2(&id, sizeof id);
	g_hash_table_replace(fata->unread, key, GINT_TO_POINTER(unread));
}

void
fb_data_add_message(FbData *fata, FbApiMessage *msg)
{
	g_return_if_fail(FB_IS_DATA(fata));

	g_queue_push_tail(fata->msgs, msg);
}

void
fb_data_remove_message(FbData *fata, FbApiMessage *msg)
{
	g_return_if_fail(FB_IS_DATA(fata));

	g_queue_remove(fata->msgs, msg);
}

GSList *
fb_data_take_messages(FbData *fata, FbId uid)
{
	FbApiMessage *msg;
	GList *l;
	GList *prev;
	GSList *msgs = NULL;

	g_return_val_if_fail(FB_IS_DATA(fata), NULL);
	l = fata->msgs->tail;

	while (l != NULL) {
		msg = l->data;
		prev = l->prev;

		if (msg->uid == uid) {
			msgs = g_slist_prepend(msgs, msg);
			g_queue_delete_link(fata->msgs, l);
		}

		l = prev;
	}

	return msgs;
}

FbDataImage *
fb_data_image_add(FbData *fata, const gchar *url, FbDataImageFunc func,
                  gpointer data, GDestroyNotify dunc)
{
	FbDataImage *img;

	g_return_val_if_fail(FB_IS_DATA(fata), NULL);
	g_return_val_if_fail(url != NULL, NULL);
	g_return_val_if_fail(func != NULL, NULL);

	img = g_object_new(FB_TYPE_DATA_IMAGE, NULL);

	img->fata = fata;
	img->url = g_strdup(url);
	img->func = func;
	img->data = data;
	img->dunc = dunc;

	g_hash_table_add(fata->imgs, img);
	return img;
}

gboolean
fb_data_image_get_active(FbDataImage *img)
{
	g_return_val_if_fail(FB_IS_DATA_IMAGE(img), FALSE);

	return img->active;
}

gpointer
fb_data_image_get_data(FbDataImage *img)
{
	g_return_val_if_fail(FB_IS_DATA_IMAGE(img), NULL);

	return img->data;
}

FbData *
fb_data_image_get_fata(FbDataImage *img)
{
	g_return_val_if_fail(FB_IS_DATA_IMAGE(img), NULL);

	return img->fata;
}

const guint8 *
fb_data_image_get_image(FbDataImage *img, gsize *size)
{
	g_return_val_if_fail(FB_IS_DATA_IMAGE(img), NULL);

	if (size != NULL) {
		*size = img->size;
	}

	return img->image;
}

guint8 *
fb_data_image_dup_image(FbDataImage *img, gsize *size)
{
	g_return_val_if_fail(FB_IS_DATA_IMAGE(img), NULL);

	if (size != NULL) {
		*size = img->size;
	}

	if (img->size < 1) {
		return NULL;
	}

	return g_memdup2(img->image, img->size);
}

const gchar *
fb_data_image_get_url(FbDataImage *img)
{
	g_return_val_if_fail(FB_IS_DATA_IMAGE(img), NULL);

	return img->url;
}

static void
fb_data_image_cb(GObject *source, GAsyncResult *result, gpointer data) {
	SoupMessage *msg = data;
	FbDataImage *img = g_object_get_data(G_OBJECT(msg), "facebook-data-image");
	GError *err = NULL;

	if(fb_http_error_chk(msg, &err)) {
		GBytes *bytes = NULL;

		bytes = soup_session_send_and_read_finish(SOUP_SESSION(source),
		                                          result, &err);
		if(bytes != NULL) {
			img->image = g_bytes_unref_to_data(bytes, &img->size);
		}
	}

	img->func(img, err);

	if (G_LIKELY(err == NULL)) {
		fb_data_image_queue(img->fata);
	} else {
		g_error_free(err);
	}

	g_object_unref(img);
	g_object_unref(msg);
}

void
fb_data_image_queue(FbData *fata)
{
	const gchar *url;
	FbDataImage *img;
	GHashTableIter iter;
	guint active = 0;

	g_return_if_fail(FB_IS_DATA(fata));

	g_hash_table_iter_init(&iter, fata->imgs);
	while (g_hash_table_iter_next(&iter, (gpointer *) &img, NULL)) {
		if (fb_data_image_get_active(img)) {
			active++;
		}
	}

	if (active >= FB_DATA_ICON_MAX) {
		return;
	}

	g_hash_table_iter_init(&iter, fata->imgs);
	while (g_hash_table_iter_next(&iter, (gpointer *) &img, NULL)) {
		SoupMessage *msg;

		if (fb_data_image_get_active(img)) {
			continue;
		}

		img->active = TRUE;
		url = fb_data_image_get_url(img);

		msg = soup_message_new("GET", url);
		g_object_set_data(G_OBJECT(msg), "facebook-data-image", img);
		// purple_http_request_set_max_len(req, FB_DATA_ICON_SIZE_MAX);
		soup_session_send_and_read_async(fata->cons, msg, G_PRIORITY_DEFAULT,
		                                 NULL, fb_data_image_cb, msg);

		if (++active >= FB_DATA_ICON_MAX) {
			break;
		}
	}
}
