/*
 * Copyright (C) 2011-2021 Gary Kramlich <grim@reaperworld.com>
 * Copyright (C) 2013 Ankit Vani <a@nevitus.org>
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

#ifndef DYNAMIC_TEST_H
#define DYNAMIC_TEST_H

#include <glib.h>
#include <glib-object.h>

#define DYNAMIC_TYPE_TEST (dynamic_test_get_type())
G_DECLARE_FINAL_TYPE(DynamicTest, dynamic_test, DYNAMIC, TEST, GObject)

struct _DynamicTest {
	GObject gparent;
};

#endif /* DYNAMIC_TEST_H */
