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

#ifndef GNT_BINDABLE_H
#define GNT_BINDABLE_H

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>

#define GNT_TYPE_BINDABLE gnt_bindable_get_type()

typedef struct _GntBindable GntBindable;

/**
 * GntBindableClass:
 * @remaps: A table of key remaps from one key to another.
 * @actions: A table of registered names to actions, added by
 *           gnt_bindable_class_register_action().
 * @bindings: A table of registered keys to actions, added by
 *            gnt_bindable_register_binding().
 * @help_window: A #GntWindow used for displaying key binding help.
 *
 * The class structure for #GntBindable. Note, while documented, the fields here
 * are private.
 */
struct _GntBindableClass
{
	/*< private >*/
	GObjectClass parent;

	/*< public >*/
	GHashTable *remaps;
	GHashTable *actions;
	GHashTable *bindings;

	GntBindable * help_window;

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntBindable, gnt_bindable, GNT, BINDABLE, GObject)

const char * gnt_bindable_remap_keys(GntBindable *bindable, const char *text);

/**
 * GntBindableActionCallback:
 * @bindable: A bindable object.
 * @params: Parameters passed to gnt_bindable_class_register_action().
 *
 * A callback for an action registered by gnt_bindable_class_register_action().
 */
typedef gboolean (*GntBindableActionCallback)(GntBindable *bindable,
                                              GList *params);
/**
 * GntBindableActionCallbackNoParam:
 * @bindable: A bindable object.
 *
 * A callback for an action with no parameters.
 */
typedef gboolean (*GntBindableActionCallbackNoParam)(GntBindable *bindable);

/**
 * gnt_bindable_class_register_action:
 * @klass:    The class the binding is for.
 * @name:     The name of the binding.
 * @callback: (scope call): The callback for the binding.
 * @trigger:  The default trigger for the binding, or %NULL.
 * @...:      A %NULL-terminated list of default parameters.
 *
 * Register a bindable action for a class.
 */
void gnt_bindable_class_register_action(GntBindableClass *klass,
                                        const char *name,
                                        GntBindableActionCallback callback,
                                        const char *trigger,
                                        ...) G_GNUC_NULL_TERMINATED;

/**
 * gnt_bindable_register_binding:
 * @klass:   The class the binding is for.
 * @name:    The name of the binding.
 * @trigger: A new trigger for the binding.
 * @...:     A %NULL-terminated list of parameters for the callback.
 *
 * Register a key-binding to an existing action.
 */
void gnt_bindable_register_binding(GntBindableClass *klass, const char *name,
                                   const char *trigger,
                                   ...) G_GNUC_NULL_TERMINATED;

/**
 * gnt_bindable_perform_action_key:
 * @bindable:  The bindable object.
 * @keys:      The key to trigger the action.
 *
 * Perform an action from a keybinding.
 *
 * Returns:  %TRUE if the action was performed successfully, %FALSE otherwise.
 */
gboolean gnt_bindable_perform_action_key(GntBindable *bindable, const char *keys);

/**
 * gnt_bindable_check_key:
 * @bindable:  The bindable object.
 * @keys:      The key to check for.
 *
 * Discover if a key is bound.
 *
 * Returns:  %TRUE if the the key has an action associated with it.
 *
 * Since: 2.4.2
 */
gboolean gnt_bindable_check_key(GntBindable *bindable, const char *keys);

/**
 * gnt_bindable_perform_action_named:
 * @bindable: The bindable object.
 * @name:     The action to perform.
 * @...:      A %NULL-terminated list of parameters.
 *
 * Perform an action on a bindable object.
 *
 * Returns:  %TRUE if the action was performed successfully, %FALSE otherwise.
 */
gboolean gnt_bindable_perform_action_named(GntBindable *bindable, const char *name, ...) G_GNUC_NULL_TERMINATED;

/**
 * gnt_bindable_bindings_view:
 * @bind:  The object to list the bindings for.
 *
 * Returns a GntTree populated with "key" -> "binding" for the widget.
 *
 * Returns: (transfer full): The GntTree.
 *
 * Since: 2.1.1
 */
GntBindable * gnt_bindable_bindings_view(GntBindable *bind);

/**
 * gnt_bindable_build_help_window:
 * @bindable:   The object to list the bindings for.
 *
 * Builds a window that list the key bindings for a GntBindable object.
 * From this window a user can select a listing to rebind a new key for the given action.
 *
 * Returns:  %TRUE
 *
 * Since: 2.1.1
 */

gboolean gnt_bindable_build_help_window(GntBindable *bindable);

G_END_DECLS

#endif /* GNT_BINDABLE_H */

