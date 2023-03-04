/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include <glib.h>

#include <purple.h>

#include "../purpleircv3parser.h"

#define TEST_IRCV3_PARSER_DOMAIN (g_quark_from_static_string("test-ircv3-parser"))

typedef struct {
	GHashTable *tags;
	gchar *source;
	gchar *command;
	guint n_params;
	const gchar * const params[16];
} TestPurpleIRCv3ParserData;

/******************************************************************************
 * Handlers
 *****************************************************************************/
static gboolean
test_purple_ircv3_test_handler(GHashTable *tags, const gchar *source,
                               const gchar *command, guint n_params,
                               GStrv params, G_GNUC_UNUSED GError **error,
                               gpointer data)
{
	TestPurpleIRCv3ParserData *d = data;
	GHashTableIter iter;

	/* Make sure we have an expected tags hash table before checking them. */
	if(d->tags != NULL) {
		gpointer expected_key;
		gpointer expected_value;
		guint actual_size;
		guint expected_size;

		/* Make sure the tag hash tables have the same size. */
		expected_size = g_hash_table_size(d->tags);
		actual_size = g_hash_table_size(tags);
		g_assert_cmpuint(actual_size, ==, expected_size);

		/* Since the tables have the same size, we can walk through the expected
		 * table and use it to verify the actual table.
		 */
		g_hash_table_iter_init(&iter, d->tags);
		while(g_hash_table_iter_next(&iter, &expected_key, &expected_value)) {
			gpointer actual_value = NULL;
			gboolean found = FALSE;

			found = g_hash_table_lookup_extended(tags, expected_key, NULL,
			                                     &actual_value);
			g_assert_true(found);
			g_assert_cmpstr(actual_value, ==, expected_value);
		}
	}

	/* If the expected strings values are NULL, set them to empty string as
	 * that's what g_match_info_get_named will return for them.
	 */
	if(d->source == NULL) {
		d->source = "";
	}

	if(d->command == NULL) {
		d->command = "";
	}

	/* Walk through the params checking against the expected values. */
	if(d->n_params > 0) {
		g_assert_cmpuint(n_params, ==, d->n_params);

		for(guint i = 0; i < d->n_params; i++) {
			g_assert_cmpstr(params[i], ==, d->params[i]);
		}
	}

	/* Validate all the string parameters. */
	g_assert_cmpstr(source, ==, d->source);
	g_assert_cmpstr(command, ==, d->command);

	/* Cleanup everything the caller allocated. */
	g_clear_pointer(&d->tags, g_hash_table_destroy);

	/* Return the return value the caller asked for. */
	return TRUE;
}

static gboolean
test_purple_ircv3_test_handler_error(G_GNUC_UNUSED GHashTable *tags,
                                     G_GNUC_UNUSED const gchar *source,
                                     G_GNUC_UNUSED const gchar *command,
                                     G_GNUC_UNUSED guint n_params,
                                     G_GNUC_UNUSED GStrv params,
                                     GError **error,
                                     G_GNUC_UNUSED gpointer data)
{
	g_set_error(error, TEST_IRCV3_PARSER_DOMAIN, 0, "test error");

	return FALSE;
}

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
test_purple_ircv3_parser(const gchar *source, TestPurpleIRCv3ParserData *d) {
	PurpleIRCv3Parser *parser = purple_ircv3_parser_new();
	GError *error = NULL;
	gboolean result = FALSE;

	purple_ircv3_parser_set_fallback_handler(parser,
	                                         test_purple_ircv3_test_handler);

	result = purple_ircv3_parser_parse(parser, source, &error, d);

	g_assert_no_error(error);
	g_assert_true(result);

	g_clear_object(&parser);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_ircv3_parser_propagates_errors(void) {
	PurpleIRCv3Parser *parser = purple_ircv3_parser_new();
	GError *error = NULL;
	gboolean result = FALSE;

	purple_ircv3_parser_set_fallback_handler(parser,
	                                         test_purple_ircv3_test_handler_error);

	result = purple_ircv3_parser_parse(parser, "COMMAND", &error, NULL);
	g_assert_error(error, TEST_IRCV3_PARSER_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_false(result);

	g_clear_object(&parser);
}

static void
test_purple_ircv3_parser_simple(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", "asdf"},
	};

	test_purple_ircv3_parser("foo bar baz asdf", &data);
}

static void
test_purple_ircv3_parser_with_source(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy",
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", "asdf"},
	};

	test_purple_ircv3_parser(":coolguy foo bar baz asdf", &data);
}

static void
test_purple_ircv3_parser_with_trailing(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", "asdf quux"},
	};

	test_purple_ircv3_parser("foo bar baz :asdf quux", &data);
}

static void
test_purple_ircv3_parser_with_empty_trailing(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", ""},
	};

	test_purple_ircv3_parser("foo bar baz :", &data);
}

static void
test_purple_ircv3_parser_with_trailing_starting_colon(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", ":asdf"},
	};

	test_purple_ircv3_parser("foo bar baz ::asdf", &data);
}

static void
test_purple_ircv3_parser_with_source_and_trailing(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy",
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", "asdf quux"},
	};

	test_purple_ircv3_parser(":coolguy foo bar baz :asdf quux", &data);
}

static void
test_purple_ircv3_parser_with_source_and_trailing_whitespace(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy",
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", "  asdf quux "},
	};

	test_purple_ircv3_parser(":coolguy foo bar baz :  asdf quux ", &data);
}

static void
test_purple_ircv3_parser_with_source_and_trailing_colon(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy",
		.command = "PRIVMSG",
		.n_params = 2,
		.params = {"bar", "lol :) "},
	};

	test_purple_ircv3_parser(":coolguy PRIVMSG bar :lol :) ", &data);
}

static void
test_purple_ircv3_parser_with_source_and_empty_trailing(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy",
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", ""},
	};

	test_purple_ircv3_parser(":coolguy foo bar baz :", &data);
}

static void
test_purple_ircv3_parser_with_source_and_trailing_only_whitespace(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy",
		.command = "foo",
		.n_params = 3,
		.params = {"bar", "baz", "  "},
	};

	test_purple_ircv3_parser(":coolguy foo bar baz :  ", &data);
}

static void
test_purple_ircv3_parser_with_tags(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "foo",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "a", "b");
	g_hash_table_insert(data.tags, "c", "32");
	g_hash_table_insert(data.tags, "k", "");
	g_hash_table_insert(data.tags, "rt", "ql7");

	test_purple_ircv3_parser("@a=b;c=32;k;rt=ql7 foo", &data);
}

static void
test_purple_ircv3_parser_with_escaped_tags(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "foo",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "a", "b\\and\nk");
	g_hash_table_insert(data.tags, "c", "72 45");
	g_hash_table_insert(data.tags, "d", "gh;764");

	test_purple_ircv3_parser("@a=b\\\\and\\nk;c=72\\s45;d=gh\\:764 foo",
	                         &data);
}

static void
test_purple_ircv3_with_tags_and_source(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "quux",
		.command = "ab",
		.n_params = 1,
		.params = {"cd"},
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "c", "");
	g_hash_table_insert(data.tags, "h", "");
	g_hash_table_insert(data.tags, "a", "b");

	test_purple_ircv3_parser("@c;h=;a=b :quux ab cd", &data);
}

static void
test_purple_ircv3_last_param_no_colon(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "src",
		.command = "JOIN",
		.n_params = 1,
		.params = {"#chan"},
	};

	test_purple_ircv3_parser(":src JOIN #chan", &data);
}

static void
test_purple_ircv3_last_param_with_colon(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "src",
		.command = "JOIN",
		.n_params = 1,
		.params = {"#chan"},
	};

	test_purple_ircv3_parser(":src JOIN :#chan", &data);
}

static void
test_purple_ircv3_without_last_param(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "src",
		.command = "AWAY",
	};

	test_purple_ircv3_parser(":src AWAY", &data);
}

static void
test_purple_ircv3_with_last_param(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "src",
		.command = "AWAY",
	};

	test_purple_ircv3_parser(":src AWAY ", &data);
}

static void
test_purple_ircv3_tab_is_not_space(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "cool\tguy",
		.command = "foo",
		.n_params = 2,
		.params = {"bar", "baz"},
	};

	test_purple_ircv3_parser(":cool\tguy foo bar baz", &data);
}

static void
test_purple_ircv3_source_control_characters_1(void) {
	/* Break each string after the hex escape as they are supposed to only be
	 * a single byte, but the c compiler will keep unescaping unless we break
	 * the string.
	 */
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy!ag@net\x03" "5w\x03" "ork.admin",
		.command = "PRIVMSG",
		.n_params = 2,
		.params = {"foo", "bar baz"},
	};
	const gchar *msg = NULL;

	msg = ":coolguy!ag@net\x03" "5w\x03" "ork.admin PRIVMSG foo :bar baz";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_source_control_characters_2(void) {
	/* Break each string after the hex escape as they are supposed to only be
	 * a single byte, but the c compiler will keep unescaping unless we break
	 * the string.
	 */
	TestPurpleIRCv3ParserData data = {
		.source = "coolguy!~ag@n\x02" "et\x03" "05w\x0f" "ork.admin",
		.command = "PRIVMSG",
		.n_params = 2,
		.params = {"foo", "bar baz"},
	};
	const gchar *msg = NULL;

	msg = ":coolguy!~ag@n\x02" "et\x03" "05w\x0f" "ork.admin PRIVMSG foo :bar "
	      "baz";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_everything(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "irc.example.com",
		.command = "COMMAND",
		.n_params = 3,
		.params = {"param1", "param2", "param3 param3"},
	};
	const gchar *msg = NULL;

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "value1");
	g_hash_table_insert(data.tags, "tag2", "");
	g_hash_table_insert(data.tags, "vendor1/tag3", "value2");
	g_hash_table_insert(data.tags, "vendor2/tag4", "");

	msg = "@tag1=value1;tag2;vendor1/tag3=value2;vendor2/tag4= "
	      ":irc.example.com COMMAND param1 param2 :param3 param3";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_everything_but_tags(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "irc.example.com",
		.command = "COMMAND",
		.n_params = 3,
		.params = {"param1", "param2", "param3 param3"},
	};
	const gchar *msg = NULL;

	msg = ":irc.example.com COMMAND param1 param2 :param3 param3";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_everything_but_source(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
		.n_params = 3,
		.params = {"param1", "param2", "param3 param3"},
	};
	const gchar *msg = NULL;

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "value1");
	g_hash_table_insert(data.tags, "tag2", "");
	g_hash_table_insert(data.tags, "vendor1/tag3", "value2");
	g_hash_table_insert(data.tags, "vendor2/tag4", "");

	msg = "@tag1=value1;tag2;vendor1/tag3=value2;vendor2/tag4 "
	      "COMMAND param1 param2 :param3 param3";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_command_only(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};

	test_purple_ircv3_parser("COMMAND", &data);
}

static void
test_purple_ircv3_slashes_are_fun(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "foo", "\\\\;\\s \r\n");

	test_purple_ircv3_parser("@foo=\\\\\\\\\\:\\\\s\\s\\r\\n COMMAND", &data);
}

static void
test_purple_ircv3_unreal_broken_1(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "gravel.mozilla.org",
		.command = "432",
		.n_params = 2,
		.params = {"#momo", "Erroneous Nickname: Illegal characters"},
	};
	const gchar *msg = NULL;

	msg = ":gravel.mozilla.org 432  #momo :Erroneous Nickname: Illegal "
	      "characters";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_unreal_broken_2(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "gravel.mozilla.org",
		.command = "MODE",
		.n_params = 2,
		.params = {"#tckk", "+n"},
	};

	test_purple_ircv3_parser(":gravel.mozilla.org MODE #tckk +n ", &data);
}

static void
test_purple_ircv3_unreal_broken_3(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "services.esper.net",
		.command = "MODE",
		.n_params = 3,
		.params = {"#foo-bar", "+o", "foobar"},
	};

	test_purple_ircv3_parser(":services.esper.net MODE #foo-bar +o foobar  ",
	                         &data);
}

static void
test_purple_ircv3_tag_escape_char_at_a_time(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "value\\ntest");

	test_purple_ircv3_parser("@tag1=value\\\\ntest COMMAND", &data);
}

static void
test_purple_ircv3_tag_drop_unnecessary_escapes(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "value1");

	test_purple_ircv3_parser("@tag1=value\\1 COMMAND", &data);
}

static void
test_purple_ircv3_tag_drop_trailing_slash(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "value1");

	test_purple_ircv3_parser("@tag1=value1\\ COMMAND", &data);
}

static void
test_purple_ircv3_duplicate_tags(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "5");
	g_hash_table_insert(data.tags, "tag2", "3");
	g_hash_table_insert(data.tags, "tag3", "4");

	test_purple_ircv3_parser("@tag1=1;tag2=3;tag3=4;tag1=5 COMMAND", &data);
}

static void
test_purple_ircv3_vendor_tags_are_namespaced(void) {
	TestPurpleIRCv3ParserData data = {
		.command = "COMMAND",
	};
	const gchar *msg = NULL;

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "tag1", "5");
	g_hash_table_insert(data.tags, "tag2", "3");
	g_hash_table_insert(data.tags, "tag3", "4");
	g_hash_table_insert(data.tags, "vendor/tag2", "8");

	msg = "@tag1=1;tag2=3;tag3=4;tag1=5;vendor/tag2=8 COMMAND";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_special_mode_1(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "SomeOp",
		.command = "MODE",
		.n_params = 2,
		.params = {"#channel", "+i"},
	};

	test_purple_ircv3_parser(":SomeOp MODE #channel :+i", &data);
}

static void
test_purple_ircv3_special_mode_2(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "SomeOp",
		.command = "MODE",
		.n_params = 4,
		.params = {"#channel", "+oo", "SomeUser", "AnotherUser"},
	};

	test_purple_ircv3_parser(":SomeOp MODE #channel +oo SomeUser :AnotherUser",
	                         &data);
}

/******************************************************************************
 * Message tags examples
 *****************************************************************************/
static void
test_purple_ircv3_parser_message_tags_none(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "nick!ident@host.com",
		.command = "PRIVMSG",
		.n_params = 2,
		.params = {"me", "Hello"},
	};
	const char *msg = NULL;

	msg = ":nick!ident@host.com PRIVMSG me :Hello";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_parser_message_tags_3_tags(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "nick!ident@host.com",
		.command = "PRIVMSG",
		.n_params = 2,
		.params = {"me", "Hello"},
	};
	const char *msg = NULL;

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "aaa", "bbb");
	g_hash_table_insert(data.tags, "ccc", "");
	g_hash_table_insert(data.tags, "example.com/ddd", "eee");

	msg = "@aaa=bbb;ccc;example.com/ddd=eee :nick!ident@host.com PRIVMSG me "
	      ":Hello";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_parser_message_tags_client_only(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "url_bot!bot@example.com",
		.command = "PRIVMSG",
		.n_params = 2,
		.params = {"#channel", "Example.com: A News Story"},
	};
	const char *msg = NULL;

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "+icon", "https://example.com/favicon.png");

	msg = "@+icon=https://example.com/favicon.png :url_bot!bot@example.com "
	      "PRIVMSG #channel :Example.com: A News Story";

	test_purple_ircv3_parser(msg, &data);
}

static void
test_purple_ircv3_parser_message_tags_labeled_response(void) {
	TestPurpleIRCv3ParserData data = {
		.source = "nick!user@example.com",
		.command = "TAGMSG",
		.n_params = 1,
		.params = {"#channel"},
	};
	const char *msg = NULL;

	data.tags = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(data.tags, "label", "123");
	g_hash_table_insert(data.tags, "msgid", "abc");
	g_hash_table_insert(data.tags, "+example-client-tag", "example-value");

	msg = "@label=123;msgid=abc;+example-client-tag=example-value "
	      ":nick!user@example.com TAGMSG #channel";

	test_purple_ircv3_parser(msg, &data);

}
/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	/* Make sure an error in the handler is propagated back up to the calling
	 * function.
	 */
	g_test_add_func("/ircv3/parser/propagates-errors",
	                test_purple_ircv3_parser_propagates_errors);

	/* These tests are based on the msg-split tests from
	 * https://github.com/ircdocs/parser-tests/blob/master/tests/msg-split.yaml
	 */
	g_test_add_func("/ircv3/parser/simple",
	                test_purple_ircv3_parser_simple);
	g_test_add_func("/ircv3/parser/with-source",
	                test_purple_ircv3_parser_with_source);
	g_test_add_func("/ircv3/parser/with-trailing",
	                test_purple_ircv3_parser_with_trailing);
	g_test_add_func("/ircv3/parser/with-empty-trailing",
	                test_purple_ircv3_parser_with_empty_trailing);
	g_test_add_func("/ircv3/parser/with-trailing-starting-colon",
	                test_purple_ircv3_parser_with_trailing_starting_colon);
	g_test_add_func("/ircv3/parser/with-source-and-trailing",
	                test_purple_ircv3_parser_with_source_and_trailing);
	g_test_add_func("/ircv3/parser/with-source-and-trailing-whitespace",
	                test_purple_ircv3_parser_with_source_and_trailing_whitespace);
	g_test_add_func("/ircv3/parser/with-source-and-trailing-colon",
	                test_purple_ircv3_parser_with_source_and_trailing_colon);
	g_test_add_func("/ircv3/parser/with-source-and-empty-trailing",
	                test_purple_ircv3_parser_with_source_and_empty_trailing);
	g_test_add_func("/ircv3/parser/with-source-and-trailing-only-whitespace",
	                test_purple_ircv3_parser_with_source_and_trailing_only_whitespace);

	g_test_add_func("/ircv3/parser/with-tags",
	                test_purple_ircv3_parser_with_tags);
	g_test_add_func("/ircv3/parser/with-escaped-tags",
	                test_purple_ircv3_parser_with_escaped_tags);
	g_test_add_func("/ircv3/parser/with-tags-and-source",
	                test_purple_ircv3_with_tags_and_source);

	g_test_add_func("/ircv3/parser/last-param-no-colon",
	                test_purple_ircv3_last_param_no_colon);
	g_test_add_func("/ircv3/parser/last-param-with-colon",
	                test_purple_ircv3_last_param_with_colon);

	g_test_add_func("/ircv3/parser/without-last-param",
	                test_purple_ircv3_without_last_param);
	g_test_add_func("/ircv3/parser/with-last-parsm",
	                test_purple_ircv3_with_last_param);

	g_test_add_func("/ircv3/parser/tab-is-not-space",
	                test_purple_ircv3_tab_is_not_space);

	g_test_add_func("/ircv3/parser/source_control_characters_1",
	                test_purple_ircv3_source_control_characters_1);
	g_test_add_func("/ircv3/parser/source_control_characters_2",
	                test_purple_ircv3_source_control_characters_2);

	g_test_add_func("/ircv3/parser/everything",
	                test_purple_ircv3_everything);
	g_test_add_func("/ircv3/parser/everything-but-tags",
	                test_purple_ircv3_everything_but_tags);
	g_test_add_func("/ircv3/parser/everything-but-source",
	                test_purple_ircv3_everything_but_source);

	g_test_add_func("/ircv3/parser/command-only",
	                test_purple_ircv3_command_only);

	g_test_add_func("/ircv3/parser/slashes-are-fun",
	                test_purple_ircv3_slashes_are_fun);

	g_test_add_func("/ircv3/parser/unreal-broken-1",
	                test_purple_ircv3_unreal_broken_1);
	g_test_add_func("/ircv3/parser/unreal-broken-2",
	                test_purple_ircv3_unreal_broken_2);
	g_test_add_func("/ircv3/parser/unreal-broken-3",
	                test_purple_ircv3_unreal_broken_3);

	g_test_add_func("/ircv3/parser/tag-escape-char-at-a-time",
	                test_purple_ircv3_tag_escape_char_at_a_time);
	g_test_add_func("/ircv3/parser/tag-drop-unnecessary-escapes",
	                test_purple_ircv3_tag_drop_unnecessary_escapes);
	g_test_add_func("/ircv3/parser/tag-drop-trailing-slash",
	                test_purple_ircv3_tag_drop_trailing_slash);

	g_test_add_func("/ircv3/parser/duplicate-tags",
	                test_purple_ircv3_duplicate_tags);
	g_test_add_func("/ircv3/parser/vendor-tags-are-namespaced",
	                test_purple_ircv3_vendor_tags_are_namespaced);

	g_test_add_func("/ircv3/parser/special-mode-1",
	                test_purple_ircv3_special_mode_1);
	g_test_add_func("/ircv3/parser/special-mode-2",
	                test_purple_ircv3_special_mode_2);

	/* These tests are the examples from the message-tags specification,
	 * https://ircv3.net/specs/extensions/message-tags.html.
	 */
	g_test_add_func("/ircv3/parser/message-tags/none",
	                test_purple_ircv3_parser_message_tags_none);
	g_test_add_func("/ircv3/parser/message-tags/3-tags",
	                test_purple_ircv3_parser_message_tags_3_tags);
	g_test_add_func("/ircv3/parser/message-tags/client-only",
	                test_purple_ircv3_parser_message_tags_client_only);
	g_test_add_func("/ircv3/parser/message-tags/labeled-message",
	                test_purple_ircv3_parser_message_tags_labeled_response);

	return g_test_run();
}
