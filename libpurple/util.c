/* Purple is the legal property of its developers, whose names are too numerous
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

#include "internal.h"

#include <glib/gi18n-lib.h>

#include <ctype.h>

#include "purpleprivate.h"

#include "core.h"
#include "debug.h"
#include "notify.h"
#include "prefs.h"
#include "purpleaccountmanager.h"
#include "purpleconversation.h"
#include "purplepath.h"
#include "purpleprotocol.h"
#include "purpleprotocolclient.h"
#include "util.h"

#include <json-glib/json-glib.h>

void
purple_util_init(void) {
}

void
purple_util_uninit(void) {
	purple_util_set_user_dir(NULL);
}

/**************************************************************************
 * Path/Filename Functions
 **************************************************************************/
static gboolean
purple_util_write_data_to_file_common(const char *dir, const char *filename, const char *data, gssize size)
{
	gchar *filename_full;
	gboolean ret = FALSE;

	g_return_val_if_fail(dir != NULL, FALSE);

	purple_debug_misc("util", "Writing file %s to directory %s",
			  filename, dir);

	/* Ensure the directory exists */
	if (!g_file_test(dir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
		{
			purple_debug_error("util", "Error creating directory %s: %s\n",
					   dir, g_strerror(errno));
			return FALSE;
		}
	}

	filename_full = g_build_filename(dir, filename, NULL);
	ret = g_file_set_contents(filename_full, data, size, NULL);
	g_free(filename_full);

	return ret;
}

gboolean
purple_util_write_data_to_cache_file(const char *filename, const char *data, gssize size)
{
	const char *cache_dir = purple_cache_dir();
	gboolean ret = purple_util_write_data_to_file_common(cache_dir, filename, data, size);

	return ret;
}

gboolean
purple_util_write_data_to_config_file(const char *filename, const char *data, gssize size)
{
	const char *config_dir = purple_config_dir();
	gboolean ret = purple_util_write_data_to_file_common(config_dir, filename, data, size);

	return ret;
}

gboolean
purple_util_write_data_to_data_file(const char *filename, const char *data, gssize size)
{
	const char *data_dir = purple_data_dir();
	gboolean ret = purple_util_write_data_to_file_common(data_dir, filename, data, size);

	return ret;
}

PurpleXmlNode *
purple_util_read_xml_from_cache_file(const char *filename, const char *description)
{
	return purple_xmlnode_from_file(purple_cache_dir(), filename, description, "util");
}

PurpleXmlNode *
purple_util_read_xml_from_config_file(const char *filename, const char *description)
{
	return purple_xmlnode_from_file(purple_config_dir(), filename, description, "util");
}

PurpleXmlNode *
purple_util_read_xml_from_data_file(const char *filename, const char *description)
{
	return purple_xmlnode_from_file(purple_data_dir(), filename, description, "util");
}

gboolean
purple_running_gnome(void)
{
#ifndef _WIN32
	gchar *tmp = g_find_program_in_path("gvfs-open");

	if (tmp == NULL) {
		tmp = g_find_program_in_path("gnome-open");

		if (tmp == NULL) {
			return FALSE;
		}
	}

	g_free(tmp);

	tmp = (gchar *)g_getenv("GNOME_DESKTOP_SESSION_ID");

	return ((tmp != NULL) && (*tmp != '\0'));
#else
	return FALSE;
#endif
}

gboolean
purple_running_kde(void)
{
#ifndef _WIN32
	gchar *tmp = g_find_program_in_path("kfmclient");
	const char *session;

	if (tmp == NULL)
		return FALSE;
	g_free(tmp);

	session = g_getenv("KDE_FULL_SESSION");
	if (purple_strequal(session, "true"))
		return TRUE;

	/* If you run Purple from Konsole under !KDE, this will provide a
	 * a false positive.  Since we do the GNOME checks first, this is
	 * only a problem if you're running something !(KDE || GNOME) and
	 * you run Purple from Konsole. This really shouldn't be a problem. */
	return ((g_getenv("KDEDIR") != NULL) || g_getenv("KDEDIRS") != NULL);
#else
	return FALSE;
#endif
}

/**************************************************************************
 * String Functions
 **************************************************************************/
const char *
purple_normalize(PurpleAccount *account, const char *str)
{
	const char *ret = NULL;
	static char buf[BUF_LEN];

	/* This should prevent a crash if purple_normalize gets called with NULL str, see #10115 */
	g_return_val_if_fail(str != NULL, "");

	if (account != NULL)
	{
		PurpleProtocol *protocol = purple_account_get_protocol(account);

		if(PURPLE_IS_PROTOCOL_CLIENT(protocol)) {
			ret = purple_protocol_client_normalize(PURPLE_PROTOCOL_CLIENT(protocol), account, str);
		}
	}

	if (ret == NULL)
	{
		char *tmp;

		tmp = g_utf8_normalize(str, -1, G_NORMALIZE_DEFAULT);
		g_snprintf(buf, sizeof(buf), "%s", tmp);
		g_free(tmp);

		ret = buf;
	}

	return ret;
}

/*
 * You probably don't want to call this directly, it is
 * mainly for use as a protocol callback function.  See the
 * comments in util.h.
 */
const char *
purple_normalize_nocase(const char *str)
{
	static char buf[BUF_LEN];
	char *tmp1, *tmp2;

	g_return_val_if_fail(str != NULL, NULL);

	tmp1 = g_utf8_strdown(str, -1);
	tmp2 = g_utf8_normalize(tmp1, -1, G_NORMALIZE_DEFAULT);
	g_snprintf(buf, sizeof(buf), "%s", tmp2 ? tmp2 : "");
	g_free(tmp2);
	g_free(tmp1);

	return buf;
}

gboolean
purple_validate(PurpleProtocol *protocol, const char *str)
{
	const char *normalized;

	g_return_val_if_fail(protocol != NULL, FALSE);
	g_return_val_if_fail(str != NULL, FALSE);

	if (str[0] == '\0')
		return FALSE;

	if (!PURPLE_PROTOCOL_IMPLEMENTS(protocol, CLIENT, normalize))
		return TRUE;

	normalized = purple_protocol_client_normalize(PURPLE_PROTOCOL_CLIENT(protocol),
	                                              NULL, str);

	return (NULL != normalized);
}

gchar *
purple_strdup_withhtml(const gchar *src)
{
	gulong destsize, i, j;
	gchar *dest;

	g_return_val_if_fail(src != NULL, NULL);

	/* New length is (length of src) + (number of \n's * 3) - (number of \r's) + 1 */
	destsize = 1;
	for (i = 0; src[i] != '\0'; i++)
	{
		if (src[i] == '\n')
			destsize += 4;
		else if (src[i] != '\r')
			destsize++;
	}

	dest = g_malloc(destsize);

	/* Copy stuff, ignoring \r's, because they are dumb */
	for (i = 0, j = 0; src[i] != '\0'; i++) {
		if (src[i] == '\n') {
			strcpy(&dest[j], "<BR>");
			j += 4;
		} else if (src[i] != '\r')
			dest[j++] = src[i];
	}

	dest[destsize-1] = '\0';

	return dest;
}

gboolean
purple_str_has_caseprefix(const gchar *s, const gchar *p)
{
	g_return_val_if_fail(s, FALSE);
	g_return_val_if_fail(p, FALSE);

	return (g_ascii_strncasecmp(s, p, strlen(p)) == 0);
}

void
purple_str_strip_char(char *text, char thechar)
{
	int i, j;

	g_return_if_fail(text != NULL);

	for (i = 0, j = 0; text[i]; i++)
		if (text[i] != thechar)
			text[j++] = text[i];

	text[j] = '\0';
}

void
purple_util_chrreplace(char *string, char delimiter,
					 char replacement)
{
	int i = 0;

	g_return_if_fail(string != NULL);

	while (string[i] != '\0')
	{
		if (string[i] == delimiter)
			string[i] = replacement;
		i++;
	}
}

gchar *
purple_strreplace(const char *string, const char *delimiter,
				const char *replacement)
{
	gchar **split;
	gchar *ret;

	g_return_val_if_fail(string      != NULL, NULL);
	g_return_val_if_fail(delimiter   != NULL, NULL);
	g_return_val_if_fail(replacement != NULL, NULL);

	split = g_strsplit(string, delimiter, 0);
	ret = g_strjoinv(replacement, split);
	g_strfreev(split);

	return ret;
}

char *
purple_str_seconds_to_string(guint secs)
{
	char *ret = NULL;
	guint days, hrs, mins;

	if (secs < 60)
	{
		return g_strdup_printf(dngettext(PACKAGE, "%d second", "%d seconds", secs), secs);
	}

	days = secs / (60 * 60 * 24);
	secs = secs % (60 * 60 * 24);
	hrs  = secs / (60 * 60);
	secs = secs % (60 * 60);
	mins = secs / 60;
	/* secs = secs % 60; */

	if (days > 0)
	{
		ret = g_strdup_printf(dngettext(PACKAGE, "%d day", "%d days", days), days);
	}

	if (hrs > 0)
	{
		if (ret != NULL)
		{
			char *tmp = g_strdup_printf(
					dngettext(PACKAGE, "%s, %d hour", "%s, %d hours", hrs),
							ret, hrs);
			g_free(ret);
			ret = tmp;
		}
		else
			ret = g_strdup_printf(dngettext(PACKAGE, "%d hour", "%d hours", hrs), hrs);
	}

	if (mins > 0)
	{
		if (ret != NULL)
		{
			char *tmp = g_strdup_printf(
					dngettext(PACKAGE, "%s, %d minute", "%s, %d minutes", mins),
							ret, mins);
			g_free(ret);
			ret = tmp;
		}
		else
			ret = g_strdup_printf(dngettext(PACKAGE, "%d minute", "%d minutes", mins), mins);
	}

	return ret;
}

void
purple_str_wipe(gchar *str)
{
	if (str == NULL)
		return;
	memset(str, 0, strlen(str));
	g_free(str);
}

gboolean
purple_strmatches(const char *pattern, const char *str) {
	char *normal_pattern = NULL;
	char *normal_str = NULL;
	char *cmp_pattern = NULL;
	char *cmp_str = NULL;
	char *idx_pattern = NULL;
	char *idx_str = NULL;

	g_return_val_if_fail(pattern != NULL, FALSE);

	/* Short circuit on NULL and empty string. */
	if(purple_strempty(str)) {
		return FALSE;
	}

	normal_pattern = g_utf8_normalize(pattern, -1, G_NORMALIZE_ALL);
	cmp_pattern = g_utf8_casefold(normal_pattern, -1);
	g_free(normal_pattern);

	normal_str = g_utf8_normalize(str, -1, G_NORMALIZE_ALL);
	cmp_str = g_utf8_casefold(normal_str, -1);
	g_free(normal_str);

	idx_pattern = cmp_pattern;
	idx_str = cmp_str;

	/* I know while(TRUE)'s suck, but the alternative would be a multi-line for
	 * loop that wouldn't have the additional comments, which is much better
	 * IMHO. -- GK 2023-01-24.
	 */
	while(TRUE) {
		gunichar character = g_utf8_get_char(idx_pattern);

		/* If we've reached the end of the pattern, we're done. */
		if(character == 0) {
			break;
		}

		idx_str = g_utf8_strchr(idx_str, -1, character);
		if(idx_str == NULL) {
			g_free(cmp_pattern);
			g_free(cmp_str);

			return FALSE;
		}

		/* We found the current character in pattern, so move to the next. */
		idx_pattern = g_utf8_next_char(idx_pattern);

		/* move idx_str to the next character as well, because the current
		 * character has already been matched.
		 */
		idx_str = g_utf8_next_char(idx_str);
	};

	g_free(cmp_pattern);
	g_free(cmp_str);

	return TRUE;
}

/**************************************************************************
 * URI/URL Functions
 **************************************************************************/

void purple_got_protocol_handler_uri(const char *uri)
{
	char proto[11];
	char delimiter;
	const char *tmp, *param_string;
	char *cmd;
	GHashTable *params = NULL;
	gsize len;
	if (!(tmp = strchr(uri, ':')) || tmp == uri) {
		purple_debug_error("util", "Malformed protocol handler message - missing protocol.\n");
		return;
	}

	len = MIN(sizeof(proto) - 1, (gsize)(tmp - uri));

	strncpy(proto, uri, len);
	proto[len] = '\0';

	tmp++;

	if (purple_strequal(proto, "xmpp"))
		delimiter = ';';
	else
		delimiter = '&';

	purple_debug_info("util", "Processing message '%s' for protocol '%s' using delimiter '%c'.\n", tmp, proto, delimiter);

	if ((param_string = strchr(tmp, '?'))) {
		const char *keyend = NULL, *pairstart;
		char *key, *value = NULL;

		cmd = g_strndup(tmp, (param_string - tmp));
		param_string++;

		params = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
		pairstart = tmp = param_string;

		while (*tmp || *pairstart) {
			if (*tmp == delimiter || !(*tmp)) {
				/* If there is no explicit value */
				if (keyend == NULL) {
					keyend = tmp;
				}
				/* without these brackets, clang won't
				 * recognize tmp as a non-NULL
				 */

				if (keyend && keyend != pairstart) {
					char *p;
					key = g_strndup(pairstart, (keyend - pairstart));
					/* If there is an explicit value */
					if (keyend != tmp && keyend != (tmp - 1))
						value = g_strndup(keyend + 1, (tmp - keyend - 1));
					for (p = key; *p; ++p)
						*p = g_ascii_tolower(*p);
					g_hash_table_insert(params, key, value);
				}
				keyend = value = NULL;
				pairstart = (*tmp) ? tmp + 1 : tmp;
			} else if (*tmp == '=')
				keyend = tmp;

			if (*tmp)
				tmp++;
		}
	} else
		cmd = g_strdup(tmp);

	purple_signal_emit_return_1(purple_get_core(), "uri-handler", proto, cmd, params);

	g_free(cmd);
	if (params)
		g_hash_table_destroy(params);
}

const char *
purple_url_encode(const char *str)
{
	const char *iter;
	static char buf[BUF_LEN];
	char utf_char[6];
	guint i, j = 0;

	g_return_val_if_fail(str != NULL, NULL);
	g_return_val_if_fail(g_utf8_validate(str, -1, NULL), NULL);

	iter = str;
	for (; *iter && j < (BUF_LEN - 1) ; iter = g_utf8_next_char(iter)) {
		gunichar c = g_utf8_get_char(iter);
		/* If the character is an ASCII character and is alphanumeric
		 * no need to escape */
		if (c < 128 && (isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~')) {
			buf[j++] = c;
		} else {
			int bytes = g_unichar_to_utf8(c, utf_char);
			for (i = 0; (int)i < bytes; i++) {
				if (j > (BUF_LEN - 4))
					break;
				if (i >= sizeof(utf_char)) {
					g_warn_if_reached();
					break;
				}
				sprintf(buf + j, "%%%02X", utf_char[i] & 0xff);
				j += 3;
			}
		}
	}

	buf[j] = '\0';

	return buf;
}

/* Originally lifted from
 * http://www.oreillynet.com/pub/a/network/excerpt/spcookbook_chap03/index3.html
 * ... and slightly modified to be a bit more rfc822 compliant
 * ... and modified a bit more to make domain checking rfc1035 compliant
 *     with the exception permitted in rfc1101 for domains to start with digit
 *     but not completely checking to avoid conflicts with IP addresses
 */
gboolean
purple_email_is_valid(const char *address)
{
	const char *c, *domain;
	static char *rfc822_specials = "()<>@,;:\\\"[]";

	g_return_val_if_fail(address != NULL, FALSE);

	if (*address == '.') return FALSE;

	/* first we validate the name portion (name@domain) (rfc822)*/
	for (c = address;  *c;  c++) {
		if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) == '\"')) {
			while (*++c) {
				if (*c == '\\') {
					if (*c++ && *c < 127 && *c > 0 && *c != '\n' && *c != '\r') continue;
					else return FALSE;
				}
				if (*c == '\"') break;
				if (*c < ' ' || *c >= 127) return FALSE;
			}
			if (!*c++) return FALSE;
			if (*c == '@') break;
			if (*c != '.') return FALSE;
			continue;
		}
		if (*c == '@') break;
		if (*c <= ' ' || *c >= 127) return FALSE;
		if (strchr(rfc822_specials, *c)) return FALSE;
	}

	/* It's obviously not an email address if we didn't find an '@' above */
	if (*c == '\0') return FALSE;

	/* strictly we should return false if (*(c - 1) == '.') too, but I think
	 * we should permit user.@domain type addresses - they do work :) */
	if (c == address) return FALSE;

	/* next we validate the domain portion (name@domain) (rfc1035 & rfc1011) */
	if (!*(domain = ++c)) return FALSE;
	do {
		if (*c == '.' && (c == domain || *(c - 1) == '.' || *(c - 1) == '-'))
			return FALSE;
		if (*c == '-' && (*(c - 1) == '.' || *(c - 1) == '@')) return FALSE;
		if ((*c < '0' && *c != '-' && *c != '.') || (*c > '9' && *c < 'A') ||
			(*c > 'Z' && *c < 'a') || (*c > 'z')) return FALSE;
	} while (*++c);

	if (*(c - 1) == '-') return FALSE;

	return ((c - domain) > 3 ? TRUE : FALSE);
}

/**************************************************************************
 * UTF8 String Functions
 **************************************************************************/
gchar *
purple_utf8_try_convert(const char *str)
{
	gsize converted;
	gchar *utf8;

	g_return_val_if_fail(str != NULL, NULL);

	if (g_utf8_validate(str, -1, NULL)) {
		return g_strdup(str);
	}

	utf8 = g_locale_to_utf8(str, -1, &converted, NULL, NULL);
	if (utf8 != NULL)
		return utf8;

	utf8 = g_convert(str, -1, "UTF-8", "ISO-8859-15", &converted, NULL, NULL);
	if ((utf8 != NULL) && (converted == strlen(str)))
		return utf8;

	g_free(utf8);

	return NULL;
}

gchar *
purple_utf8_strip_unprintables(const gchar *str)
{
	gchar *workstr, *iter;
	const gchar *bad;

	if (str == NULL)
		/* Act like g_strdup */
		return NULL;

	if (!g_utf8_validate(str, -1, &bad)) {
		purple_debug_error("util", "purple_utf8_strip_unprintables(%s) failed; "
		                           "first bad character was %02x (%c)\n",
		                   str, *bad, *bad);
		g_return_val_if_reached(NULL);
	}

	workstr = iter = g_new(gchar, strlen(str) + 1);
	while (*str) {
		gunichar ch = g_utf8_get_char(str);
		gchar *next = g_utf8_next_char(str);
		/*
		 * Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] |
		 *          [#x10000-#x10FFFF]
		 */
		if ((ch == '\t' || ch == '\n' || ch == '\r') ||
				(ch >= 0x20 && ch <= 0xD7FF) ||
				(ch >= 0xE000 && ch <= 0xFFFD) ||
				(ch >= 0x10000 && ch <= 0x10FFFF)) {
			memcpy(iter, str, next - str);
			iter += (next - str);
		}

		str = next;
	}

	/* nul-terminate the new string */
	*iter = '\0';

	return workstr;
}

char *
purple_utf8_ncr_encode(const char *str)
{
	GString *out;

	g_return_val_if_fail(str != NULL, NULL);
	g_return_val_if_fail(g_utf8_validate(str, -1, NULL), NULL);

	out = g_string_new("");

	for(; *str; str = g_utf8_next_char(str)) {
		gunichar wc = g_utf8_get_char(str);

		/* super simple check. hopefully not too wrong. */
		if(wc >= 0x80) {
			g_string_append_printf(out, "&#%u;", (guint32) wc);
		} else {
			g_string_append_unichar(out, wc);
		}
	}

	return g_string_free(out, FALSE);
}


char *
purple_utf8_ncr_decode(const char *str)
{
	GString *out;
	char *buf, *b;

	g_return_val_if_fail(str != NULL, NULL);
	g_return_val_if_fail(g_utf8_validate(str, -1, NULL), NULL);

	buf = (char *) str;
	out = g_string_new("");

	while( (b = strstr(buf, "&#")) ) {
		gunichar wc;
		int base = 0;

		/* append everything leading up to the &# */
		g_string_append_len(out, buf, b-buf);

		b += 2; /* skip past the &# */

		/* strtoul will treat 0x prefix as hex, but not just x */
		if(*b == 'x' || *b == 'X') {
			base = 16;
			b++;
		}

		/* advances buf to the end of the ncr segment */
		wc = (gunichar) strtoul(b, &buf, base);

		/* this mimics the previous impl of ncr_decode */
		if(*buf == ';') {
			g_string_append_unichar(out, wc);
			buf++;
		}
	}

	/* append whatever's left */
	g_string_append(out, buf);

	return g_string_free(out, FALSE);
}


int
purple_utf8_strcasecmp(const char *a, const char *b)
{
	char *a_norm = NULL;
	char *b_norm = NULL;
	int ret = -1;

	if(!a && b)
		return -1;
	else if(!b && a)
		return 1;
	else if(!a && !b)
		return 0;

	if(!g_utf8_validate(a, -1, NULL) || !g_utf8_validate(b, -1, NULL))
	{
		purple_debug_error("purple_utf8_strcasecmp",
						 "One or both parameters are invalid UTF8\n");
		return ret;
	}

	a_norm = g_utf8_casefold(a, -1);
	b_norm = g_utf8_casefold(b, -1);
	ret = g_utf8_collate(a_norm, b_norm);
	g_free(a_norm);
	g_free(b_norm);

	return ret;
}

/* previously conversation::find_nick() */
gboolean
purple_utf8_has_word(const char *haystack, const char *needle)
{
	char *hay, *pin, *p;
	const char *start, *prev_char;
	gunichar before, after;
	int n;
	gboolean ret = FALSE;

	start = hay = g_utf8_strdown(haystack, -1);

	pin = g_utf8_strdown(needle, -1);
	n = strlen(pin);

	while ((p = strstr(start, pin)) != NULL) {
		prev_char = g_utf8_find_prev_char(hay, p);
		before = -2;
		if (prev_char) {
			before = g_utf8_get_char(prev_char);
		}
		after = g_utf8_get_char_validated(p + n, - 1);

		if ((p == hay ||
				/* The character before is a reasonable guess for a word boundary
				   ("!g_unichar_isalnum()" is not a valid way to determine word
				    boundaries, but it is the only reasonable thing to do here),
				   and isn't the '&' from a "&amp;" or some such entity*/
				(before != (gunichar)-2 && !g_unichar_isalnum(before) && *(p - 1) != '&'))
				&& after != (gunichar)-2 && !g_unichar_isalnum(after)) {
			ret = TRUE;
			break;
		}
		start = p + 1;
	}

	g_free(pin);
	g_free(hay);

	return ret;
}

gboolean purple_message_meify(char *message, gssize len)
{
	char *c;
	gboolean inside_html = FALSE;

	g_return_val_if_fail(message != NULL, FALSE);

	if(len == -1)
		len = strlen(message);

	for (c = message; *c; c++, len--) {
		if(inside_html) {
			if(*c == '>')
				inside_html = FALSE;
		} else {
			if(*c == '<')
				inside_html = TRUE;
			else
				break;
		}
	}

	if(*c && !g_ascii_strncasecmp(c, "/me ", 4)) {
		memmove(c, c+4, len-3);
		return TRUE;
	}

	return FALSE;
}

char *purple_text_strip_mnemonic(const char *in)
{
	char *out;
	char *a;
	char *a0;
	const char *b;

	g_return_val_if_fail(in != NULL, NULL);

	out = g_malloc(strlen(in)+1);
	a = out;
	b = in;

	a0 = a; /* The last non-space char seen so far, or the first char */

	while(*b) {
		if(*b == '_') {
			if(a > out && b > in && *(b-1) == '(' && *(b+1) && !(*(b+1) & 0x80) && *(b+2) == ')') {
				/* Detected CJK style shortcut (Bug 875311) */
				a = a0;	/* undo the left parenthesis */
				b += 3;	/* and skip the whole mess */
			} else if(*(b+1) == '_') {
				*(a++) = '_';
				b += 2;
				a0 = a;
			} else {
				b++;
			}
		/* We don't want to corrupt the middle of UTF-8 characters */
		} else if (!(*b & 0x80)) {	/* other 1-byte char */
			if (*b != ' ')
				a0 = a;
			*(a++) = *(b++);
		} else {
			/* Multibyte utf8 char, don't look for _ inside these */
			int n = 0;
			int i;
			if ((*b & 0xe0) == 0xc0) {
				n = 2;
			} else if ((*b & 0xf0) == 0xe0) {
				n = 3;
			} else if ((*b & 0xf8) == 0xf0) {
				n = 4;
			} else if ((*b & 0xfc) == 0xf8) {
				n = 5;
			} else if ((*b & 0xfe) == 0xfc) {
				n = 6;
			} else {		/* Illegal utf8 */
				n = 1;
			}
			a0 = a; /* unless we want to delete CJK spaces too */
			for (i = 0; i < n && *b; i += 1) {
				*(a++) = *(b++);
			}
		}
	}
	*a = '\0';

	return out;
}

/* this is almost identical to purple_url_encode (hence purple_url_decode
 * being used above), but we want to keep certain characters unescaped
 * for compat reasons */
const char *
purple_escape_filename(const char *str)
{
	const char *iter;
	static char buf[BUF_LEN];
	char utf_char[6];
	guint i, j = 0;

	g_return_val_if_fail(str != NULL, NULL);
	g_return_val_if_fail(g_utf8_validate(str, -1, NULL), NULL);

	iter = str;
	for (; *iter && j < (BUF_LEN - 1) ; iter = g_utf8_next_char(iter)) {
		gunichar c = g_utf8_get_char(iter);
		/* If the character is an ASCII character and is alphanumeric,
		 * or one of the specified values, no need to escape */
		if (c < 128 && (g_ascii_isalnum(c) || c == '@' || c == '-' ||
				c == '_' || c == '.' || c == '#')) {
			buf[j++] = c;
		} else {
			int bytes = g_unichar_to_utf8(c, utf_char);
			for (i = 0; (int)i < bytes; i++) {
				if (j > (BUF_LEN - 4))
					break;
				if (i >= sizeof(utf_char)) {
					g_warn_if_reached();
					break;
				}
				sprintf(buf + j, "%%%02x", utf_char[i] & 0xff);
				j += 3;
			}
		}
	}
#ifdef _WIN32
	/* File/Directory names in windows cannot end in periods/spaces.
	 * http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
	 */
	while (j > 0 && (buf[j - 1] == '.' || buf[j - 1] == ' '))
		j--;
#endif
	buf[j] = '\0';

	return buf;
}

void purple_util_set_current_song(const char *title, const char *artist, const char *album)
{
	GListModel *manager_model = NULL;
	guint n_items = 0;

	manager_model = purple_account_manager_get_default_as_model();
	n_items = g_list_model_get_n_items(manager_model);
	for(guint index = 0; index < n_items; index++) {
		PurpleAccount *account = g_list_model_get_item(manager_model, index);
		PurplePresence *presence;
		PurpleStatus *tune;

		if (!purple_account_get_enabled(account)) {
			g_object_unref(account);
			continue;
		}

		presence = purple_account_get_presence(account);
		tune = purple_presence_get_status(presence, "tune");
		if (!tune) {
			g_object_unref(account);
			continue;
		}
		if (title) {
			GHashTable *attributes = g_hash_table_new(g_str_hash, g_str_equal);

			g_hash_table_insert(attributes, PURPLE_TUNE_TITLE,
			                    (gpointer)title);
			g_hash_table_insert(attributes, PURPLE_TUNE_ARTIST,
			                    (gpointer)artist);
			g_hash_table_insert(attributes, PURPLE_TUNE_TITLE,
			                    (gpointer)album);

			purple_status_set_active_with_attributes(tune, TRUE, attributes);

			g_hash_table_destroy(attributes);
		} else {
			purple_status_set_active(tune, FALSE);
		}

		g_object_unref(account);
	}
}

char *
purple_util_format_song_info(const char *title, const char *artist,
                             const char *album)
{
	GString *string;
	char *esc;

	if (!title || !*title) {
		return NULL;
	}

	esc = g_markup_escape_text(title, -1);
	string = g_string_new("");
	g_string_append_printf(string, "%s", esc);
	g_free(esc);

	if (artist && *artist) {
		esc = g_markup_escape_text(artist, -1);
		g_string_append_printf(string, _(" - %s"), esc);
		g_free(esc);
	}

	if (album && *album) {
		esc = g_markup_escape_text(album, -1);
		g_string_append_printf(string, _(" (%s)"), esc);
		g_free(esc);
	}

	return g_string_free(string, FALSE);
}

GValue *
purple_value_new(GType type)
{
	GValue *ret;

	g_return_val_if_fail(type != G_TYPE_NONE, NULL);

	ret = g_new0(GValue, 1);
	g_value_init(ret, type);

	return ret;
}

GValue *
purple_value_dup(GValue *value)
{
	GValue *ret;

	g_return_val_if_fail(value != NULL, NULL);

	ret = g_new0(GValue, 1);
	g_value_init(ret, G_VALUE_TYPE(value));
	g_value_copy(value, ret);

	return ret;
}

void
purple_value_free(GValue *value)
{
	g_return_if_fail(value != NULL);

	g_value_unset(value);
	g_free(value);
}
