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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <pidgin/pidgintalkatu.h>

GtkWidget *
pidgin_talkatu_editor_new_for_connection(PurpleConnection *pc) {
	GtkWidget *editor = NULL, *input = NULL;

	g_return_val_if_fail(pc != NULL, NULL);

	editor = talkatu_editor_new();
	input = talkatu_editor_get_input(TALKATU_EDITOR(editor));

	gtk_text_view_set_buffer(
		GTK_TEXT_VIEW(input),
		pidgin_talkatu_buffer_new_for_connection(pc)
	);

	return editor;
}

GtkTextBuffer *
pidgin_talkatu_buffer_new_for_connection(PurpleConnection *pc) {
	PurpleConnectionFlags flags = 0;
	GtkTextBuffer *buffer = NULL;
	GSimpleActionGroup *ag = NULL;
	TalkatuBufferStyle style = TALKATU_BUFFER_STYLE_RICH;

	g_return_val_if_fail(pc != NULL, NULL);

	flags = purple_connection_get_flags(pc);

	if(flags & PURPLE_CONNECTION_FLAG_HTML) {
		ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	}

	if(flags & PURPLE_CONNECTION_FLAG_FORMATTING_WBFO) {
		style = TALKATU_BUFFER_STYLE_WHOLE;
	}

	buffer = g_object_new(TALKATU_TYPE_BUFFER,
	                      "action-group", ag,
	                      "style", style,
	                      NULL);
	if(TALKATU_IS_ACTION_GROUP(ag)) {
		talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);
	}
	g_clear_object(&ag);

	return buffer;
}
