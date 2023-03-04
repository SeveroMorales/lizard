/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <purple.h>

typedef struct {
	gchar *markup;
	gchar *xhtml;
	gchar *plaintext;
} MarkupTestData;

static void
test_purple_markup_html_to_xhtml(void) {
	gint i;
	MarkupTestData data[] = {
		{
			"<a>",
			"<a href=\"\"></a>",
			"",
		}, {
			"<A href='URL'>ABOUT</a>",
			"<a href=\"URL\">ABOUT</a>",
			"ABOUT <URL>",
		}, {
			"<a href='URL'>URL</a>",
			"<a href=\"URL\">URL</a>",
			"URL",
		}, {
			"<a href='mailto:mail'>mail</a>",
			"<a href=\"mailto:mail\">mail</a>",
			"mail",
		}, {
			"<A href='\"U&apos;R&L'>ABOUT</a>",
			"<a href=\"&quot;U&apos;R&amp;L\">ABOUT</a>",
			"ABOUT <\"U'R&L>",
		}, {
			"<img src='SRC' alt='ALT'/>",
			"<img src='SRC' alt='ALT' />",
			"ALT",
		}, {
			"<img src=\"'S&apos;R&C\" alt=\"'A&apos;L&T\"/>",
			"<img src='&apos;S&apos;R&amp;C' alt='&apos;A&apos;L&amp;T' />",
			"'A'L&T",
		}, {
			"<unknown>",
			"&lt;unknown>",
			"<unknown>",
		}, {
			"&eacute;&amp;",
			"&eacute;&amp;",
			"&eacute;&",
		}, {
			"<h1>A<h2>B</h2>C</h1>",
			"<h1>A<h2>B</h2>C</h1>",
			"ABC",
		}, {
			"<h1><h2><h3><h4>",
			"<h1><h2><h3><h4></h4></h3></h2></h1>",
			"",
		}, {
			"<italic/>",
			"<em/>",
			"",
		}, {
			"</",
			"&lt;/",
			"</",
		}, {
			"</div>",
			"",
			"",
		}, {
			"<hr/>",
			"<br/>",
			"\n",
		}, {
			"<hr>",
			"<br/>",
			"\n",
		}, {
			"<br />",
			"<br/>",
			"\n",
		}, {
			"<br>INSIDE</br>",
			"<br/>INSIDE",
			"\nINSIDE",
		}, {
			"<div></div>",
			"<div></div>",
			"",
		}, {
			"<div/>",
			"<div/>",
			"",
		}, {
			"<div attr='\"&<>'/>",
			"<div attr='&quot;&amp;&lt;&gt;'/>",
			"",
		}, {
			"<div attr=\"'\"/>",
			"<div attr=\"&apos;\"/>",
			"",
		}, {
			"<div/> < <div/>",
			"<div/> &lt; <div/>",
			" < ",
		}, {
			"<div>x</div>",
			"<div>x</div>",
			"x",
		}, {
			"<b>x</b>",
			"<span style='font-weight: bold;'>x</span>",
			"x",
		}, {
			"<bold>x</bold>",
			"<span style='font-weight: bold;'>x</span>",
			"x",
		}, {
			"<strong>x</strong>",
			"<span style='font-weight: bold;'>x</span>",
			"x",
		}, {
			"<u>x</u>",
			"<span style='text-decoration: underline;'>x</span>",
			"x",
		}, {
			"<underline>x</underline>",
			"<span style='text-decoration: underline;'>x</span>",
			"x",
		}, {
			"<s>x</s>",
			"<span style='text-decoration: line-through;'>x</span>",
			"x",
		}, {
			"<strike>x</strike>",
			"<span style='text-decoration: line-through;'>x</span>",
			"x",
		}, {
			"<sub>x</sub>",
			"<span style='vertical-align:sub;'>x</span>",
			"x",
		}, {
			"<sup>x</sup>",
			"<span style='vertical-align:super;'>x</span>",
			"x",
		}, {
			"<FONT>x</FONT>",
			"x",
			"x",
		}, {
			"<font face=\"'Times&gt;New & Roman'\">x</font>",
			"<span style='font-family: \"Times&gt;New &amp; Roman\";'>x</span>",
			"x",
		}, {
			"<font back=\"'color&gt;blue&red'\">x</font>",
			"<span style='background: \"color&gt;blue&amp;red\";'>x</span>",
			"x",
		}, {
			"<font color=\"'color&gt;blue&red'\">x</font>",
			"<span style='color: \"color&gt;blue&amp;red\";'>x</span>",
			"x",
		}, {
			"<font size=1>x</font>",
			"<span style='font-size: xx-small;'>x</span>",
			"x",
		}, {
			"<font size=432>x</font>",
			"<span style='font-size: medium;'>x</span>",
			"x",
		}, {
			"<!--COMMENT-->",
			"<!--COMMENT-->",
			"COMMENT-->",
		}, {
			"<br  />",
			"&lt;br  />",
			"<br  />",
		}, {
			"<hr  />",
			"&lt;hr  />",
			"<hr  />"
		}, {
			NULL, NULL, NULL,
		}
	};

	for(i = 0; data[i].markup; i++) {
		gchar *xhtml = NULL, *plaintext = NULL;

		purple_markup_html_to_xhtml(data[i].markup, &xhtml, &plaintext);

		g_assert_cmpstr(data[i].xhtml, ==, xhtml);
		g_free(xhtml);

		g_assert_cmpstr(data[i].plaintext, ==, plaintext);
		g_free(plaintext);
	}
}

static void
test_purple_markup_strip_html(void) {
	MarkupTestData data[] = {
		{
			.markup = "",
			.plaintext = "",
		}, {
			.markup = "<a href=\"https://example.com/\">https://example.com/</a>",
			.plaintext = "https://example.com/",
		}, {
			.markup = "<a href=\"https://example.com/\">example.com</a>",
			.plaintext = "example.com (https://example.com/)",
		}, {
			.markup = "<script>/* this should be ignored */</script>",
			.plaintext = "",
		}, {
			.markup = "<style>/* this should be ignored */</style>",
			.plaintext = "",
		}, {
			.markup = "<table><tr><td>1</td><td>2</td></tr><tr><td>3</td><td>4</td></tr></table>",
			.plaintext = "1\t2\n3\t4\n",
		}, {
			.markup = "<p>foo</p><p>bar</p><p>baz</p>",
			.plaintext = "foo\nbar\nbaz",
		}, {
			.markup = "<div><p>foo</p><p>bar</p></div>",
			.plaintext = "foo\nbar",
		}, {
			.markup = "<hr>",
			.plaintext = "",
		}, {
			.markup = "<br>",
			.plaintext = "\n",
		}, {
			.markup = NULL,
		}
	};

	for(int i = 0; data[i].markup != NULL; i++) {
		char *plaintext = purple_markup_strip_html(data[i].markup);

		g_assert_cmpstr(plaintext, ==, data[i].plaintext);
		g_free(plaintext);
	}
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/util/markup/html-to-xhtml",
	                test_purple_markup_html_to_xhtml);
	g_test_add_func("/util/markup/strip-html",
	                test_purple_markup_strip_html);

	return g_test_run();
}
