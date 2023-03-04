/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#include <gntconfig.h>
#include <gntmain.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "Gnt"

#define GNT_ESCAPE '\033'

#ifdef __GNUC__
# ifndef GNT_LOG_DOMAIN
#  define GNT_LOG_DOMAIN ""
# endif
# define gnt_warning(format, args...)  g_warning("(%s) %s: " format, GNT_LOG_DOMAIN, __PRETTY_FUNCTION__, args)
#else /* __GNUC__ */
# define gnt_warning g_warning
#endif

extern int gnt_need_conversation_to_locale;
extern const char *C_(const char *x);
const gchar *gnt_get_config_dir(void);
