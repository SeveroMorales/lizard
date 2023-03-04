/*
 * Pidgin - Universal Chat Client
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gtkidle.h"

#ifdef HAVE_IOKIT
/* HAVE_UNISTD_H must have a value, see
 * https://forums.developer.apple.com/thread/86887
 */
# ifdef HAVE_UNISTD_H
#  undef HAVE_UNISTD_H
#  define HAVE_UNISTD_H 1
# else
#  define HAVE_UNISTD_H 0
# endif

# include <CoreFoundation/CoreFoundation.h>
# include <IOKit/IOKitLib.h>
#elif defined (_WIN32)
# include "win32/gtkwin32dep.h"
#endif

#include <purple.h>

struct _PidginIdle {
	GObject parent;
};

/******************************************************************************
 * PurpleIdleUi Implementation
 *****************************************************************************/
#ifdef _WIN32
static time_t
pidgin_idle_get_idle_time(G_GNUC_UNUSED PurpleIdleUi *ui) {
	return (GetTickCount() - winpidgin_get_lastactive()) / 1000;
}
#endif /* _WIN32 */

#ifdef HAVE_IOKIT
static time_t
pidgin_idle_get_idle_time(PurpleIdleUi *ui) {
	/* Query the IOKit API */
	double idleSeconds = -1;
	io_iterator_t iter = 0;
	if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOHIDSystem"), &iter) == KERN_SUCCESS) {
		io_registry_entry_t entry = IOIteratorNext(iter);
		if (entry) {
			CFMutableDictionaryRef dict = NULL;
			kern_return_t status;
			status = IORegistryEntryCreateCFProperties(entry, &dict, kCFAllocatorDefault, 0);
			if (status == KERN_SUCCESS) {
				CFNumberRef obj = CFDictionaryGetValue(dict, CFSTR("HIDIdleTime"));
				if (obj) {
					int64_t nanoseconds = 0;
					if (CFNumberGetValue(obj, kCFNumberSInt64Type, &nanoseconds)) {
						idleSeconds = (double) nanoseconds / NSEC_PER_SEC;
					}
				}
				CFRelease(dict);
			}
			IOObjectRelease(entry);
		}
		IOObjectRelease(iter);
	}
	return idleSeconds;
}
#endif /* HAVE_IOKIT */

#if !defined(_WIN32) && !defined(HAVE_IOKIT)
typedef struct {
	gchar *bus_name;
	gchar *object_path;
	gchar *iface_name;
} PidginDBusScreenSaverInfo;

static const PidginDBusScreenSaverInfo screensavers[] = {
	{
		"org.freedesktop.ScreenSaver",
		"/org/freedesktop/ScreenSaver",
		"org.freedesktop.ScreenSaver"
	}, {
		"org.gnome.ScreenSaver",
		"/org/gnome/ScreenSaver",
		"org.gnome.ScreenSaver"
	}, {
		"org.kde.ScreenSaver",
		"/org/kde/ScreenSaver",
		"org.kde.ScreenSaver"
	},
};

static time_t
pidgin_idle_get_idle_time(G_GNUC_UNUSED PurpleIdleUi *ui) {
	static guint idx = 0;
	GApplication *app;
	GDBusConnection *conn;
	GVariant *reply = NULL;
	guint32 active_time = 0;
	GError *error = NULL;

	app = g_application_get_default();

	if (app == NULL) {
		purple_debug_error("gtkidle",
				"Unable to retrieve GApplication");
		return 0;
	}

	conn = g_application_get_dbus_connection(app);

	if (conn == NULL) {
		purple_debug_misc("gtkidle",
				"GApplication lacking DBus connection. "
				"Skip checking ScreenSaver interface");
		return 0;
	}

	for (; idx < G_N_ELEMENTS(screensavers); ++idx) {
		const PidginDBusScreenSaverInfo *info = &screensavers[idx];

		reply = g_dbus_connection_call_sync(conn,
				info->bus_name, info->object_path,
				info->iface_name, "GetActiveTime",
				NULL, G_VARIANT_TYPE("(u)"),
				G_DBUS_CALL_FLAGS_NO_AUTO_START, 1000,
				NULL, &error);

		if (reply != NULL) {
			break;
		}

		if (g_error_matches(error, G_DBUS_ERROR,
				G_DBUS_ERROR_NOT_SUPPORTED)) {
			purple_debug_info("gtkidle",
					"Querying idle time on '%s' "
					"unsupported. Trying the next one",
					info->bus_name);
		} else if (g_error_matches(error, G_DBUS_ERROR,
				G_DBUS_ERROR_NAME_HAS_NO_OWNER)) {
			purple_debug_info("gtkidle",
					"Querying idle time on '%s' "
					"not found. Trying the next one",
					info->bus_name);
		} else {
			purple_debug_error("gtkidle",
					"Querying idle time on '%s' "
					"error: %s", info->bus_name,
					error->message);
		}

		g_clear_error(&error);
	}

	if (reply == NULL) {
		purple_debug_info("gtkidle",
				"Failed to query ScreenSaver active time: "
				"No working ScreenSaver interfaces");
		return 0;
	}

	g_variant_get(reply, "(u)", &active_time);
	g_variant_unref(reply);

	return active_time;
}
#endif /* !defined(_WIN32) && !defined(HAVE_IOKIT) */

static void
pidgin_idle_purple_ui_init(PurpleIdleUiInterface *iface) {
	iface->get_idle_time = pidgin_idle_get_idle_time;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(
	PidginIdle,
	pidgin_idle,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE(
		PURPLE_TYPE_IDLE_UI,
		pidgin_idle_purple_ui_init
	)
);

static void
pidgin_idle_init(G_GNUC_UNUSED PidginIdle *idle) {
}

static void
pidgin_idle_class_init(G_GNUC_UNUSED PidginIdleClass *klass) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleIdleUi *
pidgin_idle_new(void) {
	return g_object_new(PIDGIN_TYPE_IDLE, NULL);
}
