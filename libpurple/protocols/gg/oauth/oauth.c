/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * Code adapted from libgadu (C) 2008 Wojtek Kaniewski <wojtekka@irc.pl>
 * (http://toxygen.net/libgadu/) during Google Summer of Code 2012
 * by Tomek Wasilczyk (http://www.wasilczyk.pl).
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

#include "oauth.h"

#include <time.h>

#include <glib.h>

char *gg_oauth_static_nonce;		/* dla unit testów */
char *gg_oauth_static_timestamp;	/* dla unit testów */

static char *gg_oauth_generate_request(gboolean header, ...) G_GNUC_NULL_TERMINATED;

static void gg_oauth_generate_nonce(char *buf, int len)
{
	const char charset[] = "0123456789";

	if (buf == NULL || len < 1)
		return;

	while (len > 1) {
		*buf++ = charset[(unsigned) (((float) sizeof(charset) - 1.0) * g_random_int() / (RAND_MAX + 1.0))];
		len--;
	}

	*buf = 0;
}

/* Returns a comma separated header value if header is true,
 * or a url-encoded request otherwise
 */
static char *
gg_oauth_generate_request(gboolean header, ...)
{
	GString *res = g_string_new(NULL);
	va_list params;
	const gchar *key;
	gboolean truncate = FALSE;

	if(header) {
		res = g_string_append(res, "OAuth ");
	}

	va_start(params, header);
	while((key = va_arg(params, const gchar *))) {
		const gchar *value = va_arg(params, const gchar *);
		gchar *escaped = g_uri_escape_string(value, NULL, FALSE);

		if(header) {
			g_string_append_printf(res, "%s=\"%s\",", key, escaped);
		} else {
			g_string_append_printf(res, "%s=%s&", key, escaped);
		}

		g_free(escaped);

		truncate = TRUE;
	}
	va_end(params);

	if(truncate) {
		/* remove trailing separator */
		res = g_string_truncate(res, res->len - 1);
	}

	return g_string_free(res, FALSE);
}

static gchar *gg_hmac_sha1(const char *key, const char *message)
{
	GHmac *hmac;
	guchar digest[20];
	gsize digest_len = 20;

	hmac = g_hmac_new(G_CHECKSUM_SHA1, (guchar *)key, strlen(key));
	g_hmac_update(hmac, (guchar *)message, -1);
	g_hmac_get_digest(hmac, digest, &digest_len);
	g_hmac_unref(hmac);

	return g_base64_encode(digest, sizeof(digest));
}

static char *
gg_oauth_generate_signature(const char *method, const char *url,
	const char *request, const char *consumer_secret,
	const char *token_secret)
{
	char *text, *key, *res;
	gchar *url_e, *request_e, *consumer_secret_e, *token_secret_e;

	url_e = g_uri_escape_string(url, "?", FALSE);
	g_strdelimit(url_e, "?", '\0');
	request_e = g_uri_escape_string(request, NULL, FALSE);
	text = g_strdup_printf("%s&%s&%s", method, url_e, request_e);
	g_free(url_e);
	g_free(request_e);

	consumer_secret_e = g_uri_escape_string(consumer_secret, NULL, FALSE);
	token_secret_e = token_secret ? g_uri_escape_string(token_secret, NULL, FALSE) : NULL;
	key = g_strdup_printf("%s&%s", consumer_secret_e, token_secret ? token_secret_e : "");
	g_free(consumer_secret_e);
	g_free(token_secret_e);

	res = gg_hmac_sha1(key, text);

	g_free(key);
	g_free(text);

	return res;
}

char *
gg_oauth_generate_header(const char *method, const char *url,
	const char *consumer_key, const char *consumer_secret,
	const char *token, const char *token_secret)
{
	char *request, *signature, *res;
	char nonce[80], timestamp[16];

	if (gg_oauth_static_nonce == NULL)
		gg_oauth_generate_nonce(nonce, sizeof(nonce));
	else {
		strncpy(nonce, gg_oauth_static_nonce, sizeof(nonce) - 1);
		nonce[sizeof(nonce) - 1] = 0;
	}

	if (gg_oauth_static_timestamp == NULL) {
		g_snprintf(timestamp, sizeof(timestamp), "%ld", time(NULL));
	} else {
		strncpy(timestamp, gg_oauth_static_timestamp, sizeof(timestamp) - 1);
		timestamp[sizeof(timestamp) - 1] = 0;
	}

	request = gg_oauth_generate_request(FALSE,
	                                    "oauth_consumer_key", consumer_key,
	                                    "oauth_nonce", nonce,
	                                    "oauth_signature_method", "HMAC-SHA1",
	                                    "oauth_timestamp", timestamp,
	                                    "oauth_token", token,
	                                    "oauth_version", "1.0",
	                                    NULL);

	signature = gg_oauth_generate_signature(method, url, request, consumer_secret, token_secret);

	g_free(request);

	if (signature == NULL)
		return NULL;

	res = gg_oauth_generate_request(TRUE,
	                                "oauth_version", "1.0",
	                                "oauth_nonce", nonce,
	                                "oauth_timestamp", timestamp,
	                                "oauth_consumer_key", consumer_key,
	                                "oauth_token", token,
	                                "oauth_signature_method", "HMAC-SHA1",
	                                "oauth_signature", signature,
	                                NULL);
	g_free(signature);

	return res;
}
