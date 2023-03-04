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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <glib.h>

#include <pygobject.h>

gint
main(gint argc, gchar *argv[])
{
#if PY_VERSION_HEX >= 0x03080000
	PyConfig config;
	PyStatus status;

	PyConfig_InitIsolatedConfig(&config);
	status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
	if(PyStatus_Exception(status)) {
		printf("failed to convert argv[0] to wchar_t string: %s\n", argv[0]);
		PyConfig_Clear(&config);
		return FALSE;
	}

	status = Py_InitializeFromConfig(&config);
	PyConfig_Clear(&config);

	if(PyStatus_Exception(status)) {
		printf("Could not initialize Python.\n");
		return FALSE;
	}
#else  /* PY_VERSION_HEX >= 0x03080000 */
	wchar_t *wargv[] = {NULL, NULL};
	size_t len;

	/* initialize python */
	if(!Py_IsInitialized()) {
		Py_InitializeEx(FALSE);
	}

	/* setup wargv */
	len = mbstowcs(NULL, argv[0], 0);
	if(len == (size_t)-1) {
		printf(
			"Failed to call mbstowcs to find length of argv[0]: %s\n",
			argv[0]);
		return -1;
	}

	wargv[0] = g_new0(wchar_t, len + 1);
	len = mbstowcs(wargv[0], argv[0], len + 1);
	if(len == (size_t)-1) {
		g_free(wargv[0]);
		printf("Failed to call mbstowcs to convert argv[0]: %s\n", argv[0]);
		return -1;
	}

	/* setup sys.path */
	PySys_SetArgvEx(1, wargv, 0);

	g_free(wargv[0]);
#endif /* PY_VERSION_HEX >= 0x03080000 */

	/* initialize pygobject */
	pygobject_init(3, 0, 0);
	if(PyErr_Occurred()) {
		printf("Calling pygobject_init failed.\n");
		PyErr_Print();
		return -1;
	}

	return 0;
}
