/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
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

#include <stdlib.h>

#include <glib/gi18n-lib.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-version.h>

/******************************************************************************
 * Globals
 *****************************************************************************/
GRegex *regex = NULL;

static const gchar *version_pattern = "^(?P<major>\\d+)(\\."
									  "(?P<minor>\\d+)"
									  "(\\.(?P<micro>\\d+)(?P<extra>.*))?)?$";

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
gplugin_version_lazy_init(void)
{
	static gsize init = 0;

	if(g_once_init_enter(&init)) {
		GError *error = NULL;

		regex =
			g_regex_new(version_pattern, G_REGEX_NO_AUTO_CAPTURE, 0, &error);

		if(error) {
			g_warning(
				_("Failed to initialize the version regex: %s"),
				(error->message) ? error->message : _("unknown"));
			g_error_free(error);
		}

		g_once_init_leave(&init, 1);
	}
}

/*< private >
 * gplugin_version_parser:
 * @version: The string version to parse.
 * @major: (out) (nullable): A return gint pointer for the major version.
 * @minor: (out) (nullable): A return gint pointer for the minor version.
 * @micro: (out) (nullable): A return gint pointer for the micro version.
 * @extra: (out) (nullable): A return gchar * pointer for any extra version
 *         info.
 *
 * Attempts to parse a version string into its @major, @minor, @micro, and
 * @extra parts.  If @version doesn't match a semantic version string, @major
 * will be set to %-1, @minor will be set to %0, and @micro will be set to %0.
 */
static void
gplugin_version_parser(
	const gchar *version,
	gint *major,
	gint *minor,
	gint *micro,
	gchar **extra)
{
	GMatchInfo *info = NULL;
	gboolean matches = FALSE;
	gchar *temp = NULL;

	/* initialize everything to our failed state */
	if(major != NULL) {
		*major = -1;
	}

	if(minor != NULL) {
		*minor = 0;
	}

	if(micro != NULL) {
		*micro = 0;
	}

	if(version == NULL) {
		/* if a version was not provided, return our failed values */
		return;
	}

	matches = g_regex_match(regex, version, 0, &info);
	if(!matches) {
		/* If we failed to match the regex, free info and return our failed
		 * values.
		 */
		g_match_info_unref(info);

		return;
	}

	/* grab the major version */
	if(major) {
		temp = g_match_info_fetch_named(info, "major");
		*major = (temp) ? atoi(temp) : 0;
		g_free(temp);
	}

	/* grab the minor version */
	if(minor) {
		temp = g_match_info_fetch_named(info, "minor");
		*minor = (temp) ? atoi(temp) : 0;
		g_free(temp);
	}

	/* grab the micro version */
	if(micro) {
		temp = g_match_info_fetch_named(info, "micro");
		*micro = (temp) ? atoi(temp) : 0;
		g_free(temp);
	}

	/* grab the extra version */
	if(extra) {
		*extra = g_match_info_fetch_named(info, "extra");
	}

	g_match_info_unref(info);
}

/******************************************************************************
 * GPluginVersion API
 *****************************************************************************/

/**
 * GPLUGIN_MAJOR_VERSION:
 *
 * This is the major version number of GPlugin that was compiled against.
 */

/**
 * GPLUGIN_MINOR_VERSION:
 *
 * This is the minor version number of GPlugin that was compiled against.
 */

/**
 * GPLUGIN_MICRO_VERSION:
 *
 * This is the micro version number of GPlugin that was compiled against.
 */

/**
 * GPLUGIN_EXTRA_VERSION:
 *
 * This is the extra version string of GPlugin that was compiled against.
 */

/**
 * GPLUGIN_VERSION:
 *
 * This is the string version number of GPlugin that was compiled against.
 */

/**
 * GPLUGIN_VERSION_CHECK:
 * @major: The major version to compare for.
 * @minor: The minor version to compare for.
 * @micro: The micro version to compare for.
 *
 * Checks the version of the GPlugin library that is being compiled
 * against.
 *
 * Returns: %TRUE if the version of the GPlugin header files
 * is the same as or newer than the passed-in version.
 */

/**
 * gplugin_version_check:
 * @major: The required major version.
 * @minor: The required minor version.
 * @micro: The required micro version.
 *
 * Checks that the GPlugin library in use is compatible with the given version.
 *
 * Generally you would pass in the constants [const@GPlugin.MAJOR_VERSION],
 * [const@GPlugin.MINOR_VERSION], [const@GPlugin.MICRO_VERSION] as the three
 * arguments to this function; that produces a check that the library in use is
 * compatible with the version of GPlugin the application or module was
 * compiled against.
 *
 * Compatibility is defined by two things: first the version of the running
 * library is newer than the version @major.@minor.@micro. Second the running
 * library must be binary compatible with the version @major.@minor.@micro
 * (same major version).
 *
 * Returns: %NULL if the GPlugin library is compatible with the given version,
 *          or a string describing the version mismatch. The returned string
 *          is owned by GPlugin and must not be modified or freed.
 */
const gchar *
gplugin_version_check(guint major, guint minor, guint micro)
{
	if(major > GPLUGIN_MAJOR_VERSION) {
		return "gplugin version too old (major mismatch)";
	}

	/* This will warn until 1.0.0 is released. */
	if(major < GPLUGIN_MAJOR_VERSION) {
		return "gplugin version too new (major mismatch)";
	}

	if(minor > GPLUGIN_MINOR_VERSION) {
		return "gplugin version too old (minor mismatch)";
	}

	if(minor == GPLUGIN_MINOR_VERSION && micro > GPLUGIN_MICRO_VERSION) {
		return "gplugin version too old (micro mismatch)";
	}

	return NULL;
}

/**
 * gplugin_version_compare:
 * @v1: The first version to compare.
 * @v2: The second version to compare.
 *
 * A semantic version checker which ignores any characters after the micro
 * version.
 *
 * Returns: less than 0 if @v1 is less than @v2, 0 if @v1 is equal to @v1, and
 *          greater than 0 if @v1 is greater than @v2.
 */
gint
gplugin_version_compare(const gchar *v1, const gchar *v2)
{
	gint v1_maj = 0, v1_min = 0, v1_mic = 0;
	gint v2_maj = 0, v2_min = 0, v2_mic = 0;
	gint t = 0;

	if(regex == NULL) {
		gplugin_version_lazy_init();
	}

	/* parse v1 */
	gplugin_version_parser(v1, &v1_maj, &v1_min, &v1_mic, NULL);

	/* parse v2 */
	gplugin_version_parser(v2, &v2_maj, &v2_min, &v2_mic, NULL);

	/* now figure out if they match */
	t = v1_maj - v2_maj;
	if(t != 0)
		return t;

	t = v1_min - v2_min;
	if(t != 0)
		return t;

	return v1_mic - v2_mic;
}
