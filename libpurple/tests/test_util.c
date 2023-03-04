/*
 * purple
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
 *
 */
#include <glib.h>

#include <purple.h>

/******************************************************************************
 * filename escape tests
 *****************************************************************************/
static void
test_util_filename_escape(void) {
	g_assert_cmpstr("foo", ==, purple_escape_filename("foo"));
	g_assert_cmpstr("@oo", ==, purple_escape_filename("@oo"));
	g_assert_cmpstr("#oo", ==, purple_escape_filename("#oo"));
	g_assert_cmpstr("-oo", ==, purple_escape_filename("-oo"));
	g_assert_cmpstr("_oo", ==, purple_escape_filename("_oo"));
	g_assert_cmpstr(".oo", ==, purple_escape_filename(".oo"));
	g_assert_cmpstr("%25oo", ==, purple_escape_filename("%oo"));
	g_assert_cmpstr("%21oo", ==, purple_escape_filename("!oo"));
}

/******************************************************************************
 * text_strip tests
 *****************************************************************************/
static void
test_util_text_strip_mnemonic(void) {
	g_assert_cmpstr("", ==, purple_text_strip_mnemonic(""));
	g_assert_cmpstr("foo", ==, purple_text_strip_mnemonic("foo"));
	g_assert_cmpstr("foo", ==, purple_text_strip_mnemonic("_foo"));

}

/******************************************************************************
 * email tests
 *****************************************************************************/
/*
 * Many of the valid and invalid email addresses lised below are from
 * http://fightingforalostcause.net/misc/2006/compare-email-regex.php
 */
const gchar *valid_emails[] = {
	"purple-devel@lists.sf.net",
	"l3tt3rsAndNumb3rs@domain.com",
	"has-dash@domain.com",
	"hasApostrophe.o'leary@domain.org",
	"uncommonTLD@domain.museum",
	"uncommonTLD@domain.travel",
	"uncommonTLD@domain.mobi",
	"countryCodeTLD@domain.uk",
	"countryCodeTLD@domain.rw",
	"lettersInDomain@911.com",
	"underscore_inLocal@domain.net",
	"IPInsteadOfDomain@127.0.0.1",
	/* "IPAndPort@127.0.0.1:25", */
	"subdomain@sub.domain.com",
	"local@dash-inDomain.com",
	"dot.inLocal@foo.com",
	"a@singleLetterLocal.org",
	"singleLetterDomain@x.org",
	"&*=?^+{}'~@validCharsInLocal.net",
	"foor@bar.newTLD",
	"HenryTheGreatWhiteCricket@live.ca",
	"HenryThe__WhiteCricket@hotmail.com"
};

static void
test_util_email_is_valid(void) {
	size_t i;

	for (i = 0; i < G_N_ELEMENTS(valid_emails); i++)
		g_assert_true(purple_email_is_valid(valid_emails[i]));
}

const gchar *invalid_emails[] = {
	"purple-devel@@lists.sf.net",
	"purple@devel@lists.sf.net",
	"purple-devel@list..sf.net",
	"purple-devel",
	"purple-devel@",
	"@lists.sf.net",
	"totally bogus",
	"missingDomain@.com",
	"@missingLocal.org",
	"missingatSign.net",
	"missingDot@com",
	"two@@signs.com",
	"colonButNoPort@127.0.0.1:",
	"",
	/* "someone-else@127.0.0.1.26", */
	".localStartsWithDot@domain.com",
	/* "localEndsWithDot.@domain.com", */ /* I don't think this is invalid -- Stu */
	/* "two..consecutiveDots@domain.com", */ /* I don't think this is invalid -- Stu */
	"domainStartsWithDash@-domain.com",
	"domainEndsWithDash@domain-.com",
	/* "numbersInTLD@domain.c0m", */
	/* "missingTLD@domain.", */ /* This certainly isn't invalid -- Stu */
	"! \"#$%(),/;<>[]`|@invalidCharsInLocal.org",
	"invalidCharsInDomain@! \"#$%(),/;<>_[]`|.org",
	/* "local@SecondLevelDomainNamesAreInvalidIfTheyAreLongerThan64Charactersss.org" */
};

static void
test_util_email_is_invalid(void) {
	size_t i;

	for (i = 0; i < G_N_ELEMENTS(invalid_emails); i++)
		g_assert_false(purple_email_is_valid(invalid_emails[i]));
}

/******************************************************************************
 * UTF8 tests
 *****************************************************************************/
typedef struct {
	gchar *input;
	gchar *output;
} UTF8TestData;

static void
test_util_utf8_strip_unprintables(void) {
	gint i;
	UTF8TestData data[] = {
		{
			/* \t, \n, \r, space */
			"ab \tcd\nef\r   ",
			"ab \tcd\nef\r   ",
		}, {
			/* ASCII control characters (stripped) */
			"\x01\x02\x03\x04\x05\x06\x07\x08\x0B\x0C\x0E\x0F\x10 aaaa "
			"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
			" aaaa ",
		}, {
			/* Basic ASCII */
			"Foobar",
			"Foobar",
		}, {
			/* 0xE000 - 0xFFFD (UTF-8 encoded) */
			/* U+F1F7 */
			"aaaa\xef\x87\xb7",
			"aaaa\xef\x87\xb7",
		}, {
			/* U+FEFF (should not be stripped) */
			"aaaa\xef\xbb\xbf",
			"aaaa\xef\xbb\xbf",
		}, {
			/* U+FFFE (should be stripped) */
			"aaaa\xef\xbf\xbe",
			"aaaa",
		}, {
			NULL,
			NULL,
		}
	};

	for(i = 0; ; i++) {
		gchar *result = purple_utf8_strip_unprintables(data[i].input);

		g_assert_cmpstr(data[i].output, ==, result);

		g_free(result);

		/* NULL as input is a valid test, but it's the last test, so we break
		 * after it.
		 */
		if(data[i].input == NULL)
			break;
	}
}

/******************************************************************************
 * strdup_withhtml tests
 *****************************************************************************/
static void
test_util_strdup_withhtml(void) {
	gchar *result = purple_strdup_withhtml("hi\r\nthere\n");

	g_assert_cmpstr("hi<BR>there<BR>", ==, result);

	g_free(result);
}

/******************************************************************************
 * MANE
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/util/filename/escape",
	                test_util_filename_escape);

	g_test_add_func("/util/mnemonic/strip",
	                test_util_text_strip_mnemonic);

	g_test_add_func("/util/email/is valid",
	                test_util_email_is_valid);
	g_test_add_func("/util/email/is invalid",
	                test_util_email_is_invalid);

	g_test_add_func("/util/utf8/strip unprintables",
	                test_util_utf8_strip_unprintables);

	g_test_add_func("/util/test_strdup_withhtml",
	                test_util_strdup_withhtml);

	return g_test_run();
}
