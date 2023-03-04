/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(TALKATU_GLOBAL_HEADER_INSIDE) && !defined(TALKATU_COMPILATION)
#error "only <talkatu.h> may be included directly"
#endif

#ifndef TALKATU_INPUT_H
#define TALKATU_INPUT_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <talkatu/talkatuview.h>

typedef enum _TalkatuInputSendBinding /*< prefix=TALKATU_INPUT_SEND_BINDING,underscore_name=TALKATU_INPUT_SEND_BINDING >*/
{
	TALKATU_INPUT_SEND_BINDING_RETURN         = 1 << 0,
	TALKATU_INPUT_SEND_BINDING_KP_ENTER       = 1 << 1,
	TALKATU_INPUT_SEND_BINDING_SHIFT_RETURN   = 1 << 2,
	TALKATU_INPUT_SEND_BINDING_CONTROL_RETURN = 1 << 3,
} TalkatuInputSendBinding;

G_BEGIN_DECLS

#define TALKATU_TYPE_INPUT (talkatu_input_get_type())
G_DECLARE_DERIVABLE_TYPE(TalkatuInput, talkatu_input, TALKATU, INPUT, TalkatuView)

struct _TalkatuInputClass {
	/*< private >*/
	TalkatuViewClass parent;

	/*< public >*/
	void (*send_message)(TalkatuInput *input);

	/*< private >*/
	gpointer reserved[4];
};

GtkWidget *talkatu_input_new(void);

void talkatu_input_set_send_binding(TalkatuInput *input, TalkatuInputSendBinding bindings);
TalkatuInputSendBinding talkatu_input_get_send_binding(TalkatuInput *input);

void talkatu_input_send_message(TalkatuInput *input);

G_END_DECLS

#endif /* TALKATU_INPUT_H */
