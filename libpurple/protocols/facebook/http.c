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

#include <glib/gi18n-lib.h>

#include <string.h>

#include "http.h"

GQuark
fb_http_error_quark(void)
{
	static GQuark q = 0;

	if (G_UNLIKELY(q == 0)) {
		q = g_quark_from_static_string("fb-http-error-quark");
	}

	return q;
}

gboolean
fb_http_error_chk(SoupMessage *res, GError **error)
{
	SoupStatus status_code = soup_message_get_status(res);

	if (SOUP_STATUS_IS_SUCCESSFUL(status_code)) {
		return TRUE;
	}

	g_set_error(error, FB_HTTP_ERROR, status_code, "%s",
	            soup_message_get_reason_phrase(res));
	return FALSE;
}

FbHttpParams *
fb_http_params_new(void)
{
        return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

FbHttpParams *
fb_http_params_new_parse(const gchar *data, gboolean isurl)
{
	gchar *query = NULL;
	FbHttpParams *params;

	if (data == NULL) {
		return fb_http_params_new();
	}

	if (isurl) {
		if (!g_uri_split(data, G_URI_FLAGS_ENCODED_QUERY, NULL, NULL, NULL,
		                 NULL, NULL, &query, NULL, NULL))
		{
			return fb_http_params_new();
		}

		data = query;
	}

	params = soup_form_decode(data);

	g_free(query);

	return params;
}

void
fb_http_params_free(FbHttpParams *params)
{
	g_hash_table_destroy(params);
}

static const gchar *
fb_http_params_get(FbHttpParams *params, const gchar *name, GError **error)
{
	const gchar *ret;

	ret = g_hash_table_lookup(params, name);

	if (ret == NULL) {
		g_set_error(error, FB_HTTP_ERROR, FB_HTTP_ERROR_NOMATCH,
		            _("No matches for %s"), name);
		return NULL;
	}

	return ret;
}

gboolean
fb_http_params_get_bool(FbHttpParams *params, const gchar *name,
                        GError **error)
{
	const gchar *val;

	val = fb_http_params_get(params, name, error);

	if (val == NULL) {
		return FALSE;
	}

	return g_ascii_strcasecmp(val, "TRUE") == 0;
}

gdouble
fb_http_params_get_dbl(FbHttpParams *params, const gchar *name,
                       GError **error)
{
	const gchar *val;

	val = fb_http_params_get(params, name, error);

	if (val == NULL) {
		return 0.0;
	}

	return g_ascii_strtod(val, NULL);
}

gint64
fb_http_params_get_int(FbHttpParams *params, const gchar *name,
                       GError **error)
{
	const gchar *val;

	val = fb_http_params_get(params, name, error);

	if (val == NULL) {
		return 0;
	}

	return g_ascii_strtoll(val, NULL, 10);
}

const gchar *
fb_http_params_get_str(FbHttpParams *params, const gchar *name,
                       GError **error)
{
	return fb_http_params_get(params, name, error);
}

gchar *
fb_http_params_dup_str(FbHttpParams *params, const gchar *name,
                       GError **error)
{
	const gchar *str;

	str = fb_http_params_get(params, name, error);
	return g_strdup(str);
}

static void
fb_http_params_set(FbHttpParams *params, const gchar *name, gchar *value)
{
	gchar *key;

	key = g_strdup(name);
	g_hash_table_replace(params, key, value);
}

void
fb_http_params_set_bool(FbHttpParams *params, const gchar *name,
                        gboolean value)
{
	gchar *val;

	val = g_strdup(value ? "true" : "false");
	fb_http_params_set(params, name, val);
}

void
fb_http_params_set_dbl(FbHttpParams *params, const gchar *name, gdouble value)
{
	gchar *val;

	val = g_strdup_printf("%f", value);
	fb_http_params_set(params, name, val);
}

void
fb_http_params_set_int(FbHttpParams *params, const gchar *name, gint64 value)
{
	gchar *val;

	val = g_strdup_printf("%" G_GINT64_FORMAT, value);
	fb_http_params_set(params, name, val);
}

void
fb_http_params_set_str(FbHttpParams *params, const gchar *name,
                       const gchar *value)
{
	gchar *val;

	val = g_strdup(value);
	fb_http_params_set(params, name, val);
}

void
fb_http_params_set_strf(FbHttpParams *params, const gchar *name,
                        const gchar *format, ...)
{
	gchar *val;
	va_list ap;

	va_start(ap, format);
	val = g_strdup_vprintf(format, ap);
	va_end(ap);

	fb_http_params_set(params, name, val);
}

gboolean
fb_http_urlcmp(const gchar *url1, const gchar *url2, gboolean protocol)
{
	gboolean ret = TRUE;
#if SOUP_MAJOR_VERSION >= 3
	GUri *uri1, *uri2;
#else
	SoupURI *uri1, *uri2;
#endif

	if ((url1 == NULL) || (url2 == NULL)) {
		return url1 == url2;
	}

	if (strstr(url1, url2) != NULL || strstr(url2, url1) != NULL) {
		return TRUE;
	}

#if SOUP_MAJOR_VERSION >= 3
	uri1 = g_uri_parse(url1, SOUP_HTTP_URI_FLAGS, NULL);
	if (uri1 == NULL) {
		return g_ascii_strcasecmp(url1, url2) == 0;
	}

	uri2 = g_uri_parse(url2, SOUP_HTTP_URI_FLAGS, NULL);
	if (uri2 == NULL) {
		g_uri_unref(uri1);
		return g_ascii_strcasecmp(url1, url2) == 0;
	}

	if (!protocol) {
		/* Force the same scheme (and same port). */
		GUri *tmp = NULL;

		tmp = soup_uri_copy(uri1,
		                    SOUP_URI_SCHEME, "https",
		                    SOUP_URI_PORT, 443,
		                    SOUP_URI_NONE);
		g_uri_unref(uri1);
		uri1 = tmp;

		tmp = soup_uri_copy(uri2,
		                    SOUP_URI_SCHEME, "https",
		                    SOUP_URI_PORT, 443,
		                    SOUP_URI_NONE);
		g_uri_unref(uri2);
		uri2 = tmp;
	}
#else
	uri1 = soup_uri_new(url1);
	if (uri1 == NULL) {
		return g_ascii_strcasecmp(url1, url2) == 0;
	}

	uri2 = soup_uri_new(url2);
	if (uri2 == NULL) {
		soup_uri_free(uri1);
		return g_ascii_strcasecmp(url1, url2) == 0;
	}

	if (!protocol) {
		/* Force the same scheme (and same port). */
		soup_uri_set_scheme(uri1, SOUP_URI_SCHEME_HTTPS);
		soup_uri_set_scheme(uri2, SOUP_URI_SCHEME_HTTPS);
	}
#endif

	ret = soup_uri_equal(uri1, uri2);

#if SOUP_MAJOR_VERSION >= 3
	g_uri_unref(uri1);
	g_uri_unref(uri2);
#else
	soup_uri_free(uri1);
	soup_uri_free(uri2);
#endif
	return ret;
}
