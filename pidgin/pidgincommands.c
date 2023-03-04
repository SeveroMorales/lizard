/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>

#include <pidginprivate.h>

/******************************************************************************
 * Command Implementations
 *****************************************************************************/
static PurpleCmdRet
say_command_cb(PurpleConversation *conv, G_GNUC_UNUSED const char *cmd,
               char **args, G_GNUC_UNUSED char **error,
               G_GNUC_UNUSED gpointer data)
{
	purple_conversation_send(conv, args[0]);

	return PURPLE_CMD_RET_OK;
}

static PurpleCmdRet
me_command_cb(PurpleConversation *conv, G_GNUC_UNUSED const char *cmd,
              char **args, G_GNUC_UNUSED char **error,
              G_GNUC_UNUSED gpointer data)
{
	char *tmp;

	tmp = g_strdup_printf("/me %s", args[0]);
	purple_conversation_send(conv, tmp);

	g_free(tmp);
	return PURPLE_CMD_RET_OK;
}

static PurpleCmdRet
debug_command_cb(PurpleConversation *conv, G_GNUC_UNUSED const char *cmd,
                 char **args, G_GNUC_UNUSED char **error,
                 G_GNUC_UNUSED gpointer data)
{
	char *tmp, *markup;

	if (!g_ascii_strcasecmp(args[0], "version")) {
		tmp = g_strdup_printf("Using Pidgin v%s with libpurple v%s.",
				DISPLAY_VERSION, purple_core_get_version());
	} else if (!g_ascii_strcasecmp(args[0], "plugins")) {
		/* Show all the loaded plugins, including plugins marked internal.
		 * This is intentional, since third party protocols are often sources of bugs, and some
		 * plugin loaders can also be buggy.
		 */
		GString *str = g_string_new("Loaded Plugins: ");
		const GList *plugins = purple_plugins_get_loaded();
		if (plugins) {
			for (; plugins; plugins = plugins->next) {
				GPluginPluginInfo *info = GPLUGIN_PLUGIN_INFO(
				        purple_plugin_get_info(
				                PURPLE_PLUGIN(plugins->data)));
				str = g_string_append(
				        str,
				        gplugin_plugin_info_get_name(info));

				if (plugins->next)
					str = g_string_append(str, ", ");
			}
		} else {
			str = g_string_append(str, "(none)");
		}

		tmp = g_string_free(str, FALSE);
	} else if (!g_ascii_strcasecmp(args[0], "unsafe")) {
		if (purple_debug_is_unsafe()) {
			purple_debug_set_unsafe(FALSE);
			purple_conversation_write_system_message(conv,
				_("Unsafe debugging is now disabled."),
				PURPLE_MESSAGE_NO_LOG);
		} else {
			purple_debug_set_unsafe(TRUE);
			purple_conversation_write_system_message(conv,
				_("Unsafe debugging is now enabled."),
				PURPLE_MESSAGE_NO_LOG);
		}

		return PURPLE_CMD_RET_OK;
	} else if (!g_ascii_strcasecmp(args[0], "verbose")) {
		if (purple_debug_is_verbose()) {
			purple_debug_set_verbose(FALSE);
			purple_conversation_write_system_message(conv,
				_("Verbose debugging is now disabled."),
				PURPLE_MESSAGE_NO_LOG);
		} else {
			purple_debug_set_verbose(TRUE);
			purple_conversation_write_system_message(conv,
				_("Verbose debugging is now enabled."),
				PURPLE_MESSAGE_NO_LOG);
		}

		return PURPLE_CMD_RET_OK;
	} else {
		purple_conversation_write_system_message(conv,
			_("Supported debug options are: plugins, version, unsafe, verbose"),
			PURPLE_MESSAGE_NO_LOG);
		return PURPLE_CMD_RET_OK;
	}

	markup = g_markup_escape_text(tmp, -1);
	purple_conversation_send(conv, markup);

	g_free(tmp);
	g_free(markup);
	return PURPLE_CMD_RET_OK;
}

static PurpleCmdRet
help_command_cb(PurpleConversation *conv, G_GNUC_UNUSED const char *cmd,
                char **args, G_GNUC_UNUSED char **error,
                G_GNUC_UNUSED gpointer data)
{
	GList *l, *text;
	GString *s;

	if (args[0] != NULL) {
		s = g_string_new("");
		text = purple_cmd_help(conv, args[0]);

		if (text) {
			for (l = text; l; l = l->next)
				if (l->next)
					g_string_append_printf(s, "%s\n", (char *)l->data);
				else
					g_string_append_printf(s, "%s", (char *)l->data);
		} else {
			g_string_append(s, _("No such command (in this context)."));
		}
	} else {
		s = g_string_new(_("Use \"/help &lt;command&gt;\" for help with a "
				"specific command.<br/>The following commands are available "
				"in this context:<br/>"));

		text = purple_cmd_list(conv);
		for (l = text; l; l = l->next)
			if (l->next)
				g_string_append_printf(s, "%s, ", (char *)l->data);
			else
				g_string_append_printf(s, "%s.", (char *)l->data);
		g_list_free(text);
	}

	purple_conversation_write_system_message(conv, s->str, PURPLE_MESSAGE_NO_LOG);
	g_string_free(s, TRUE);

	return PURPLE_CMD_RET_OK;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
pidgin_commands_init(void) {
	purple_cmd_register("say", "S", PURPLE_CMD_P_DEFAULT,
	                  PURPLE_CMD_FLAG_CHAT | PURPLE_CMD_FLAG_IM, NULL,
	                  say_command_cb, _("say &lt;message&gt;:  Send a message normally as if you weren't using a command."), NULL);
	purple_cmd_register("me", "S", PURPLE_CMD_P_DEFAULT,
	                  PURPLE_CMD_FLAG_CHAT | PURPLE_CMD_FLAG_IM, NULL,
	                  me_command_cb, _("me &lt;action&gt;:  Send an IRC style action to a buddy or chat."), NULL);
	purple_cmd_register("debug", "w", PURPLE_CMD_P_DEFAULT,
	                  PURPLE_CMD_FLAG_CHAT | PURPLE_CMD_FLAG_IM, NULL,
	                  debug_command_cb, _("debug &lt;option&gt;:  Send various debug information to the current conversation."), NULL);
	purple_cmd_register("help", "w", PURPLE_CMD_P_DEFAULT,
	                  PURPLE_CMD_FLAG_CHAT | PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS, NULL,
	                  help_command_cb, _("help &lt;command&gt;:  Help on a specific command."), NULL);
}

void
pidgin_commands_uninit(void) {
}
