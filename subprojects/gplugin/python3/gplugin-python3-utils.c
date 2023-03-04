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

#include "gplugin-python3-utils.h"

#include <gplugin.h>

gchar *
gplugin_python3_filename_to_module(const gchar *filename)
{
	gchar *base = NULL;
	gchar *e = NULL, *r = NULL;

	g_return_val_if_fail(filename != NULL, NULL);

	/* first make sure we just have a filename */
	base = g_path_get_basename(filename);

	/* now find the last . for the extension */
	e = g_utf8_strrchr(base, -1, g_utf8_get_char("."));
	if(e == NULL) {
		return base;
	}

	/* now copy the module name into r */
	r = g_strndup(base, e - base);

	/* free the basename */
	g_free(base);

	return r;
}

gboolean
gplugin_python3_add_module_path(const gchar *module_path)
{
	PyObject *sys_path = NULL, *path = NULL;
	gboolean ret = FALSE;

	sys_path = PySys_GetObject("path");

	path = PyUnicode_FromString(module_path);

	if(PySequence_Contains(sys_path, path) == 0) {
		PyList_Insert(sys_path, 0, path);
		ret = TRUE;
	}

	Py_DECREF(path);

	return ret;
}

GError *
gplugin_python3_exception_to_gerror(void)
{
	GError *error = NULL;
	PyObject *type = NULL, *value = NULL, *trace = NULL;
	PyObject *type_name = NULL, *value_str = NULL, *obj = NULL;

	if(!PyErr_Occurred()) {
		return NULL;
	}

	PyErr_Fetch(&type, &value, &trace);
	if(type == NULL) {
		return NULL;
	}

	PyErr_NormalizeException(&type, &value, &trace);
	Py_XDECREF(trace);

	type_name = PyObject_GetAttrString(type, "__name__");
	Py_DECREF(type);

	value_str = PyObject_Str(value);
	Py_DECREF(value);

	/* now decode the utf8 into a string we can use */
	obj = PyUnicode_AsUTF8String(type_name);
	Py_DECREF(type_name);
	type_name = obj;

	obj = PyUnicode_AsUTF8String(value_str);
	Py_DECREF(value_str);
	value_str = obj;

	error = g_error_new(
		GPLUGIN_DOMAIN,
		0,
		"%s: %s",
		PyBytes_AsString(type_name),
		PyBytes_AsString(value_str));

	Py_DECREF(type_name);
	Py_DECREF(value_str);

	return error;
}
