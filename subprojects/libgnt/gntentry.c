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

#include <ctype.h>
#include <string.h>

#include "gntinternal.h"
#include "gntbox.h"
#include "gntentry.h"
#include "gntstyle.h"
#include "gnttree.h"
#include "gntutils.h"

#include "gntwidgetprivate.h"

enum
{
	SIG_TEXT_CHANGED,
	SIG_COMPLETION,
	SIGS,
};

typedef enum
{
	ENTRY_JAIL = -1,    /* Suspend the kill ring. */
	ENTRY_DEL_BWD_WORD = 1,
	ENTRY_DEL_BWD_CHAR,
	ENTRY_DEL_FWD_WORD,
	ENTRY_DEL_FWD_CHAR,
	ENTRY_DEL_EOL,
	ENTRY_DEL_BOL,
} GntEntryAction;

typedef struct
{
	GString *buffer;
	GntEntryAction last;
} GntEntryKillRing;

typedef struct
{
	char *needle;
} GntEntrySearch;

typedef struct
{
	GntEntryFlag flag;

	char *start;
	char *end;
	char *scroll; /* Current scrolling position */
	char *cursor; /* Cursor location */
	              /* 0 <= cursor - scroll < widget-width */

	size_t buffer; /* Size of the buffer */

	int max; /* 0 means infinite */
	gboolean masked;

	GList *history; /* History of the strings. User can use this by pressing
	                   ctrl+up/down */
	int histlength; /* How long can the history be? */

	GList *suggests; /* List of suggestions */
	gboolean word; /* Are the suggestions for only a word, or for the whole
	                  thing? */
	gboolean always; /* Should the list of suggestions show at all times, or
	                    only on tab-press? */
	GntWidget *ddown; /* The dropdown with the suggested list */
	GntEntryKillRing *killring;
	GntEntrySearch *search;
} GntEntryPrivate;

static guint signals[SIGS] = { 0 };

static gboolean gnt_entry_key_pressed(GntWidget *widget, const char *text);
static void gnt_entry_set_text_internal(GntEntry *entry, const char *text);

G_DEFINE_TYPE_WITH_PRIVATE(GntEntry, gnt_entry, GNT_TYPE_WIDGET)

static gboolean
update_kill_ring(GntEntryPrivate *priv, GntEntryAction action, const char *text,
                 int len)
{
	if (action < 0) {
		priv->killring->last = action;
		return FALSE;
	}

	if (len == 0)
		len = strlen(text);
	else if (len < 0) {
		text += len;
		len = -len;
	}

	if (action != priv->killring->last) {
		struct {
			GntEntryAction one;
			GntEntryAction two;
		} merges[] = {
			{ENTRY_DEL_BWD_WORD, ENTRY_DEL_FWD_WORD},
			{ENTRY_DEL_BWD_CHAR, ENTRY_DEL_FWD_CHAR},
			{ENTRY_DEL_BOL, ENTRY_DEL_EOL},
			{ENTRY_JAIL, ENTRY_JAIL},
		};
		int i;

		for (i = 0; merges[i].one != ENTRY_JAIL; i++) {
			if (merges[i].one == priv->killring->last &&
			    merges[i].two == action) {
				g_string_append_len(priv->killring->buffer,
				                    text, len);
				break;
			} else if (merges[i].one == action &&
			           merges[i].two == priv->killring->last) {
				g_string_prepend_len(priv->killring->buffer,
				                     text, len);
				break;
			}
		}
		if (merges[i].one == ENTRY_JAIL) {
			g_string_assign(priv->killring->buffer, text);
			g_string_truncate(priv->killring->buffer, len);
		}
		priv->killring->last = action;
	} else {
		if (action == ENTRY_DEL_BWD_CHAR || action == ENTRY_DEL_BWD_WORD)
			g_string_prepend_len(priv->killring->buffer, text, len);
		else
			g_string_append_len(priv->killring->buffer, text, len);
	}
	return TRUE;
}

static void
destroy_suggest(GntEntryPrivate *priv)
{
	if (priv->ddown) {
		gnt_widget_destroy(gnt_widget_get_parent(priv->ddown));
		priv->ddown = NULL;
	}
}

static char *
get_beginning_of_word(GntEntryPrivate *priv)
{
	char *s = priv->cursor;
	while (s > priv->start) {
		char *t = g_utf8_find_prev_char(priv->start, s);
		if (isspace(*t))
			break;
		s = t;
	}
	return s;
}

static gboolean
complete_suggest(GntEntry *entry, const char *text)
{
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	int offstart = 0, offend = 0;

	if (priv->word) {
		char *s = get_beginning_of_word(priv);
		const char *iter = text;
		offstart = g_utf8_pointer_to_offset(priv->start, s);
		while (*iter && toupper(*s) == toupper(*iter)) {
			*s++ = *iter++;
		}
		if (*iter) {
			gnt_entry_key_pressed(GNT_WIDGET(entry), iter);
		}
		offend = g_utf8_pointer_to_offset(priv->start, priv->cursor);
	} else {
		offstart = 0;
		gnt_entry_set_text_internal(entry, text);
		offend = g_utf8_strlen(text, -1);
	}

	g_signal_emit(G_OBJECT(entry), signals[SIG_COMPLETION], 0,
	              priv->start + offstart, priv->start + offend);
	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	return TRUE;
}

static int
max_common_prefix(const char *s, const char *t)
{
	const char *f = s;
	while (*f && *t && *f == *t++)
		f++;
	return f - s;
}

static gboolean
show_suggest_dropdown(GntEntry *entry)
{
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	char *suggest = NULL;
	gsize len;
	int offset = 0, x, y;
	int count = 0;
	GList *iter;
	const char *text = NULL;
	const char *sgst = NULL;
	int max = -1;

	if (priv->word) {
		char *s = get_beginning_of_word(priv);
		suggest = g_strndup(s, priv->cursor - s);
		if (priv->scroll < s) {
			offset = gnt_util_onscreen_width(priv->scroll, s);
		}
	} else {
		suggest = g_strdup(priv->start);
	}
	len = strlen(suggest);  /* Don't need to use the utf8-function here */

	if (priv->ddown == NULL) {
		GntWidget *box = gnt_vbox_new(FALSE);
		priv->ddown = gnt_tree_new();
		gnt_tree_set_compare_func(GNT_TREE(priv->ddown),
		                          (GCompareFunc)g_utf8_collate);
		gnt_box_add_widget(GNT_BOX(box), priv->ddown);

		gnt_widget_set_transient(box, TRUE);

		gnt_widget_get_position(GNT_WIDGET(entry), &x, &y);
		x += offset;
		y++;
		if (y + 10 >= getmaxy(stdscr))
			y -= 11;
		gnt_widget_set_position(box, x, y);
	} else {
		gnt_tree_remove_all(GNT_TREE(priv->ddown));
	}

	for (count = 0, iter = priv->suggests; iter; iter = iter->next) {
		text = iter->data;
		if (g_ascii_strncasecmp(suggest, text, len) == 0 && strlen(text) >= len)
		{
			gnt_tree_add_row_after(
			        GNT_TREE(priv->ddown), (gpointer)text,
			        gnt_tree_create_row(GNT_TREE(priv->ddown),
			                            text),
			        NULL, NULL);
			count++;
			if (max == -1)
				max = strlen(text) - len;
			else if (max)
				max = MIN(max, max_common_prefix(sgst + len, text + len));
			sgst = text;
		}
	}
	g_free(suggest);

	if (count == 0) {
		destroy_suggest(priv);
		return FALSE;
	} else if (count == 1) {
		char *store = g_strndup(priv->start, priv->end - priv->start);
		gboolean ret;

		destroy_suggest(priv);
		complete_suggest(entry, sgst);

		ret = (strncmp(store, priv->start, priv->end - priv->start) !=
		       0);
		g_free(store);
		return ret;
	} else {
		if (max > 0) {
			GntWidget *ddown = priv->ddown;
			char *match = g_strndup(sgst + len, max);
			priv->ddown = NULL;
			gnt_entry_key_pressed(GNT_WIDGET(entry), match);
			g_free(match);
			priv->ddown = ddown;
		}
		gnt_widget_draw(gnt_widget_get_parent(priv->ddown));
	}

	return TRUE;
}

static void
gnt_entry_draw(GntWidget *widget)
{
	GntEntry *entry = GNT_ENTRY(widget);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	WINDOW *window = gnt_widget_get_window(widget);
	gint width;
	int stop;
	gboolean focus;
	int curpos;

	if ((focus = gnt_widget_has_focus(widget)))
		wbkgdset(window, '\0' | gnt_color_pair(GNT_COLOR_TEXT_NORMAL));
	else
		wbkgdset(window, '\0' | gnt_color_pair(GNT_COLOR_HIGHLIGHT_D));

	if (priv->masked) {
		mvwhline(window, 0, 0, gnt_ascii_only() ? '*' : ACS_BULLET,
		         g_utf8_pointer_to_offset(priv->scroll, priv->end));
	} else
		mvwprintw(window, 0, 0, "%s", C_(priv->scroll));

	stop = gnt_util_onscreen_width(priv->scroll, priv->end);
	gnt_widget_get_internal_size(GNT_WIDGET(entry), &width, NULL);
	if (stop < width) {
		mvwhline(window, 0, stop, GNT_ENTRY_CHAR, width - stop);
	}

	curpos = gnt_util_onscreen_width(priv->scroll, priv->cursor);
	if (focus) {
		mvwchgat(window, 0, curpos, 1, A_REVERSE, GNT_COLOR_TEXT_NORMAL,
		         NULL);
	}
	(void)wmove(window, 0, curpos);
}

static void
gnt_entry_size_request(GntWidget *widget)
{
	if (!gnt_widget_get_mapped(widget)) {
		gnt_widget_set_internal_size(widget, 20, 1);
	}
}

static void
gnt_entry_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
}

static void
entry_redraw(GntWidget *widget)
{
	gnt_entry_draw(widget);
	gnt_widget_queue_update(widget);
}

static void
entry_text_changed(GntEntry *entry)
{
	g_signal_emit(entry, signals[SIG_TEXT_CHANGED], 0);
}

static gboolean
move_back(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (priv->cursor <= priv->start) {
		return FALSE;
	}

	priv->cursor = g_utf8_find_prev_char(priv->start, priv->cursor);
	if (priv->cursor < priv->scroll) {
		priv->scroll = priv->cursor;
	}

	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	entry_redraw(GNT_WIDGET(entry));
	return TRUE;
}

static void
scroll_to_fit(GntEntry *entry)
{
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	gint width;

	gnt_widget_get_internal_size(GNT_WIDGET(entry), &width, NULL);
	while (gnt_util_onscreen_width(priv->scroll, priv->cursor) >= width) {
		priv->scroll = g_utf8_find_next_char(priv->scroll, NULL);
	}
}

static gboolean
move_forward(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (priv->cursor >= priv->end) {
		return FALSE;
	}

	priv->cursor = g_utf8_find_next_char(priv->cursor, NULL);
	scroll_to_fit(entry);

	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	entry_redraw(GNT_WIDGET(entry));
	return TRUE;
}

static gboolean
backspace(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	int len;
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (priv->cursor <= priv->start) {
		return TRUE;
	}

	len = priv->cursor - g_utf8_find_prev_char(priv->start, priv->cursor);
	update_kill_ring(priv, ENTRY_JAIL, priv->cursor, -len);
	priv->cursor -= len;

	memmove(priv->cursor, priv->cursor + len, priv->end - priv->cursor);
	priv->end -= len;

	if (priv->scroll > priv->start) {
		priv->scroll = g_utf8_find_prev_char(priv->start, priv->scroll);
	}

	entry_redraw(GNT_WIDGET(entry));
	if (priv->ddown) {
		show_suggest_dropdown(entry);
	}

	entry_text_changed(entry);
	return TRUE;
}

static gboolean
delkey(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	int len;
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (priv->cursor >= priv->end) {
		return FALSE;
	}

	len = g_utf8_find_next_char(priv->cursor, NULL) - priv->cursor;
	update_kill_ring(priv, ENTRY_JAIL, priv->cursor, len);
	memmove(priv->cursor, priv->cursor + len,
	        priv->end - priv->cursor - len + 1);
	priv->end -= len;
	entry_redraw(GNT_WIDGET(entry));

	if (priv->ddown) {
		show_suggest_dropdown(entry);
	}

	entry_text_changed(entry);
	return TRUE;
}

static gboolean
move_start(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	priv->scroll = priv->cursor = priv->start;
	entry_redraw(GNT_WIDGET(entry));
	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	return TRUE;
}

static gboolean
move_end(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	priv->cursor = priv->end;

	/* This should be better than this */
	scroll_to_fit(entry);

	entry_redraw(GNT_WIDGET(entry));
	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	return TRUE;
}

static gboolean
history_next(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->histlength && priv->history->prev) {
		priv->history = priv->history->prev;
		gnt_entry_set_text_internal(entry, priv->history->data);
		destroy_suggest(priv);
		entry_text_changed(entry);

		update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
		return TRUE;
	}
	return FALSE;
}

static gboolean
history_prev(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->histlength && priv->history->next) {
		if (priv->history->prev == NULL) {
			/* Save the current contents */
			char *text = g_strdup(gnt_entry_get_text(entry));
			g_free(priv->history->data);
			priv->history->data = text;
		}

		priv->history = priv->history->next;
		gnt_entry_set_text_internal(entry, priv->history->data);
		destroy_suggest(priv);
		entry_text_changed(entry);

		update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
		return TRUE;
	}
	return FALSE;
}

static gboolean
history_search(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	GList *iter;
	const char *current;

	if (priv->history->prev && priv->search->needle) {
		current = priv->search->needle;
	} else {
		current = gnt_entry_get_text(entry);
	}

	if (!priv->histlength || !priv->history->next || !*current) {
		return FALSE;
	}

	for (iter = priv->history->next; iter; iter = iter->next) {
		const char *str = iter->data;
		/* A more utf8-friendly version of strstr would have been better, but
		 * for now, this will have to do. */
		if (strstr(str, current) != NULL)
			break;
	}

	if (!iter)
		return TRUE;

	if (priv->history->prev == NULL) {
		/* We are doing it for the first time. Save the current contents */
		char *text = g_strdup(gnt_entry_get_text(entry));

		g_free(priv->search->needle);
		priv->search->needle = g_strdup(current);

		g_free(priv->history->data);
		priv->history->data = text;
	}

	priv->history = iter;
	gnt_entry_set_text_internal(entry, priv->history->data);
	destroy_suggest(priv);
	entry_text_changed(entry);

	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	return TRUE;
}

static gboolean
clipboard_paste(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	gchar *i, *text, *a, *all;
	text = i = gnt_get_clipboard_string();
	while (*i != '\0') {
		i = g_utf8_next_char(i);
		if (*i == '\r' || *i == '\n')
			*i = ' ';
	}
	a = g_strndup(priv->start, priv->cursor - priv->start);
	all = g_strconcat(a, text, priv->cursor, NULL);
	gnt_entry_set_text_internal(entry, all);
	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	g_free(a);
	g_free(text);
	g_free(all);
	return TRUE;
}

static gboolean
suggest_show(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->ddown) {
		gnt_bindable_perform_action_named(GNT_BINDABLE(priv->ddown),
		                                  "move-down", NULL);
		return TRUE;
	}
	return show_suggest_dropdown(entry);
}

static gboolean
suggest_next(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->ddown) {
		gnt_bindable_perform_action_named(GNT_BINDABLE(priv->ddown),
		                                  "move-down", NULL);
		return TRUE;
	}
	return FALSE;
}

static gboolean
suggest_prev(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->ddown) {
		gnt_bindable_perform_action_named(GNT_BINDABLE(priv->ddown),
		                                  "move-up", NULL);
		return TRUE;
	}
	return FALSE;
}

static gboolean
suggest_next_page(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->ddown) {
		gnt_bindable_perform_action_named(GNT_BINDABLE(priv->ddown),
		                                  "page-down", NULL);
		return TRUE;
	}
	return FALSE;
}

static gboolean
suggest_prev_page(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	if (priv->ddown) {
		gnt_bindable_perform_action_named(GNT_BINDABLE(priv->ddown),
		                                  "page-up", NULL);
		return TRUE;
	}
	return FALSE;
}

static gboolean
del_to_home(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (priv->cursor <= priv->start) {
		return TRUE;
	}

	update_kill_ring(priv, ENTRY_DEL_BOL, priv->start,
	                 priv->cursor - priv->start);
	memmove(priv->start, priv->cursor, priv->end - priv->cursor);
	priv->end -= (priv->cursor - priv->start);
	priv->cursor = priv->scroll = priv->start;
	memset(priv->end, '\0', priv->buffer - (priv->end - priv->start));
	entry_redraw(GNT_WIDGET(bind));
	entry_text_changed(entry);
	return TRUE;
}

static gboolean
del_to_end(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (priv->end <= priv->cursor) {
		return TRUE;
	}

	update_kill_ring(priv, ENTRY_DEL_EOL, priv->cursor,
	                 priv->end - priv->cursor);
	priv->end = priv->cursor;
	memset(priv->end, '\0', priv->buffer - (priv->end - priv->start));
	entry_redraw(GNT_WIDGET(bind));
	entry_text_changed(entry);
	return TRUE;
}

#define SAME(a,b)    ((g_unichar_isalnum(a) && g_unichar_isalnum(b)) || \
				(g_unichar_isspace(a) && g_unichar_isspace(b)) || \
				(g_unichar_iswide(a) && g_unichar_iswide(b)) || \
				(g_unichar_ispunct(a) && g_unichar_ispunct(b)))

static const char *
begin_word(const char *text, const char *begin)
{
	gunichar ch = 0;
	while (text > begin && (!*text || g_unichar_isspace(g_utf8_get_char(text))))
		text = g_utf8_find_prev_char(begin, text);
	ch = g_utf8_get_char(text);
	while ((text = g_utf8_find_prev_char(begin, text)) >= begin) {
		gunichar cur = g_utf8_get_char(text);
		if (!SAME(ch, cur))
			break;
	}

	return (text ? g_utf8_find_next_char(text, NULL) : begin);
}

static const char *
next_begin_word(const char *text, const char *end)
{
	gunichar ch = 0;

	while (text && text < end && g_unichar_isspace(g_utf8_get_char(text)))
		text = g_utf8_find_next_char(text, end);

	if (text) {
		ch = g_utf8_get_char(text);
		while ((text = g_utf8_find_next_char(text, end)) != NULL && text <= end) {
			gunichar cur = g_utf8_get_char(text);
			if (!SAME(ch, cur))
				break;
		}
	}
	return (text ? text : end);
}

#undef SAME
static gboolean
move_back_word(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	const char *iter = g_utf8_find_prev_char(priv->start, priv->cursor);

	if (iter < priv->start) {
		return TRUE;
	}

	iter = begin_word(iter, priv->start);
	priv->cursor = (char *)iter;
	if (priv->cursor < priv->scroll) {
		priv->scroll = priv->cursor;
	}
	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	entry_redraw(GNT_WIDGET(bind));
	return TRUE;
}

static gboolean
del_prev_word(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntWidget *widget = GNT_WIDGET(bind);
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	char *iter = g_utf8_find_prev_char(priv->start, priv->cursor);
	int count;

	if (iter < priv->start) {
		return TRUE;
	}

	iter = (char *)begin_word(iter, priv->start);
	count = priv->cursor - iter;
	update_kill_ring(priv, ENTRY_DEL_BWD_WORD, iter, count);
	memmove(iter, priv->cursor, priv->end - priv->cursor);
	priv->end -= count;
	priv->cursor = iter;
	if (priv->cursor <= priv->scroll) {
		gint width;
		gnt_widget_get_internal_size(widget, &width, NULL);
		priv->scroll = priv->cursor - width + 2;
		if (priv->scroll < priv->start) {
			priv->scroll = priv->start;
		}
	}
	memset(priv->end, '\0', priv->buffer - (priv->end - priv->start));
	entry_redraw(widget);
	entry_text_changed(entry);

	return TRUE;
}

static gboolean
move_forward_word(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	GntWidget *widget = GNT_WIDGET(bind);
	priv->cursor = (char *)next_begin_word(priv->cursor, priv->end);
	scroll_to_fit(entry);
	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	entry_redraw(widget);
	return TRUE;
}

static gboolean
delete_forward_word(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	GntWidget *widget = GNT_WIDGET(bind);
	char *iter = (char *)next_begin_word(priv->cursor, priv->end);
	int len = priv->end - iter + 1;
	if (len <= 0)
		return TRUE;
	update_kill_ring(priv, ENTRY_DEL_FWD_WORD, priv->cursor,
	                 iter - priv->cursor);
	memmove(priv->cursor, iter, len);
	len = iter - priv->cursor;
	priv->end -= len;
	memset(priv->end, '\0', len);
	entry_redraw(widget);
	entry_text_changed(entry);
	return TRUE;
}

static gboolean
transpose_chars(GntBindable *bind, GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	char *current, *prev;
	char hold[8];  /* that's right */

	if (priv->cursor <= priv->start) {
		return FALSE;
	}

	if (!*priv->cursor) {
		priv->cursor = g_utf8_find_prev_char(priv->start, priv->cursor);
	}

	current = priv->cursor;
	prev = g_utf8_find_prev_char(priv->start, priv->cursor);
	move_forward(bind, params);

	/* Let's do this dance! */
	memcpy(hold, prev, current - prev);
	memmove(prev, current, priv->cursor - current);
	memcpy(prev + (priv->cursor - current), hold, current - prev);

	update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
	entry_redraw(GNT_WIDGET(entry));
	entry_text_changed(entry);
	return TRUE;
}

static gboolean
entry_yank(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntEntry *entry = GNT_ENTRY(bind);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	gnt_entry_key_pressed(GNT_WIDGET(entry), priv->killring->buffer->str);
	return TRUE;
}

static gboolean
gnt_entry_key_pressed(GntWidget *widget, const char *text)
{
	GntEntry *entry = GNT_ENTRY(widget);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	if (text[0] == 27)
	{
		if (text[1] == 0)
		{
			destroy_suggest(priv);
			return TRUE;
		}

		return FALSE;
	}

	if ((text[0] == '\r' || text[0] == ' ' || text[0] == '\n') &&
	    priv->ddown) {
		char *text = g_strdup(
		        gnt_tree_get_selection_data(GNT_TREE(priv->ddown)));
		destroy_suggest(priv);
		complete_suggest(entry, text);
		g_free(text);
		update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
		entry_text_changed(entry);
		return TRUE;
	}

	if (!iscntrl(text[0]))
	{
		const char *str, *next;

		for (str = text; *str; str = next)
		{
			gsize len;
			next = g_utf8_find_next_char(str, NULL);
			len = next - str;

			/* Valid input? */
			/* XXX: Is it necessary to use _unichar_ variants here? */
			if (ispunct(*str) &&
			    (priv->flag & GNT_ENTRY_FLAG_NO_PUNCT)) {
				continue;
			}
			if (isspace(*str) &&
			    (priv->flag & GNT_ENTRY_FLAG_NO_SPACE)) {
				continue;
			}
			if (isalpha(*str) &&
			    !(priv->flag & GNT_ENTRY_FLAG_ALPHA)) {
				continue;
			}
			if (isdigit(*str) &&
			    !(priv->flag & GNT_ENTRY_FLAG_INT)) {
				continue;
			}

			/* Reached the max? */
			if (priv->max &&
			    g_utf8_pointer_to_offset(priv->start, priv->end) >=
			            priv->max) {
				continue;
			}

			if ((gsize)(priv->end + len - priv->start) >=
			    priv->buffer) {
				/* This will cause the buffer to grow */
				char *tmp = g_strdup(priv->start);
				gnt_entry_set_text_internal(entry, tmp);
				g_free(tmp);
			}

			memmove(priv->cursor + len, priv->cursor,
			        priv->end - priv->cursor + 1);
			priv->end += len;

			while (str < next)
			{
				if (*str == '\r' || *str == '\n')
					*priv->cursor = ' ';
				else
					*priv->cursor = *str;
				priv->cursor++;
				str++;
			}

			scroll_to_fit(entry);

			if (priv->ddown) {
				show_suggest_dropdown(entry);
			}
		}
		update_kill_ring(priv, ENTRY_JAIL, NULL, 0);
		entry_redraw(widget);
		entry_text_changed(entry);
		return TRUE;
	}

	if (text[0] == '\r' || text[0] == '\n') {
		gnt_widget_activate(widget);
		return TRUE;
	}

	return FALSE;
}

static void
jail_killring(GntEntryKillRing *kr)
{
	g_string_free(kr->buffer, TRUE);
	g_free(kr);
}

static void
gnt_entry_destroy(GntWidget *widget)
{
	GntEntry *entry = GNT_ENTRY(widget);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	g_clear_pointer(&priv->start, g_free);

	if (priv->history) {
		g_list_free_full(g_list_first(priv->history), g_free);
		priv->history = NULL;
	}

	if (priv->suggests) {
		g_list_free_full(priv->suggests, g_free);
		priv->suggests = NULL;
	}

	if (priv->ddown) {
		gnt_widget_destroy(gnt_widget_get_parent(priv->ddown));
		priv->ddown = NULL;
	}

	g_clear_pointer(&priv->search->needle, g_free);
	g_clear_pointer(&priv->search, g_free);

	g_clear_pointer(&priv->killring, jail_killring);
}

static void
gnt_entry_lost_focus(GntWidget *widget)
{
	GntEntry *entry = GNT_ENTRY(widget);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	destroy_suggest(priv);
	entry_redraw(widget);
}

static gboolean
gnt_entry_clicked(GntWidget *widget, GntMouseEvent event, G_GNUC_UNUSED int x,
                  G_GNUC_UNUSED int y)
{
	if (event == GNT_MIDDLE_MOUSE_DOWN) {
		clipboard_paste(GNT_BINDABLE(widget), NULL);
		return TRUE;
	}
	return FALSE;

}

static void
gnt_entry_class_init(GntEntryClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);
	char s[3] = {GNT_ESCAPE, erasechar(), 0};

	widget_class->clicked = gnt_entry_clicked;
	widget_class->destroy = gnt_entry_destroy;
	widget_class->draw = gnt_entry_draw;
	widget_class->map = gnt_entry_map;
	widget_class->size_request = gnt_entry_size_request;
	widget_class->key_pressed = gnt_entry_key_pressed;
	widget_class->lost_focus = gnt_entry_lost_focus;

	signals[SIG_TEXT_CHANGED] =
		g_signal_new("text_changed",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntEntryClass, text_changed),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 0);

	/**
	 * GntEntry::completion:
	 *
	 * Since: 2.1.0
	 */
	signals[SIG_COMPLETION] =
		g_signal_new("completion",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER);

	gnt_bindable_class_register_action(bindable, "cursor-home", move_start,
				GNT_KEY_CTRL_A, NULL);
	gnt_bindable_register_binding(bindable, "cursor-home", GNT_KEY_HOME, NULL);
	gnt_bindable_class_register_action(bindable, "cursor-end", move_end,
				GNT_KEY_CTRL_E, NULL);
	gnt_bindable_register_binding(bindable, "cursor-end", GNT_KEY_END, NULL);
	gnt_bindable_class_register_action(bindable, "delete-prev", backspace,
				GNT_KEY_BACKSPACE, NULL);
	gnt_bindable_register_binding(bindable, "delete-prev", s + 1, NULL);
	gnt_bindable_register_binding(bindable, "delete-prev", GNT_KEY_CTRL_H, NULL);
	gnt_bindable_class_register_action(bindable, "delete-next", delkey,
				GNT_KEY_DEL, NULL);
	gnt_bindable_register_binding(bindable, "delete-next", GNT_KEY_CTRL_D, NULL);
	gnt_bindable_class_register_action(bindable, "delete-start", del_to_home,
				GNT_KEY_CTRL_U, NULL);
	gnt_bindable_class_register_action(bindable, "delete-end", del_to_end,
				GNT_KEY_CTRL_K, NULL);
	gnt_bindable_class_register_action(bindable, "delete-prev-word", del_prev_word,
				GNT_KEY_CTRL_W, NULL);
	gnt_bindable_register_binding(bindable, "delete-prev-word", s, NULL);
	gnt_bindable_class_register_action(bindable, "cursor-prev-word", move_back_word,
				"\033" "b", NULL);
	gnt_bindable_class_register_action(bindable, "cursor-prev", move_back,
				GNT_KEY_LEFT, NULL);
	gnt_bindable_register_binding(bindable, "cursor-prev", GNT_KEY_CTRL_B, NULL);
	gnt_bindable_class_register_action(bindable, "cursor-next", move_forward,
				GNT_KEY_RIGHT, NULL);
	gnt_bindable_register_binding(bindable, "cursor-next", GNT_KEY_CTRL_F, NULL);
	gnt_bindable_class_register_action(bindable, "cursor-next-word", move_forward_word,
				"\033" "f", NULL);
	gnt_bindable_class_register_action(bindable, "delete-next-word", delete_forward_word,
				"\033" "d", NULL);
	gnt_bindable_class_register_action(bindable, "transpose-chars", transpose_chars,
				GNT_KEY_CTRL_T, NULL);
	gnt_bindable_class_register_action(bindable, "yank", entry_yank,
				GNT_KEY_CTRL_Y, NULL);
	gnt_bindable_class_register_action(bindable, "suggest-show", suggest_show,
				"\t", NULL);
	gnt_bindable_class_register_action(bindable, "suggest-next", suggest_next,
				GNT_KEY_DOWN, NULL);
	gnt_bindable_class_register_action(bindable, "suggest-prev", suggest_prev,
				GNT_KEY_UP, NULL);
	gnt_bindable_class_register_action(bindable, "suggest-next-page", suggest_next_page,
				GNT_KEY_PGDOWN, NULL);
	gnt_bindable_class_register_action(bindable, "suggest-prev-page", suggest_prev_page,
				GNT_KEY_PGUP, NULL);
	gnt_bindable_class_register_action(bindable, "history-next", history_next,
				GNT_KEY_CTRL_DOWN, NULL);
	gnt_bindable_class_register_action(bindable, "history-prev", history_prev,
				GNT_KEY_CTRL_UP, NULL);
	gnt_bindable_register_binding(bindable, "history-prev", GNT_KEY_CTRL_P, NULL);
	gnt_bindable_register_binding(bindable, "history-next", GNT_KEY_CTRL_N, NULL);
	gnt_bindable_class_register_action(bindable, "history-search", history_search,
				GNT_KEY_CTRL_R, NULL);
	gnt_bindable_class_register_action(bindable, "clipboard-paste", clipboard_paste,
				GNT_KEY_CTRL_V, NULL);

	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), GNT_BINDABLE_CLASS(klass));
}

static GntEntryKillRing *
new_killring(void)
{
	GntEntryKillRing *kr = g_new0(GntEntryKillRing, 1);
	kr->buffer = g_string_new(NULL);
	return kr;
}

static void
gnt_entry_init(GntEntry *entry)
{
	GntWidget *widget = GNT_WIDGET(entry);
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);

	priv->flag = GNT_ENTRY_FLAG_ALL;
	priv->max = 0;

	priv->histlength = 0;
	priv->history = NULL;

	priv->word = TRUE;
	priv->always = FALSE;
	priv->suggests = NULL;
	priv->killring = new_killring();
	priv->search = g_new0(GntEntrySearch, 1);

	gnt_widget_set_has_border(widget, FALSE);
	gnt_widget_set_has_shadow(widget, FALSE);
	gnt_widget_set_take_focus(widget, TRUE);
	gnt_widget_set_grow_x(widget, TRUE);

	gnt_widget_set_minimum_size(widget, 3, 1);
}

/******************************************************************************
 * GntEntry API
 *****************************************************************************/
GntWidget *gnt_entry_new(const char *text)
{
	GntWidget *widget = g_object_new(GNT_TYPE_ENTRY, NULL);
	GntEntry *entry = GNT_ENTRY(widget);

	gnt_entry_set_text_internal(entry, text);

	return widget;
}

static void
gnt_entry_set_text_internal(GntEntry *entry, const char *text)
{
	GntEntryPrivate *priv = gnt_entry_get_instance_private(entry);
	int len;
	int scroll, cursor;

	if (text && text[0])
	{
		len = strlen(text);
	}
	else
	{
		len = 0;
	}

	priv->buffer = len + 128;

	scroll = priv->scroll - priv->start;
	cursor = priv->end - priv->cursor;

	g_free(priv->start);
	priv->start = g_new0(char, priv->buffer);
	if (text) {
		snprintf(priv->start, len + 1, "%s", text);
	}
	priv->end = priv->start + len;

	if ((priv->scroll = priv->start + scroll) > priv->end) {
		priv->scroll = priv->end;
	}

	if ((priv->cursor = priv->end - cursor) > priv->end) {
		priv->cursor = priv->end;
	}

	if (gnt_widget_get_mapped(GNT_WIDGET(entry)))
		entry_redraw(GNT_WIDGET(entry));
}

void gnt_entry_set_text(GntEntry *entry, const char *text)
{
	GntEntryPrivate *priv = NULL;
	gboolean changed = TRUE;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	if (text == NULL && priv->start == NULL) {
		changed = FALSE;
	}
	if (text && priv->start && g_utf8_collate(text, priv->start) == 0) {
		changed = FALSE;
	}
	gnt_entry_set_text_internal(entry, text);
	if (changed)
		entry_text_changed(entry);
}

void gnt_entry_set_max(GntEntry *entry, int max)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	priv->max = max;
}

void gnt_entry_set_flag(GntEntry *entry, GntEntryFlag flag)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	priv->flag = flag;
	/* XXX: Check the existing string to make sure the flags are respected? */
}

const char *gnt_entry_get_text(GntEntry *entry)
{
	GntEntryPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_ENTRY(entry), NULL);
	priv = gnt_entry_get_instance_private(entry);

	return priv->start;
}

void gnt_entry_clear(GntEntry *entry)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	gnt_entry_set_text_internal(entry, NULL);
	priv->scroll = priv->cursor = priv->end = priv->start;
	entry_redraw(GNT_WIDGET(entry));
	destroy_suggest(priv);
	entry_text_changed(entry);
}

void gnt_entry_set_masked(GntEntry *entry, gboolean set)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	priv->masked = set;
}

void gnt_entry_add_to_history(GntEntry *entry, const char *text)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	/* Must have called set_history_length first */
	g_return_if_fail(priv->history != NULL);

	if (priv->histlength >= 0 &&
	    g_list_length(priv->history) >= (gsize)priv->histlength) {
		return;
	}

	priv->history = g_list_first(priv->history);
	g_free(priv->history->data);
	priv->history->data = g_strdup(text);
	priv->history = g_list_prepend(priv->history, NULL);
}

void gnt_entry_set_history_length(GntEntry *entry, int num)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	if (num == 0)
	{
		priv->histlength = num;
		if (priv->history) {
			priv->history = g_list_first(priv->history);
			g_list_free_full(priv->history, g_free);
			priv->history = NULL;
		}
		return;
	}

	if (priv->histlength == 0) {
		priv->histlength = num;
		priv->history = g_list_append(NULL, NULL);
		return;
	}

	if (num > 0 && num < priv->histlength) {
		GList *first, *iter;
		int index = 0;
		for (first = priv->history, index = 0; first->prev;
		     first = first->prev, index++) {
			/* Nothing. */
		}
		while ((iter = g_list_nth(first, num)) != NULL)
		{
			g_free(iter->data);
			first = g_list_delete_link(first, iter);
		}
		priv->histlength = num;
		if (index >= num)
			priv->history = g_list_last(first);
		return;
	}

	priv->histlength = num;
}

void gnt_entry_set_word_suggest(GntEntry *entry, gboolean word)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	priv->word = word;
}

void gnt_entry_set_always_suggest(GntEntry *entry, gboolean always)
{
	GntEntryPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	priv->always = always;
}

void gnt_entry_add_suggest(GntEntry *entry, const char *text)
{
	GntEntryPrivate *priv = NULL;
	GList *find;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	if (!text || !*text)
		return;

	find = g_list_find_custom(priv->suggests, text,
	                          (GCompareFunc)g_utf8_collate);
	if (find)
		return;
	priv->suggests = g_list_append(priv->suggests, g_strdup(text));
}

void gnt_entry_remove_suggest(GntEntry *entry, const char *text)
{
	GntEntryPrivate *priv = NULL;
	GList *find = NULL;

	g_return_if_fail(GNT_IS_ENTRY(entry));
	priv = gnt_entry_get_instance_private(entry);

	find = g_list_find_custom(priv->suggests, text,
	                          (GCompareFunc)g_utf8_collate);
	if (find)
	{
		g_free(find->data);
		priv->suggests = g_list_delete_link(priv->suggests, find);
	}
}
