/*
 * Finch - Universal Text Chat Client
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Finch is the legal property of its developers, whose names are too numerous
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

#if !defined(FINCH_GLOBAL_HEADER_INSIDE) && !defined(FINCH_COMPILATION)
# error "only <finch.h> may be included directly"
#endif

#ifndef FINCH_UI_H
#define FINCH_UI_H

#include <purple.h>

G_BEGIN_DECLS

#define FINCH_TYPE_UI (finch_ui_get_type())
G_DECLARE_FINAL_TYPE(FinchUi, finch_ui, FINCH, UI, PurpleUi)

/**
 * finch_ui_new:
 *
 * Creates the [class@Purple.Ui] for finch.
 *
 * Note: This isn't really useful outside of Finch itself.
 *
 * Since: 3.0.0
 */
PurpleUi *finch_ui_new(void);

G_END_DECLS

#endif /* FINCH_UI_H */

