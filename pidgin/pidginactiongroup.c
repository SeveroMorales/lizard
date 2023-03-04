/*
 * pidgin
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "pidginactiongroup.h"

#include <purple.h>

#include "pidgincore.h"

struct _PidginActionGroup {
	GSimpleActionGroup parent;
};

/******************************************************************************
 * Helpers
 *****************************************************************************/

/*< private >
 * pidgin_action_group_string_pref_handler:
 * @group: The #PidginActionGroup instance.
 * @action_name: The name of the action to update.
 * @value: The value of the preference.
 *
 * Changes the state of the action named @action_name to match @value.
 *
 * This function is meant to be called from a #PurplePrefCallback function as
 * there isn't a good way to have a #PurplePrefCallback with multiple items in
 * the data parameter without leaking them forever.
 */
static void
pidgin_action_group_string_pref_handler(PidginActionGroup *group,
                                        const gchar *action_name,
                                        const gchar *value)
{
	GAction *action = NULL;

	action = g_action_map_lookup_action(G_ACTION_MAP(group), action_name);
	if(action != NULL) {
		g_simple_action_set_state(G_SIMPLE_ACTION(action),
		                          g_variant_new_string(value));
	}
}

/*< private >
 * pidgin_action_group_setup_string:
 * @group: The #PidginActionGroup instance.
 * @action_name: The name of the action to setup.
 * @pref_name: The name of the preference that @action_name is tied to.
 * @callback: (scope notified): A #PurplePrefCallback to call when the
 *            preference is changed.
 *
 * Initializes the string action named @action_name to the value of @pref_name
 * and setups up a preference change callback to @callback to maintain the
 * state of the action.
 */
static void
pidgin_action_group_setup_string(PidginActionGroup *group,
                                 const gchar *action_name,
                                 const gchar *pref_name,
                                 PurplePrefCallback callback)
{
	GAction *action = NULL;
	const gchar *value = NULL;

	/* find the action, if we can't find it, bail */
	action = g_action_map_lookup_action(G_ACTION_MAP(group), action_name);
	g_return_if_fail(action != NULL);

	/* change the state of the action to match the preference value. */
	value = purple_prefs_get_string(pref_name);
	g_simple_action_set_state(G_SIMPLE_ACTION(action),
	                          g_variant_new_string(value));

	/* finally add a preference callback to update the state based on the
	 * preference.
	 */
	purple_prefs_connect_callback(group, pref_name, callback, group);
}

/******************************************************************************
 * Preference Callbacks
 *****************************************************************************/
static void
pidgin_action_group_sort_method_callback(G_GNUC_UNUSED const gchar *name,
                                         G_GNUC_UNUSED PurplePrefType type,
                                         gconstpointer value,
                                         gpointer data)
{
	PidginActionGroup *group = PIDGIN_ACTION_GROUP(data);

	pidgin_action_group_string_pref_handler(group,
	                                        PIDGIN_ACTION_SORT_METHOD,
	                                        value);
}

/******************************************************************************
 * Action Callbacks
 *****************************************************************************/
static void
pidgin_action_group_sort_method(G_GNUC_UNUSED GSimpleAction *action,
                                GVariant *value, G_GNUC_UNUSED gpointer data)
{
	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/blist/sort_type",
	                        g_variant_get_string(value, NULL));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginActionGroup, pidgin_action_group,
              G_TYPE_SIMPLE_ACTION_GROUP)

static void
pidgin_action_group_init(PidginActionGroup *group) {
	GActionEntry entries[] = {
		{
			.name = PIDGIN_ACTION_SORT_METHOD,
			.parameter_type = "s",
			.state = "'none'",
			.change_state = pidgin_action_group_sort_method,
		},
	};

	g_action_map_add_action_entries(G_ACTION_MAP(group), entries,
	                                G_N_ELEMENTS(entries), NULL);

	/* now add some handlers for preference changes and set actions to the
	 * correct value.
	 */
	pidgin_action_group_setup_string(group, PIDGIN_ACTION_SORT_METHOD,
	                                 PIDGIN_PREFS_ROOT "/blist/sort_type",
	                                 pidgin_action_group_sort_method_callback);
};

static void
pidgin_action_group_finalize(GObject *obj) {
	purple_signals_disconnect_by_handle(obj);

	G_OBJECT_CLASS(pidgin_action_group_parent_class)->finalize(obj);
}

static void
pidgin_action_group_class_init(PidginActionGroupClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = pidgin_action_group_finalize;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GSimpleActionGroup *
pidgin_action_group_new(void) {
	return G_SIMPLE_ACTION_GROUP(g_object_new(PIDGIN_TYPE_ACTION_GROUP, NULL));
}

