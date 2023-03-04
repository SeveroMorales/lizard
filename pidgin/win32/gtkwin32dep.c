/*
 * Pidgin is the legal property of its developers, whose names are too numerous
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
#include <io.h>
#include <stdlib.h>
#include <stdio.h>

/* winsock2.h needs to be include before windows.h or else it throws a warning
 * saying it needs to be included first. We don't use it directly but it's
 * included indirectly.
 */
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <shellapi.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <purple.h>

#include "resource.h"

#include "gtkwin32dep.h"
#include "gtkblist.h"
#include "gtkconv.h"
#include "pidgindisplaywindow.h"

/*
 *  GLOBALS
 */
HINSTANCE exe_hInstance = 0;
HINSTANCE dll_hInstance = 0;
HWND messagewin_hwnd;

/*
 *  PUBLIC CODE
 */

HINSTANCE winpidgin_exe_hinstance(void) {
	return exe_hInstance;
}

void winpidgin_set_exe_hinstance(HINSTANCE hint)
{
	exe_hInstance = hint;
}

HINSTANCE winpidgin_dll_hinstance(void) {
	return dll_hInstance;
}

#define PIDGIN_WM_FOCUS_REQUEST (WM_APP + 13)
#define PIDGIN_WM_PROTOCOL_HANDLE (WM_APP + 14)

static LRESULT CALLBACK message_window_handler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	if (msg == PIDGIN_WM_FOCUS_REQUEST) {
		GtkWidget *widget = NULL;

		purple_debug_info("winpidgin", "Got external focus request.");

		widget = pidgin_display_window_get_default();
		gtk_window_present(GTK_WINDOW(widget));

		return TRUE;
	} else if (msg == PIDGIN_WM_PROTOCOL_HANDLE) {
		char *proto_msg = (char *) lparam;
		purple_debug_info("winpidgin", "Got protocol handler request: %s\n", proto_msg ? proto_msg : "");
		purple_got_protocol_handler_uri(proto_msg);
		return TRUE;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

static HWND winpidgin_message_window_init(void) {
	HWND win_hwnd;
	WNDCLASSEX wcx;
	LPCTSTR wname;

	wname = TEXT("WinpidginMsgWinCls");

	wcx.cbSize = sizeof(wcx);
	wcx.style = 0;
	wcx.lpfnWndProc = message_window_handler;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = winpidgin_exe_hinstance();
	wcx.hIcon = NULL;
	wcx.hCursor = NULL;
	wcx.hbrBackground = NULL;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = wname;
	wcx.hIconSm = NULL;

	RegisterClassEx(&wcx);

	/* Create the window */
	if(!(win_hwnd = CreateWindow(wname, TEXT("WinpidginMsgWin"), 0, 0, 0, 0, 0,
			NULL, NULL, winpidgin_exe_hinstance(), 0))) {
		purple_debug_error("winpidgin",
			"Unable to create message window.\n");
		return NULL;
	}

	return win_hwnd;
}

void winpidgin_init(void) {
	typedef void (__cdecl* LPFNSETLOGFILE)(const LPCSTR);
	LPFNSETLOGFILE MySetLogFile;
	gchar *exchndl_dll_path;

	if (purple_debug_is_verbose())
		purple_debug_misc("winpidgin", "winpidgin_init start\n");

	exchndl_dll_path = g_build_filename(wpurple_bin_dir(), "exchndl.dll", NULL);
	MySetLogFile = (gpointer)wpurple_find_and_loadproc(exchndl_dll_path, "SetLogFile");
	g_free(exchndl_dll_path);
	exchndl_dll_path = NULL;
	if (MySetLogFile) {
		gchar *debug_dir, *locale_debug_dir;

		debug_dir = g_build_filename(purple_cache_dir(), "pidgin.RPT", NULL);
		locale_debug_dir = g_locale_from_utf8(debug_dir, -1, NULL, NULL, NULL);

		purple_debug_info("winpidgin", "Setting exchndl.dll LogFile to %s\n", debug_dir);

		MySetLogFile(locale_debug_dir);

		g_free(debug_dir);
		g_free(locale_debug_dir);
	}

	messagewin_hwnd = winpidgin_message_window_init();

	if (purple_debug_is_verbose())
		purple_debug_misc("winpidgin", "winpidgin_init end\n");
}

/* Windows Cleanup */

void winpidgin_cleanup(void) {
	purple_debug_info("winpidgin", "winpidgin_cleanup\n");

	if(messagewin_hwnd)
		DestroyWindow(messagewin_hwnd);

}

/* DLL initializer */
/* suppress gcc "no previous prototype" warning */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain(HINSTANCE hinstDLL, G_GNUC_UNUSED DWORD fdwReason, G_GNUC_UNUSED LPVOID lpvReserved) {
	dll_hInstance = hinstDLL;
	return TRUE;
}

typedef HRESULT (WINAPI* DwmIsCompositionEnabledFunction)(BOOL*);
typedef HRESULT (WINAPI* DwmGetWindowAttributeFunction)(HWND, DWORD, PVOID, DWORD);
#ifndef DWMWA_EXTENDED_FRAME_BOUNDS
#	define DWMWA_EXTENDED_FRAME_BOUNDS 9
#endif

DWORD winpidgin_get_lastactive() {
	DWORD result = 0;

	LASTINPUTINFO lii;
	memset(&lii, 0, sizeof(lii));
	lii.cbSize = sizeof(lii);
	if (GetLastInputInfo(&lii))
		result = lii.dwTime;

	return result;
}

