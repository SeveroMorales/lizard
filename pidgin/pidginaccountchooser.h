/* pidgin
 *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301 USA
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_ACCOUNT_CHOOSER_H
#define PIDGIN_ACCOUNT_CHOOSER_H

#include <gtk/gtk.h>
#include <adwaita.h>

#include <purple.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_ACCOUNT_CHOOSER (pidgin_account_chooser_get_type())

G_DECLARE_FINAL_TYPE(PidginAccountChooser, pidgin_account_chooser, PIDGIN,
                     ACCOUNT_CHOOSER, AdwBin)

/**
 * pidgin_account_chooser_new:
 *
 * Creates a combo box filled with accounts.
 *
 * Returns: (transfer full): The account chooser combo box.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_account_chooser_new(void);

/**
 * pidgin_account_chooser_get_filter:
 * @chooser: The chooser.
 *
 * Gets the filter used to determine which accounts to show.
 *
 * Returns: (transfer none): Returns the #GtkFilter that is currently
 *          displayed.
 *
 * Since: 3.0.0
 */
GtkFilter *pidgin_account_chooser_get_filter(PidginAccountChooser *chooser);

/**
 * pidgin_account_chooser_set_filter:
 * @chooser: The chooser.
 * @filter: The filter.
 *
 * Sets the current list filter for the account chooser.
 *
 * Since: 3.0.0
 */
void pidgin_account_chooser_set_filter(PidginAccountChooser *chooser, GtkFilter *filter);

/**
 * pidgin_account_chooser_get_selected:
 * @chooser: The chooser created by pidgin_account_chooser_new().
 *
 * Gets the currently selected account from an account combo box.
 *
 * Returns: (transfer full): Returns the #PurpleAccount that is currently
 *          selected.
 *
 * Since: 3.0.0
 */
PurpleAccount *pidgin_account_chooser_get_selected(PidginAccountChooser *chooser);

/**
 * pidgin_account_chooser_set_selected:
 * @chooser: The chooser created by pidgin_account_chooser_new().
 * @account: The #PurpleAccount to select.
 *
 * Sets the currently selected account for an account combo box.
 *
 * Since: 3.0.0
 */
void pidgin_account_chooser_set_selected(PidginAccountChooser *chooser,
                                         PurpleAccount *account);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_CHOOSER_H */
