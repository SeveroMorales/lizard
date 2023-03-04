/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * Rewritten from scratch during Google Summer of Code 2012
 * by Tomek Wasilczyk (http://www.wasilczyk.pl).
 *
 * Previously implemented by:
 *  - Arkadiusz Miskiewicz <misiek@pld.org.pl> - first implementation (2001);
 *  - Bartosz Oler <bartosz@bzimage.us> - reimplemented during GSoC 2005;
 *  - Krzysztof Klinikowski <grommasher@gmail.com> - some parts (2009-2011).
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

#include <errno.h>

#include <purple.h>

#include "utils.h"

#include "gg.h"

uin_t ggp_str_to_uin(const char *str)
{
	char *endptr;
	uin_t uin;

	if (!str || str[0] < '0' || str[0] > '9')
		return 0;

	errno = 0;
	uin = strtoul(str, &endptr, 10);

	if (errno == ERANGE || endptr[0] != '\0')
		return 0;

	return uin;
}

const char * ggp_uin_to_str(uin_t uin)
{
	static char buff[GGP_UIN_LEN_MAX + 1];

	g_snprintf(buff, GGP_UIN_LEN_MAX + 1, "%u", uin);

	return buff;
}

uin_t
ggp_get_my_uin(PurpleConnection *gc) {
	PurpleAccount *account = purple_connection_get_account(gc);
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);

	g_return_val_if_fail(gc != NULL, 0);

	return ggp_str_to_uin(purple_contact_info_get_username(info));
}

static gchar * ggp_convert(const gchar *src, const char *srcenc,
	const char *dstenc)
{
	gchar *dst;
	GError *err = NULL;

	if (src == NULL)
		return NULL;

	dst = g_convert_with_fallback(src, strlen(src), dstenc, srcenc, "?",
		NULL, NULL, &err);
	if (err != NULL) {
		purple_debug_error("gg", "error converting from %s to %s: %s\n",
			srcenc, dstenc, err->message);
		g_error_free(err);
	}

	if (dst == NULL)
		dst = g_strdup(src);

	return dst;
}

gchar * ggp_convert_to_cp1250(const gchar *src)
{
	return ggp_convert(src, "UTF-8", "CP1250");
}

gchar * ggp_convert_from_cp1250(const gchar *src)
{
	return ggp_convert(src, "CP1250", "UTF-8");
}

gchar * ggp_utf8_strndup(const gchar *str, gsize n)
{
	size_t raw_len;
	gchar *end_ptr;
	if (str == NULL)
		return NULL;
	raw_len = strlen(str);
	if (raw_len <= n)
		return g_strdup(str);

	end_ptr = g_utf8_offset_to_pointer(str, g_utf8_pointer_to_offset(str, &str[n]));
	raw_len = end_ptr - str;

	if (raw_len > n) {
		end_ptr = g_utf8_prev_char(end_ptr);
		raw_len = end_ptr - str;
	}

	g_assert(raw_len <= n);

	return g_strndup(str, raw_len);
}

const gchar * ggp_ipv4_to_str(uint32_t raw_ip)
{
	static gchar buff[INET_ADDRSTRLEN];
	buff[0] = '\0';

	g_snprintf(buff, sizeof(buff), "%d.%d.%d.%d",
		((raw_ip >>  0) & 0xFF),
		((raw_ip >>  8) & 0xFF),
		((raw_ip >> 16) & 0xFF),
		((raw_ip >> 24) & 0xFF));

	return buff;
}

gchar * ggp_free_if_equal(gchar *str, const gchar *pattern)
{
	if (g_strcmp0(str, pattern) == 0) {
		g_free(str);
		return NULL;
	}
	return str;
}

uint64_t * ggp_uint64dup(uint64_t val)
{
	uint64_t *ptr = g_new(uint64_t, 1);
	*ptr = val;
	return ptr;
}

JsonParser * ggp_json_parse(const gchar *data)
{
	JsonParser *parser;

	parser = json_parser_new();
	if (json_parser_load_from_data(parser, data, -1, NULL))
		return parser;

	if (purple_debug_is_unsafe())
		purple_debug_warning("gg", "Invalid JSON: %s\n", data);
	return NULL;
}
