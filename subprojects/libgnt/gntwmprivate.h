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

#ifndef GNT_WM_PRIVATE_H
#define GNT_WM_PRIVATE_H

#include "gntwm.h"

typedef enum
{
	GNT_KP_MODE_NORMAL,
	GNT_KP_MODE_RESIZE,
	GNT_KP_MODE_MOVE,
	GNT_KP_MODE_WAIT_ON_CHILD
} GntKeyPressMode;

/**
 * GntAction:
 *
 * An application can register actions which will show up in a 'start-menu'
 * like popup.
 */
typedef struct _GntAction
{
	const gchar *label;
	GCallback callback;
} GntAction;

G_BEGIN_DECLS
/* Private access to some internals. Contact us if you need these. */

G_GNUC_INTERNAL
void gnt_wm_copy_win(GntWidget *widget, GntNode *node);

G_GNUC_INTERNAL
void gnt_wm_set_mainloop(GntWM *wm, GMainLoop *loop);

/*
 * gnt_wm_is_list_window:
 * @wm:     The window-manager.
 * @widget: The widget to check.
 *
 * Check whether @widget is the window manager's listing window.
 *
 * Internal.
 */
G_GNUC_INTERNAL
gboolean gnt_wm_is_list_window(GntWM *wm, GntWidget *widget);

G_GNUC_INTERNAL
void gnt_wm_set_place_by_name(GntWM *wm, const gchar *name, GntWS *ws);
G_GNUC_INTERNAL
void gnt_wm_set_place_by_title(GntWM *wm, const gchar *title, GntWS *ws);

G_GNUC_INTERNAL
void gnt_wm_add_action(GntWM *wm, GntAction *action);

G_GNUC_INTERNAL
GntMenu *gnt_wm_get_menu(GntWM *wm);
G_GNUC_INTERNAL
void gnt_wm_set_menu(GntWM *wm, GntMenu *menu);

G_GNUC_INTERNAL
gboolean gnt_wm_get_event_stack(GntWM *wm);
G_GNUC_INTERNAL
void gnt_wm_set_event_stack(GntWM *wm, gboolean set);

G_GNUC_INTERNAL
GntKeyPressMode gnt_wm_get_keypress_mode(GntWM *wm);
G_GNUC_INTERNAL
void gnt_wm_set_keypress_mode(GntWM *wm, GntKeyPressMode mode);

G_END_DECLS

#endif /* GNT_WM_PRIVATE_H */
