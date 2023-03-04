/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_CORE_H
#define PURPLE_CORE_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/purpleui.h>

typedef struct PurpleCore PurpleCore;

G_BEGIN_DECLS

/**
 * purple_core_init:
 * @ui: (transfer full): The [class@Purple.Ui] of the UI using the core.
 * @error: Return address for a #GError, or %NULL.
 *
 * Initializes the core of purple.
 *
 * Returns: %TRUE if successful, otherwise %FALSE with @error potentially set.
 */
gboolean purple_core_init(PurpleUi *ui, GError **error);

/**
 * purple_core_quit:
 *
 * Quits the core of purple, which, depending on the UI, may quit the
 * application using the purple core.
 */
void purple_core_quit(void);

/**
 * purple_core_get_version:
 *
 * Returns the version of the core library.
 *
 * Returns: The version of the core library.
 */
const char *purple_core_get_version(void);

/**
 * purple_get_core:
 *
 * This is used to connect to
 * <link linkend="chapter-signals-core">core signals</link>.
 *
 * Returns: (transfer none): A handle to the purple core.
 */
PurpleCore *purple_get_core(void);

/**
 * purple_core_get_settings_backend:
 *
 * Gets the settings backend to use when saving/loading settings.
 *
 * Note, because we do not want to leak `G_SETTINGS_ENABLE_BACKEND` into
 * libpurple users, this function returns a `gpointer`, and you should cast to
 * `GSettingsBackend *` after setting `G_SETTINGS_ENABLE_BACKEND` for the files
 * where you need it.
 *
 * Returns: (transfer none): The [class@Gio.SettingsBackend] to use.
 *
 * Since: 3.0.0
 */
gpointer purple_core_get_settings_backend(void);

/**
 * purple_core_get_ui:
 *
 * Gets the [class@Purple.Ui] that is running.
 *
 * Returns: (transfer none): The ui.
 */
PurpleUi *purple_core_get_ui(void);

G_END_DECLS

#endif /* PURPLE_CORE_H */

/*

                                                  /===-
                                                `//"\\   """"`---.___.-""
             ______-==|                         | |  \\           _-"`
       __--"""  ,-/-==\\                        | |   `\        ,'
    _-"       /'    |  \\            ___         / /      \      /
  .'        /       |   \\         /"   "\    /' /        \   /'
 /  ____  /         |    \`\.__/-""  D O   \_/'  /          \/'
/-'"    """""---__  |     "-/"   O G     R   /'        _--"`
                  \_|      /   R    __--_  t ),   __--""
                    '""--_/  T   _-"_>--<_\ h '-" \
                   {\__--_/}    / \\__>--<__\ e B  \
                   /'   (_/  _-"  | |__>--<__|   U  |
                  |   _/) )-"     | |__>--<__|  R   |
                  / /" ,_/       / /__>---<__/ N    |
                 o-o _//        /-"_>---<__-" I    /
                 (^("          /"_>---<__-  N   _-"
                ,/|           /__>--<__/  A  _-"
             ,//('(          |__>--<__|  T  /                  .----_
            ( ( '))          |__>--<__|    |                 /' _---_"\
         `-)) )) (           |__>--<__|  O |               /'  /     "\`\
        ,/,'//( (             \__>--<__\  R \            /'  //        ||
      ,( ( ((, ))              "-__>--<_"-_  "--____---"' _/'/        /'
    `"/  )` ) ,/|                 "-_">--<_/-__       __-" _/
  ._-"//( )/ )) `                    ""-'_/_/ /"""""""__--"
   ;'( ')/ ,)(                              """"""""""
  ' ') '( (/
    '   '  `

*/
