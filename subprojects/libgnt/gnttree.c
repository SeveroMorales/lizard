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

#include "gntinternal.h"
#include "gntstyle.h"
#include "gnttree.h"
#include "gntutils.h"

#include "gntwidgetprivate.h"

#include <string.h>
#include <ctype.h>

#define SEARCH_TIMEOUT_S 4   /* 4 secs */
#define SEARCHING(priv) (priv->search && priv->search->len > 0)

#define COLUMN_INVISIBLE(priv, index) \
	(priv->columns[index].flags & GNT_TREE_COLUMN_INVISIBLE)
#define BINARY_DATA(priv, index) \
	(priv->columns[index].flags & GNT_TREE_COLUMN_BINARY_DATA)
#define RIGHT_ALIGNED(priv, index) \
	(priv->columns[index].flags & GNT_TREE_COLUMN_RIGHT_ALIGNED)

typedef enum
{
	GNT_TREE_COLUMN_INVISIBLE = 1 << 0,
	GNT_TREE_COLUMN_FIXED_SIZE = 1 << 1,
	GNT_TREE_COLUMN_BINARY_DATA = 1 << 2,
	GNT_TREE_COLUMN_RIGHT_ALIGNED = 1 << 3,
} GntTreeColumnFlag;

typedef struct _GntTreeColInfo
{
	int width;
	char *title;
	int width_ratio;
	GntTreeColumnFlag flags;
} GntTreeColInfo;

enum
{
	PROP_0,
	PROP_COLUMNS,
	PROP_EXPANDER,
};

enum
{
	SIG_SELECTION_CHANGED,
	SIG_SCROLLED,
	SIG_TOGGLED,
	SIG_COLLAPSED,
	SIGS,
};

typedef struct
{
	GntTreeRow *current; /* current selection */

	GntTreeRow *top;    /* The topmost visible item */
	GntTreeRow *bottom; /* The bottommost visible item */

	GntTreeRow *root; /* The root of all evil */

	GList *list;      /* List of GntTreeRow s */
	GHashTable *hash; /* We need this for quickly referencing the rows */

	int ncol;                /* No. of columns */
	GntTreeColInfo *columns; /* Would a GList be better? */
	gboolean show_title;
	gboolean show_separator; /* Whether to show column separators */

	GString *search;
	guint search_timeout;
	int search_column;
	gboolean (*search_func)(GntTree *tree, gpointer key, const char *search, const char *current);

	GCompareFunc compare;
	int lastvisible;
	int expander_level;
} GntTreePrivate;

#define	TAB_SIZE 3

/* XXX: Make this one into a GObject?
 * 		 ... Probably not */
struct _GntTreeRow
{
	int box_count;

	void *key;
	void *data;		/* XXX: unused */

	gboolean collapsed;
	gboolean choice;            /* Is this a choice-box?
	                               If choice is true, then child will be NULL */
	gboolean isselected;
	GntTextFormatFlags flags;
	int color;

	GntTreeRow *parent;
	GntTreeRow *child;
	GntTreeRow *next;
	GntTreeRow *prev;

	GList *columns;
	GntTree *tree;
};

typedef struct _GntTreeCol
{
	char *text;
	gboolean isbinary;
	int span;       /* How many columns does it span? */
} GntTreeCol;

static void tree_selection_changed(GntTree *, GntTreeRow *, GntTreeRow *);
static void _gnt_tree_init_internals(GntTree *tree, int col);

static guint signals[SIGS] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GntTree, gnt_tree, GNT_TYPE_WIDGET)

static void
readjust_columns(GntTree *tree)
{
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	int i, col, total;
	int width;
#define WIDTH(i) \
	(priv->columns[i].width_ratio ? priv->columns[i].width_ratio \
	                              : priv->columns[i].width)
	gnt_widget_get_size(GNT_WIDGET(tree), &width, NULL);
	if (gnt_widget_get_has_border(GNT_WIDGET(tree))) {
		width -= 2;
	}
	width -= 1;  /* Exclude the scrollbar from the calculation */
	for (i = 0, total = 0; i < priv->ncol; i++) {
		if (priv->columns[i].flags & GNT_TREE_COLUMN_INVISIBLE) {
			continue;
		}
		if (priv->columns[i].flags & GNT_TREE_COLUMN_FIXED_SIZE) {
			width -= WIDTH(i) + (priv->lastvisible != i);
		} else {
			total += WIDTH(i) + (priv->lastvisible != i);
		}
	}

	if (total == 0)
		return;

	for (i = 0; i < priv->ncol; i++) {
		if (priv->columns[i].flags & GNT_TREE_COLUMN_INVISIBLE) {
			continue;
		}
		if (priv->columns[i].flags & GNT_TREE_COLUMN_FIXED_SIZE) {
			col = WIDTH(i);
		} else {
			col = (WIDTH(i) * width) / total;
		}
		gnt_tree_set_col_width(tree, i, col);
	}
}

/* Move the item at position old to position new */
static GList *
g_list_reposition_child(GList *list, int old, int new)
{
	gpointer item = g_list_nth_data(list, old);
	list = g_list_remove(list, item);
	if (old < new)
		new--;   /* because the positions would have shifted after removing the item */
	list = g_list_insert(list, item, new);
	return list;
}

static GntTreeRow *
_get_next(GntTreeRow *row, gboolean godeep)
{
	if (row == NULL)
		return NULL;
	if (godeep && row->child)
		return row->child;
	if (row->next)
		return row->next;
	return _get_next(row->parent, FALSE);
}

static gboolean
row_matches_search(GntTreeRow *row)
{
	GntTree *tree = row->tree;
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	if (SEARCHING(priv)) {
		GntTreeCol *col;
		char *one, *two, *z;

		col = g_list_nth_data(row->columns, priv->search_column);
		if (!col) {
			col = row->columns->data;
		}

		if (priv->search_func) {
			return priv->search_func(tree, row->key,
			                         priv->search->str, col->text);
		}

		one = g_utf8_casefold(col->text, -1);
		two = g_utf8_casefold(priv->search->str, -1);
		z = strstr(one, two);
		g_free(one);
		g_free(two);
		if (z == NULL)
			return FALSE;
	}
	return TRUE;
}

static GntTreeRow *
get_next(GntTreeRow *row)
{
	if (row == NULL)
		return NULL;
	while ((row = _get_next(row, !row->collapsed)) != NULL) {
		if (row_matches_search(row))
			break;
	}
	return row;
}

/* Returns the n-th next row. If it doesn't exist, returns NULL */
static GntTreeRow *
get_next_n(GntTreeRow *row, int n)
{
	while (row && n--)
		row = get_next(row);
	return row;
}

/* Returns the n-th next row. If it doesn't exist, then the last non-NULL node */
static GntTreeRow *
get_next_n_opt(GntTreeRow *row, int n, int *pos)
{
	GntTreeRow *next = row;
	int r = 0;

	if (row == NULL)
		return NULL;

	while (row && n--)
	{
		row = get_next(row);
		if (row)
		{
			next = row;
			r++;
		}
	}

	if (pos)
		*pos = r;

	return next;
}

static GntTreeRow *
get_last_child(GntTreeRow *row)
{
	if (row == NULL)
		return NULL;
	if (!row->collapsed && row->child)
		row = row->child;
	else
		return row;

	while(row->next)
		row = row->next;
	return get_last_child(row);
}

static GntTreeRow *
get_prev(GntTreeRow *row)
{
	if (row == NULL)
		return NULL;
	while (row) {
		if (row->prev)
			row = get_last_child(row->prev);
		else
			row = row->parent;
		if (!row || row_matches_search(row))
			break;
	}
	return row;
}

static GntTreeRow *
get_prev_n(GntTreeRow *row, int n)
{
	while (row && n--)
		row = get_prev(row);
	return row;
}

/* Distance of row from the root */
/* XXX: This is uber-inefficient */
static int
get_root_distance(GntTreeRow *row)
{
	if (row == NULL)
		return -1;
	return get_root_distance(get_prev(row)) + 1;
}

/* Returns the distance between a and b.
 * If a is 'above' b, then the distance is positive */
static int
get_distance(GntTreeRow *a, GntTreeRow *b)
{
	/* First get the distance from a to the root.
	 * Then the distance from b to the root.
	 * Subtract.
	 * It's not that good, but it works. */
	int ha = get_root_distance(a);
	int hb = get_root_distance(b);

	return (hb - ha);
}

static int
find_depth(GntTreeRow *row)
{
	int dep = -1;

	while (row)
	{
		dep++;
		row = row->parent;
	}

	return dep;
}

static char *
update_row_text(GntTreePrivate *priv, GntTreeRow *row)
{
	GString *string = g_string_new(NULL);
	GList *iter;
	int i;
	gboolean notfirst = FALSE;

	for (i = 0, iter = row->columns; i < priv->ncol && iter;
	     i++, iter = iter->next) {
		GntTreeCol *col = iter->data;
		const char *text;
		int len;
		int fl = 0;
		gboolean cut = FALSE;
		int width;
		const char *display;

		if (COLUMN_INVISIBLE(priv, i)) {
			continue;
		}

		if (BINARY_DATA(priv, i)) {
			display = "";
		} else {
			display = col->text;
		}

		len = gnt_util_onscreen_width(display, NULL);

		width = priv->columns[i].width;

		if (i == 0)
		{
			if (row->choice)
			{
				g_string_append_printf(string, "[%c] ",
						row->isselected ? 'X' : ' ');
				fl = 4;
			} else if (find_depth(row) < priv->expander_level &&
			           row->child) {
				if (row->collapsed)
				{
					string = g_string_append(string, "+ ");
				}
				else
				{
					string = g_string_append(string, "- ");
				}
				fl = 2;
			} else {
				fl = TAB_SIZE * find_depth(row);
				g_string_append_printf(string, "%*s", fl, "");
			}
			len += fl;
		} else if (notfirst && priv->show_separator) {
			g_string_append_c(string, '|');
		} else {
			g_string_append_c(string, ' ');
		}

		notfirst = TRUE;

		if (len > width) {
			len = MAX(1, width - 1);
			cut = TRUE;
		}

		if (RIGHT_ALIGNED(priv, i) && len < priv->columns[i].width) {
			g_string_append_printf(string, "%*s", width - len - cut, "");
		}

		text = gnt_util_onscreen_width_to_pointer(display, len - fl, NULL);
		string = g_string_append_len(string, display, text - display);
		if (cut && width > 1) { /* ellipsis */
			if (gnt_ascii_only())
				g_string_append_c(string, '~');
			else
				string = g_string_append(string, "\342\200\246");
			len++;
		}

		if (!RIGHT_ALIGNED(priv, i) && len < priv->columns[i].width &&
		    iter->next) {
			g_string_append_printf(string, "%*s", width - len, "");
		}
	}
	return g_string_free(string, FALSE);
}

#define NEXT_X x += priv->columns[i].width + (i > 0 ? 1 : 0)

static void
tree_mark_columns(GntTree *tree, int pos, int y, chtype type)
{
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntWidget *widget = GNT_WIDGET(tree);
	int i;
	int x = pos;
	gboolean notfirst = FALSE;

	for (i = 0; i < priv->ncol - 1; i++) {
		if (!COLUMN_INVISIBLE(priv, i)) {
			notfirst = TRUE;
			NEXT_X;
		}
		if (!COLUMN_INVISIBLE(priv, i + 1) && notfirst) {
			mvwaddch(gnt_widget_get_window(widget), y, x, type);
		}
	}
}

static void
redraw_tree(GntTree *tree)
{
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	int start, i;
	GntWidget *widget = GNT_WIDGET(tree);
	WINDOW *window = gnt_widget_get_window(widget);
	GntTreeRow *row;
	gint width, height;
	int pos, up, down = 0;
	int rows, scrcol;
	int current = 0;

	if (!gnt_widget_get_mapped(GNT_WIDGET(tree)))
		return;

	gnt_widget_get_internal_size(widget, &width, &height);
	pos = gnt_widget_get_has_border(widget) ? 1 : 0;

	if (priv->top == NULL) {
		priv->top = priv->root;
	}
	if (priv->current == NULL && priv->root != NULL) {
		priv->current = priv->root;
		tree_selection_changed(tree, NULL, priv->current);
	}

	wbkgd(window, gnt_color_pair(GNT_COLOR_NORMAL));

	start = 0;
	if (priv->show_title) {
		int i;
		int x = pos;

		mvwhline(window, pos + 1, pos,
		         ACS_HLINE | gnt_color_pair(GNT_COLOR_NORMAL),
		         width - pos - 1);
		mvwhline(window, pos, pos,
		         ' ' | gnt_color_pair(GNT_COLOR_NORMAL),
		         width - pos - 1);

		for (i = 0; i < priv->ncol; i++) {
			if (COLUMN_INVISIBLE(priv, i)) {
				continue;
			}
			mvwaddnstr(window, pos, x + (x != pos),
			           priv->columns[i].title,
			           priv->columns[i].width);
			NEXT_X;
		}
		if (pos)
		{
			tree_mark_columns(
			        tree, pos, 0,
			        (priv->show_separator ? ACS_TTEE : ACS_HLINE) |
			                gnt_color_pair(GNT_COLOR_NORMAL));
			tree_mark_columns(
			        tree, pos, height - pos,
			        (priv->show_separator ? ACS_BTEE : ACS_HLINE) |
			                gnt_color_pair(GNT_COLOR_NORMAL));
		}
		tree_mark_columns(
		        tree, pos, pos + 1,
		        (priv->show_separator ? ACS_PLUS : ACS_HLINE) |
		                gnt_color_pair(GNT_COLOR_NORMAL));
		tree_mark_columns(tree, pos, pos,
		                  (priv->show_separator ? ACS_VLINE : ' ') |
		                          gnt_color_pair(GNT_COLOR_NORMAL));
		start = 2;
	}

	rows = height - pos * 2 - start - 1;
	priv->bottom = get_next_n_opt(priv->top, rows, &down);
	if (down < rows)
	{
		priv->top = get_prev_n(priv->bottom, rows);
		if (priv->top == NULL) {
			priv->top = priv->root;
		}
	}

	up = get_distance(priv->top, priv->current);
	if (up < 0)
		priv->top = priv->current;
	else if (up >= height - pos)
		priv->top = get_prev_n(priv->current, rows);

	if (priv->top && !row_matches_search(priv->top)) {
		priv->top = get_next(priv->top);
	}
	row = priv->top;
	/* exclude the borders and the scrollbar */
	scrcol = width - 1 - 2 * pos;

	if (priv->current && !row_matches_search(priv->current)) {
		GntTreeRow *old = priv->current;
		priv->current = priv->top;
		tree_selection_changed(tree, old, priv->current);
	}

	for (i = start + pos; row && i < height - pos;
	     i++, row = get_next(row)) {
		char *str;
		int wr;

		GntTextFormatFlags flags = row->flags;
		int attr = 0;

		if (!row_matches_search(row))
			continue;
		str = update_row_text(priv, row);

		if ((wr = gnt_util_onscreen_width(str, NULL)) > scrcol)
		{
			char *s = (char*)gnt_util_onscreen_width_to_pointer(str, scrcol, &wr);
			*s = '\0';
		}

		if (flags & GNT_TEXT_FLAG_BOLD)
			attr |= A_BOLD;
		if (flags & GNT_TEXT_FLAG_UNDERLINE)
			attr |= A_UNDERLINE;
		if (flags & GNT_TEXT_FLAG_BLINK)
			attr |= A_BLINK;

		if (row == priv->current) {
			current = i;
			attr |= A_BOLD;
			if (gnt_widget_has_focus(widget))
				attr |= gnt_color_pair(GNT_COLOR_HIGHLIGHT);
			else
				attr |= gnt_color_pair(GNT_COLOR_HIGHLIGHT_D);
		} else {
			if (flags & GNT_TEXT_FLAG_DIM)
				if (row->color)
					attr |= (A_DIM | gnt_color_pair(row->color));
				else
					attr |= (A_DIM | gnt_color_pair(GNT_COLOR_DISABLED));
			else if (flags & GNT_TEXT_FLAG_HIGHLIGHT)
				attr |= (A_DIM | gnt_color_pair(GNT_COLOR_HIGHLIGHT));
			else if (row->color)
				attr |= gnt_color_pair(row->color);
			else
				attr |= gnt_color_pair(GNT_COLOR_NORMAL);
		}

		wbkgdset(window, '\0' | attr);
		mvwaddstr(window, i, pos, C_(str));
		whline(window, ' ', scrcol - wr);
		priv->bottom = row;
		g_free(str);
		tree_mark_columns(tree, pos, i,
		                  (priv->show_separator ? ACS_VLINE : ' ') |
		                          attr);
	}

	wbkgdset(window, '\0' | gnt_color_pair(GNT_COLOR_NORMAL));
	while (i < height - pos) {
		mvwhline(window, i, pos, ' ', width - pos * 2 - 1);
		tree_mark_columns(tree, pos, i,
		                  (priv->show_separator ? ACS_VLINE : ' '));
		i++;
	}

	scrcol = width - pos - 1; /* position of the scrollbar */
	rows--;
	if (rows > 0)
	{
		int total = 0;
		int showing, position;

		get_next_n_opt(priv->root, g_list_length(priv->list), &total);
		showing = rows * rows / MAX(total, 1) + 1;
		showing = MIN(rows, showing);

		total -= rows;
		up = get_distance(priv->root, priv->top);
		down = total - up;

		position = (rows - showing) * up / MAX(1, up + down);
		position = MAX((priv->top != priv->root), position);

		if (showing + position > rows)
			position = rows - showing;

		if (showing + position == rows  && row)
			position = MAX(0, rows - 1 - showing);
		else if (showing + position < rows && !row)
			position = rows - showing;

		position += pos + start + 1;

		mvwvline(window, pos + start + 1, scrcol,
		         ' ' | gnt_color_pair(GNT_COLOR_NORMAL), rows);
		mvwvline(window, position, scrcol,
		         ACS_CKBOARD | gnt_color_pair(GNT_COLOR_HIGHLIGHT_D),
		         showing);
	}

	mvwaddch(window, start + pos, scrcol,
	         ((priv->top != priv->root) ? ACS_UARROW : ' ') |
	                 gnt_color_pair(GNT_COLOR_HIGHLIGHT_D));

	mvwaddch(window, height - pos - 1, scrcol,
	         (row ? ACS_DARROW : ' ') |
	                 gnt_color_pair(GNT_COLOR_HIGHLIGHT_D));

	/* If there's a search-text, show it in the bottom of the tree */
	if (SEARCHING(priv)) {
		const char *str = gnt_util_onscreen_width_to_pointer(
		        priv->search->str, scrcol - 1, NULL);
		wbkgdset(window, '\0' | gnt_color_pair(GNT_COLOR_HIGHLIGHT_D));
		mvwaddnstr(window, height - pos - 1, pos, priv->search->str,
		           str - priv->search->str);
	}
	wmove(window, current, pos);

	gnt_widget_queue_update(widget);
}

static void
gnt_tree_draw(GntWidget *widget)
{
	GntTree *tree = GNT_TREE(widget);

	redraw_tree(tree);
}

static void
gnt_tree_size_request(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);

	if (height == 0) {
		height = 10; /* XXX: Why?! */
	}

	if (width == 0) {
		GntTree *tree = GNT_TREE(widget);
		GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
		int i;
		width = gnt_widget_get_has_border(GNT_WIDGET(tree)) ? 3 : 1;
		for (i = 0; i < priv->ncol; i++) {
			if (!COLUMN_INVISIBLE(priv, i)) {
				width = width + priv->columns[i].width;
				if (priv->lastvisible != i) {
					width++;
				}
			}
		}
	}

	gnt_widget_set_internal_size(widget, width, height);
}

static void
gnt_tree_map(GntWidget *widget)
{
	GntTree *tree = GNT_TREE(widget);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
	priv->top = priv->root;
	priv->current = priv->root;
}

static void
tree_selection_changed(GntTree *tree, GntTreeRow *old, GntTreeRow *current)
{
	g_signal_emit(tree, signals[SIG_SELECTION_CHANGED], 0, old ? old->key : NULL,
				current ? current->key : NULL);
}

static gboolean
action_down(GntBindable *bind, G_GNUC_UNUSED GList *unused)
{
	int dist;
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *old = priv->current;
	GntTreeRow *row = get_next(priv->current);
	if (row == NULL)
		return FALSE;
	priv->current = row;
	if ((dist = get_distance(priv->current, priv->bottom)) < 0) {
		gnt_tree_scroll(tree, -dist);
	} else {
		redraw_tree(tree);
	}
	if (old != priv->current) {
		tree_selection_changed(tree, old, priv->current);
	}
	return TRUE;
}

static gboolean
action_move_parent(GntBindable *bind, G_GNUC_UNUSED GList *unused)
{
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *row = priv->current;
	int dist;

	if (!row || !row->parent || SEARCHING(priv)) {
		return FALSE;
	}

	priv->current = row->parent;
	if ((dist = get_distance(priv->current, priv->top)) > 0) {
		gnt_tree_scroll(tree, -dist);
	} else {
		redraw_tree(tree);
	}
	tree_selection_changed(tree, row, priv->current);
	return TRUE;
}

static gboolean
action_up(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	int dist;
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *old = priv->current;
	GntTreeRow *row = get_prev(priv->current);
	if (!row)
		return FALSE;
	priv->current = row;
	if ((dist = get_distance(priv->current, priv->top)) > 0) {
		gnt_tree_scroll(tree, -dist);
	} else {
		redraw_tree(tree);
	}
	if (old != priv->current) {
		tree_selection_changed(tree, old, priv->current);
	}

	return TRUE;
}

static gboolean
action_page_down(GntBindable *bind, G_GNUC_UNUSED GList *unused)
{
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *old = priv->current;
	GntTreeRow *row = get_next(priv->bottom);
	if (row)
	{
		int dist = get_distance(priv->top, priv->current);
		priv->top = priv->bottom;
		priv->current = get_next_n_opt(priv->top, dist, NULL);
		redraw_tree(tree);
	} else if (priv->current != priv->bottom) {
		priv->current = priv->bottom;
		redraw_tree(tree);
	}

	if (old != priv->current) {
		tree_selection_changed(tree, old, priv->current);
	}
	return TRUE;
}

static gboolean
action_page_up(GntBindable *bind, G_GNUC_UNUSED GList *unused)
{
	GntWidget *widget = GNT_WIDGET(bind);
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *row;
	GntTreeRow *old = priv->current;

	if (priv->top != priv->root) {
		int dist = get_distance(priv->top, priv->current);
		gint height;
		gnt_widget_get_internal_size(widget, NULL, &height);
		height -= 1 + priv->show_title * 2 +
		          (gnt_widget_get_has_border(widget) ? 2 : 0);
		row = get_prev_n(priv->top, height);
		if (row == NULL)
			row = priv->root;
		priv->top = row;
		priv->current = get_next_n_opt(priv->top, dist, NULL);
		redraw_tree(tree);
	} else if (priv->current != priv->top) {
		priv->current = priv->top;
		redraw_tree(tree);
	}
	if (old != priv->current) {
		tree_selection_changed(tree, old, priv->current);
	}
	return TRUE;
}

static void
end_search(GntTree *tree)
{
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	if (priv->search) {
		g_source_remove(priv->search_timeout);
		g_string_free(priv->search, TRUE);
		priv->search = NULL;
		priv->search_timeout = 0;
		gnt_widget_set_disable_actions(GNT_WIDGET(tree), FALSE);
	}
}

static gboolean
search_timeout(gpointer data)
{
	GntTree *tree = data;

	end_search(tree);
	redraw_tree(tree);

	return FALSE;
}

static gboolean
gnt_tree_key_pressed(GntWidget *widget, const char *text)
{
	GntTree *tree = GNT_TREE(widget);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	if (text[0] == '\r' || text[0] == '\n') {
		end_search(tree);
		gnt_widget_activate(widget);
	} else if (priv->search) {
		gboolean changed = TRUE;
		if (g_unichar_isprint(*text)) {
			priv->search = g_string_append_c(priv->search, *text);
		} else if (g_utf8_collate(text, GNT_KEY_BACKSPACE) == 0) {
			if (priv->search->len) {
				priv->search->str[--priv->search->len] = '\0';
			}
		} else
			changed = FALSE;
		if (changed) {
			redraw_tree(tree);
		} else {
			gnt_bindable_perform_action_key(GNT_BINDABLE(tree), text);
		}
		g_source_remove(priv->search_timeout);
		priv->search_timeout = g_timeout_add_seconds(
		        SEARCH_TIMEOUT_S, search_timeout, tree);
		return TRUE;
	} else if (text[0] == ' ' && text[1] == 0) {
		/* Space pressed */
		GntTreeRow *row = priv->current;
		if (row && row->child)
		{
			row->collapsed = !row->collapsed;
			redraw_tree(tree);
			g_signal_emit(tree, signals[SIG_COLLAPSED], 0, row->key, row->collapsed);
		}
		else if (row && row->choice)
		{
			row->isselected = !row->isselected;
			g_signal_emit(tree, signals[SIG_TOGGLED], 0, row->key);
			redraw_tree(tree);
		}
	} else {
		return FALSE;
	}

	return TRUE;
}

static void
gnt_tree_free_columns(GntTreePrivate *priv)
{
	int i;
	for (i = 0; i < priv->ncol; i++) {
		g_free(priv->columns[i].title);
	}
	priv->ncol = 0;
	g_clear_pointer(&priv->columns, g_free);
}

static void
gnt_tree_destroy(GntWidget *widget)
{
	GntTree *tree = GNT_TREE(widget);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	end_search(tree);
	g_clear_pointer(&priv->hash, g_hash_table_destroy);
	g_clear_pointer(&priv->list, g_list_free);
	gnt_tree_free_columns(priv);
}

static gboolean
gnt_tree_clicked(GntWidget *widget, GntMouseEvent event, G_GNUC_UNUSED int x,
                 int y)
{
	GntTree *tree = GNT_TREE(widget);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *old = priv->current;

	if (event == GNT_MOUSE_SCROLL_UP) {
		action_up(GNT_BINDABLE(widget), NULL);
	} else if (event == GNT_MOUSE_SCROLL_DOWN) {
		action_down(GNT_BINDABLE(widget), NULL);
	} else if (event == GNT_LEFT_MOUSE_DOWN) {
		GntTreeRow *row;
		gint widgety;
		int pos = gnt_widget_get_has_border(widget) ? 1 : 0;
		if (priv->show_title) {
			pos += 2;
		}
		gnt_widget_get_position(widget, NULL, &widgety);
		pos = y - widgety - pos;
		row = get_next_n(priv->top, pos);
		if (row && priv->current != row) {
			GntTreeRow *old = priv->current;
			priv->current = row;
			redraw_tree(tree);
			tree_selection_changed(tree, old, priv->current);
		} else if (row && row == priv->current) {
			if (row->choice) {
				row->isselected = !row->isselected;
				g_signal_emit(tree, signals[SIG_TOGGLED], 0, row->key);
				redraw_tree(tree);
			} else {
				gnt_widget_activate(widget);
			}
		}
	} else {
		return FALSE;
	}
	if (old != priv->current) {
		tree_selection_changed(tree, old, priv->current);
	}
	return TRUE;
}

static void
gnt_tree_size_changed(GntWidget *widget, G_GNUC_UNUSED int w,
                      G_GNUC_UNUSED int h)
{
	GntTree *tree = GNT_TREE(widget);
	gint width;

	gnt_widget_get_internal_size(widget, &width, NULL);
	if (width <= 0) {
		return;
	}

	readjust_columns(tree);
}

static gboolean
start_search(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntTree *tree = GNT_TREE(bindable);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	if (priv->search) {
		return FALSE;
	}

	gnt_widget_set_disable_actions(GNT_WIDGET(tree), TRUE);
	priv->search = g_string_new(NULL);
	priv->search_timeout =
	        g_timeout_add_seconds(SEARCH_TIMEOUT_S, search_timeout, tree);
	return TRUE;
}

static gboolean
end_search_action(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntTree *tree = GNT_TREE(bindable);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	if (priv->search == NULL) {
		return FALSE;
	}

	gnt_widget_set_disable_actions(GNT_WIDGET(tree), FALSE);
	end_search(tree);
	redraw_tree(tree);
	return TRUE;
}

static gboolean
move_first_action(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *row = priv->root;
	GntTreeRow *old = priv->current;
	if (row && !row_matches_search(row)) {
		row = get_next(row);
	}
	if (row) {
		priv->current = row;
		redraw_tree(tree);
		if (old != priv->current)
			tree_selection_changed(tree, old, priv->current);
	}

	return TRUE;
}

static gboolean
move_last_action(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntTree *tree = GNT_TREE(bind);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntTreeRow *old = priv->current;
	GntTreeRow *row = priv->bottom;
	GntTreeRow *next;

	while ((next = get_next(row)))
		row = next;

	if (row) {
		priv->current = row;
		redraw_tree(tree);
		if (old != priv->current) {
			tree_selection_changed(tree, old, priv->current);
		}
	}

	return TRUE;
}

static void
gnt_tree_set_property(GObject *obj, guint prop_id, const GValue *value,
                      G_GNUC_UNUSED GParamSpec *spec)
{
	GntTree *tree = GNT_TREE(obj);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	switch (prop_id) {
		case PROP_COLUMNS:
			_gnt_tree_init_internals(tree, g_value_get_int(value));
			break;
		case PROP_EXPANDER:
			if (priv->expander_level == g_value_get_int(value)) {
				break;
			}
			priv->expander_level = g_value_get_int(value);
		default:
			break;
	}
}

static void
gnt_tree_get_property(GObject *obj, guint prop_id, GValue *value,
                      G_GNUC_UNUSED GParamSpec *spec)
{
	GntTree *tree = GNT_TREE(obj);
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	switch (prop_id) {
		case PROP_COLUMNS:
			g_value_set_int(value, priv->ncol);
			break;
		case PROP_EXPANDER:
			g_value_set_int(value, priv->expander_level);
			break;
		default:
			break;
	}
}

static void
gnt_tree_class_init(GntTreeClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->destroy = gnt_tree_destroy;
	widget_class->draw = gnt_tree_draw;
	widget_class->map = gnt_tree_map;
	widget_class->size_request = gnt_tree_size_request;
	widget_class->key_pressed = gnt_tree_key_pressed;
	widget_class->clicked = gnt_tree_clicked;
	widget_class->size_changed = gnt_tree_size_changed;

	obj_class->set_property = gnt_tree_set_property;
	obj_class->get_property = gnt_tree_get_property;
	g_object_class_install_property(obj_class,
			PROP_COLUMNS,
			g_param_spec_int("columns", "Columns",
				"Number of columns in the tree.",
				1, G_MAXINT, 1,
				G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS
			)
		);
	g_object_class_install_property(obj_class,
			PROP_EXPANDER,
			g_param_spec_int("expander-level", "Expander level",
				"Number of levels to show expander in the tree.",
				0, G_MAXINT, 1,
				G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS
			)
		);

	signals[SIG_SELECTION_CHANGED] =
		g_signal_new("selection-changed",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntTreeClass, selection_changed),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER);
	signals[SIG_SCROLLED] =
		g_signal_new("scrolled",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_INT);
	signals[SIG_TOGGLED] =
		g_signal_new("toggled",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntTreeClass, toggled),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);
	signals[SIG_COLLAPSED] =
		g_signal_new("collapse-toggled",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_BOOLEAN);

	gnt_bindable_class_register_action(bindable, "move-up", action_up,
				GNT_KEY_UP, NULL);
	gnt_bindable_register_binding(bindable, "move-up", GNT_KEY_CTRL_P, NULL);
	gnt_bindable_class_register_action(bindable, "move-down", action_down,
				GNT_KEY_DOWN, NULL);
	gnt_bindable_register_binding(bindable, "move-down", GNT_KEY_CTRL_N, NULL);
	gnt_bindable_class_register_action(bindable, "move-parent", action_move_parent,
				GNT_KEY_BACKSPACE, NULL);
	gnt_bindable_class_register_action(bindable, "page-up", action_page_up,
				GNT_KEY_PGUP, NULL);
	gnt_bindable_class_register_action(bindable, "page-down", action_page_down,
				GNT_KEY_PGDOWN, NULL);
	gnt_bindable_class_register_action(bindable, "start-search", start_search,
				"/", NULL);
	gnt_bindable_class_register_action(bindable, "end-search", end_search_action,
				"\033", NULL);
	gnt_bindable_class_register_action(bindable, "move-first", move_first_action,
			GNT_KEY_HOME, NULL);
	gnt_bindable_class_register_action(bindable, "move-last", move_last_action,
			GNT_KEY_END, NULL);

	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), bindable);
}

static void
gnt_tree_init(GntTree *tree)
{
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);
	GntWidget *widget = GNT_WIDGET(tree);

	priv->show_separator = TRUE;

	gnt_widget_set_grow_x(widget, TRUE);
	gnt_widget_set_grow_y(widget, TRUE);
	gnt_widget_set_has_shadow(widget, FALSE);
	gnt_widget_set_take_focus(widget, TRUE);
	gnt_widget_set_minimum_size(widget, 4, 1);
}

/******************************************************************************
 * GntTree API
 *****************************************************************************/
static void
free_tree_col(gpointer data)
{
	GntTreeCol *col = data;
	if (!col->isbinary)
		g_free(col->text);
	g_free(col);
}

static void
free_tree_row(gpointer data)
{
	GntTreeRow *row = data;

	if (!row)
		return;

	g_list_free_full(row->columns, free_tree_col);
	g_free(row);
}

GntWidget *
gnt_tree_new(void)
{
	return gnt_tree_new_with_columns(1);
}

void gnt_tree_set_visible_rows(GntTree *tree, int rows)
{
	GntWidget *widget = GNT_WIDGET(tree);
	gint width, height;

	height = rows;
	if (gnt_widget_get_has_border(widget)) {
		height += 2;
	}

	gnt_widget_get_internal_size(widget, &width, NULL);
	gnt_widget_set_internal_size(widget, width, height);
}

int gnt_tree_get_visible_rows(GntTree *tree)
{
	GntWidget *widget = GNT_WIDGET(tree);
	gint ret;

	gnt_widget_get_internal_size(widget, NULL, &ret);
	if (gnt_widget_get_has_border(widget)) {
		ret -= 2;
	}
	return ret;
}

GList *gnt_tree_get_rows(GntTree *tree)
{
	GntTreePrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	return priv->list;
}

void gnt_tree_scroll(GntTree *tree, int count)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	if (count < 0)
	{
		if (get_root_distance(priv->top) == 0) {
			return;
		}
		row = get_prev_n(priv->top, -count);
		if (row == NULL)
			row = priv->root;
		priv->top = row;
	}
	else
	{
		get_next_n_opt(priv->bottom, count, &count);
		priv->top = get_next_n(priv->top, count);
	}

	redraw_tree(tree);
	g_signal_emit(tree, signals[SIG_SCROLLED], 0, count);
}

static gpointer
find_position(GntTreePrivate *priv, gpointer key, gpointer parent)
{
	GntTreeRow *row;

	if (priv->compare == NULL) {
		return NULL;
	}

	if (parent == NULL)
		row = priv->root;
	else
		row = g_hash_table_lookup(priv->hash, parent);

	if (!row)
		return NULL;

	if (parent)
		row = row->child;

	while (row)
	{
		if (priv->compare(key, row->key) < 0) {
			return (row->prev ? row->prev->key : NULL);
		}
		if (row->next)
			row = row->next;
		else
			return row->key;
	}
	return NULL;
}

void gnt_tree_sort_row(GntTree *tree, gpointer key)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row, *q, *s;
	int current, newp;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	if (!priv->compare) {
		return;
	}

	row = g_hash_table_lookup(priv->hash, key);
	g_return_if_fail(row != NULL);

	current = g_list_index(priv->list, key);

	if (row->parent)
		s = row->parent->child;
	else
		s = priv->root;

	q = NULL;
	while (s) {
		if (priv->compare(row->key, s->key) < 0) {
			break;
		}
		q = s;
		s = s->next;
	}

	/* Move row between q and s */
	if (row == q || row == s)
		return;

	if (q == NULL) {
		/* row becomes the first child of its parent */
		row->prev->next = row->next;  /* row->prev cannot be NULL at this point */
		if (row->next)
			row->next->prev = row->prev;
		if (row->parent)
			row->parent->child = row;
		else
			priv->root = row;
		row->next = s;
		g_return_if_fail(s != NULL); /* s cannot be NULL */
		s->prev = row;
		row->prev = NULL;
		newp = g_list_index(priv->list, s) - 1;
	} else {
		if (row->prev) {
			row->prev->next = row->next;
		} else {
			/* row was the first child of its parent */
			if (row->parent)
				row->parent->child = row->next;
			else
				priv->top = row->next;
		}

		if (row->next)
			row->next->prev = row->prev;

		q->next = row;
		row->prev = q;
		if (s)
			s->prev = row;
		row->next = s;
		newp = g_list_index(priv->list, q) + 1;
	}
	priv->list = g_list_reposition_child(priv->list, current, newp);

	redraw_tree(tree);
}

GntTreeRow *gnt_tree_add_row_after(GntTree *tree, void *key, GntTreeRow *row, void *parent, void *bigbro)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *pr = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	if (g_hash_table_lookup(priv->hash, key)) {
		gnt_tree_remove(tree, key);
	}

	row->tree = tree;
	row->key = key;
	row->data = NULL;
	g_hash_table_replace(priv->hash, key, row);

	if (bigbro == NULL && priv->compare) {
		bigbro = find_position(priv, key, parent);
	}

	if (priv->root == NULL) {
		priv->root = row;
		priv->list = g_list_prepend(priv->list, key);
	} else {
		int position = 0;

		if (bigbro)
		{
			pr = g_hash_table_lookup(priv->hash, bigbro);
			if (pr)
			{
				if (pr->next)	pr->next->prev = row;
				row->next = pr->next;
				row->prev = pr;
				pr->next = row;
				row->parent = pr->parent;

				position = g_list_index(priv->list, bigbro);
			}
		}

		if (pr == NULL && parent)
		{
			pr = g_hash_table_lookup(priv->hash, parent);
			if (pr)
			{
				if (pr->child)	pr->child->prev = row;
				row->next = pr->child;
				pr->child = row;
				row->parent = pr;

				position = g_list_index(priv->list, parent);
			}
		}

		if (pr == NULL)
		{
			GntTreeRow *r = priv->root;
			row->next = r;
			r->prev = row;
			if (priv->current == priv->root) {
				priv->current = row;
			}
			priv->root = row;
			priv->list = g_list_prepend(priv->list, key);
		}
		else
		{
			priv->list =
			        g_list_insert(priv->list, key, position + 1);
		}
	}
	redraw_tree(tree);

	return row;
}

GntTreeRow *gnt_tree_add_row_last(GntTree *tree, void *key, GntTreeRow *row, void *parent)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *pr = NULL, *br = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	if (parent)
		pr = g_hash_table_lookup(priv->hash, parent);

	if (pr)
		br = pr->child;
	else
		br = priv->root;

	if (br)
	{
		while (br->next)
			br = br->next;
	}

	return gnt_tree_add_row_after(tree, key, row, parent, br ? br->key : NULL);
}

gpointer gnt_tree_get_selection_data(GntTree *tree)
{
	GntTreePrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	if (priv->current) {
		/* XXX: perhaps we should just get rid of 'data' */
		return priv->current->key;
	}
	return NULL;
}

char *gnt_tree_get_selection_text(GntTree *tree)
{
	GntTreePrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	if (priv->current) {
		return update_row_text(priv, priv->current);
	}
	return NULL;
}

GList *gnt_tree_get_row_text_list(GntTree *tree, gpointer key)
{
	GntTreePrivate *priv = NULL;
	GList *list = NULL, *iter;
	GntTreeRow *row = NULL;
	int i;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	row = key ? g_hash_table_lookup(priv->hash, key) : priv->current;

	if (!row)
		return NULL;

	for (i = 0, iter = row->columns; i < priv->ncol && iter;
	     i++, iter = iter->next) {
		GntTreeCol *col = iter->data;
		list = g_list_append(list, BINARY_DATA(priv, i)
		                                   ? col->text
		                                   : g_strdup(col->text));
	}

	return list;
}

GList *gnt_tree_get_selection_text_list(GntTree *tree)
{
	return gnt_tree_get_row_text_list(tree, NULL);
}

void gnt_tree_remove(GntTree *tree, gpointer key)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row = NULL;
	static int depth = 0; /* Only redraw after all child nodes are removed */

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);
	if (row)
	{
		gboolean redraw = FALSE;

		if (row->child) {
			depth++;
			while (row->child) {
				gnt_tree_remove(tree, row->child->key);
			}
			depth--;
		}

		if (get_distance(priv->top, row) >= 0 &&
		    get_distance(row, priv->bottom) >= 0) {
			redraw = TRUE;
		}

		/* Update root/top/current/bottom if necessary */
		if (priv->root == row) {
			priv->root = get_next(row);
		}
		if (priv->top == row) {
			if (priv->top != priv->root) {
				priv->top = get_prev(row);
			} else {
				priv->top = get_next(row);
			}
		}
		if (priv->current == row) {
			if (priv->current != priv->root) {
				priv->current = get_prev(row);
			} else {
				priv->current = get_next(row);
			}
			tree_selection_changed(tree, row, priv->current);
		}
		if (priv->bottom == row) {
			priv->bottom = get_prev(row);
		}

		/* Fix the links */
		if (row->next)
			row->next->prev = row->prev;
		if (row->parent && row->parent->child == row)
			row->parent->child = row->next;
		if (row->prev)
			row->prev->next = row->next;

		g_hash_table_remove(priv->hash, key);
		priv->list = g_list_remove(priv->list, key);

		if (redraw && depth == 0)
		{
			redraw_tree(tree);
		}
	}
}

void gnt_tree_remove_all(GntTree *tree)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	priv->root = NULL;
	g_hash_table_remove_all(priv->hash);
	g_list_free(priv->list);
	priv->list = NULL;
	priv->current = priv->top = priv->bottom = NULL;
}

int gnt_tree_get_selection_visible_line(GntTree *tree)
{
	GntTreePrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), 0);
	priv = gnt_tree_get_instance_private(tree);

	return get_distance(priv->top, priv->current) +
	       !gnt_widget_get_has_border(GNT_WIDGET(tree));
}

void gnt_tree_change_text(GntTree *tree, gpointer key, int colno, const char *text)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;
	GntTreeCol *col;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(colno < priv->ncol);

	row = g_hash_table_lookup(priv->hash, key);
	if (row)
	{
		col = g_list_nth_data(row->columns, colno);
		if (BINARY_DATA(priv, colno)) {
			col->text = (gpointer)text;
		} else {
			g_free(col->text);
			col->text = g_strdup(text ? text : "");
		}

		if (gnt_widget_get_mapped(GNT_WIDGET(tree)) &&
		    get_distance(priv->top, row) >= 0 &&
		    get_distance(row, priv->bottom) >= 0) {
			redraw_tree(tree);
		}
	}
}

GntTreeRow *gnt_tree_add_choice(GntTree *tree, void *key, GntTreeRow *row, void *parent, void *bigbro)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *r;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	r = g_hash_table_lookup(priv->hash, key);
	g_return_val_if_fail(!r || !r->choice, NULL);

	if (bigbro == NULL) {
		if (priv->compare) {
			bigbro = find_position(priv, key, parent);
		} else {
			r = g_hash_table_lookup(priv->hash, parent);
			if (!r)
				r = priv->root;
			else
				r = r->child;
			if (r) {
				while (r->next)
					r = r->next;
				bigbro = r->key;
			}
		}
	}
	row = gnt_tree_add_row_after(tree, key, row, parent, bigbro);
	row->choice = TRUE;

	return row;
}

void gnt_tree_set_choice(GntTree *tree, void *key, gboolean set)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);

	if (!row)
		return;
	g_return_if_fail(row->choice);

	row->isselected = set;
	redraw_tree(tree);
}

gboolean gnt_tree_get_choice(GntTree *tree, void *key)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_val_if_fail(GNT_IS_TREE(tree), FALSE);
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);

	if (!row)
		return FALSE;
	g_return_val_if_fail(row->choice, FALSE);

	return row->isselected;
}

void gnt_tree_set_row_flags(GntTree *tree, void *key, GntTextFormatFlags flags)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);
	if (!row || row->flags == flags)
		return;

	row->flags = flags;
	redraw_tree(tree);	/* XXX: It shouldn't be necessary to redraw the whole darned tree */
}

void gnt_tree_set_row_color(GntTree *tree, void *key, int color)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);
	if (!row || row->color == color)
		return;

	row->color = color;
	redraw_tree(tree);
}

void gnt_tree_set_selected(GntTree *tree , void *key)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;
	int dist;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);
	if (!row || row == priv->current) {
		return;
	}

	if (priv->top == NULL) {
		priv->top = row;
	}
	if (priv->bottom == NULL) {
		priv->bottom = row;
	}

	priv->current = row;
	if ((dist = get_distance(priv->current, priv->bottom)) < 0) {
		gnt_tree_scroll(tree, -dist);
	} else if ((dist = get_distance(priv->current, priv->top)) > 0) {
		gnt_tree_scroll(tree, -dist);
	} else {
		redraw_tree(tree);
	}
	tree_selection_changed(tree, row, priv->current);
}

/* Internal. */
GntTreeRow *
gnt_tree_get_current(GntTree *tree)
{
	GntTreePrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);
	return priv->current;
}

/* Internal. */
GntTreeRow *
gnt_tree_get_top(GntTree *tree)
{
	GntTreePrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);
	return priv->top;
}

static void _gnt_tree_init_internals(GntTree *tree, int col)
{
	GntTreePrivate *priv = gnt_tree_get_instance_private(tree);

	gnt_tree_free_columns(priv);

	priv->ncol = col;
	priv->hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
	                                   free_tree_row);
	priv->columns = g_new0(struct _GntTreeColInfo, col);
	priv->lastvisible = col - 1;
	while (col--)
	{
		priv->columns[col].width = 15;
	}
	priv->list = NULL;
	priv->show_title = FALSE;
	g_object_notify(G_OBJECT(tree), "columns");
}

GntWidget *gnt_tree_new_with_columns(int col)
{
	GntWidget *widget = g_object_new(GNT_TYPE_TREE,
			"columns", col,
			"expander-level", 1,
			NULL);

	return widget;
}

GntTreeRow *gnt_tree_create_row_from_list(GntTree *tree, GList *list)
{
	GntTreePrivate *priv = NULL;
	GList *iter;
	int i;
	GntTreeRow *row;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	row = g_new0(GntTreeRow, 1);
	for (i = 0, iter = list; i < priv->ncol && iter;
	     iter = iter->next, i++) {
		GntTreeCol *col = g_new0(GntTreeCol, 1);
		col->span = 1;
		if (BINARY_DATA(priv, i)) {
			col->text = iter->data;
			col->isbinary = TRUE;
		} else {
			col->text = g_strdup(iter->data ? iter->data : "");
			col->isbinary = FALSE;
		}

		row->columns = g_list_append(row->columns, col);
	}

	return row;
}

GntTreeRow *gnt_tree_create_row(GntTree *tree, ...)
{
	GntTreePrivate *priv = NULL;
	int i;
	va_list args;
	GList *list = NULL;
	GntTreeRow *row;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	va_start(args, tree);
	for (i = 0; i < priv->ncol; i++) {
		list = g_list_append(list, va_arg(args, char *));
	}
	va_end(args);

	row = gnt_tree_create_row_from_list(tree, list);
	g_list_free(list);

	return row;
}

void gnt_tree_set_col_width(GntTree *tree, int col, int width)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(col < priv->ncol);

	priv->columns[col].width = width;
	if (priv->columns[col].width_ratio == 0) {
		priv->columns[col].width_ratio = width;
	}
}

void gnt_tree_set_column_title(GntTree *tree, int index, const char *title)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	g_free(priv->columns[index].title);
	priv->columns[index].title = g_strdup(title);
}

void gnt_tree_set_column_titles(GntTree *tree, ...)
{
	GntTreePrivate *priv = NULL;
	int i;
	va_list args;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	va_start(args, tree);
	for (i = 0; i < priv->ncol; i++) {
		const char *title = va_arg(args, const char *);
		priv->columns[i].title = g_strdup(title);
	}
	va_end(args);
}

void gnt_tree_set_show_title(GntTree *tree, gboolean set)
{
	GntTreePrivate *priv = NULL;
	gint minw;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	priv->show_title = set;
	gnt_widget_get_minimum_size(GNT_WIDGET(tree), &minw, NULL);
	gnt_widget_set_minimum_size(GNT_WIDGET(tree), minw, set ? 6 : 4);
}

void gnt_tree_set_compare_func(GntTree *tree, GCompareFunc func)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	priv->compare = func;
}

void gnt_tree_set_expanded(GntTree *tree, void *key, gboolean expanded)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);
	if (row) {
		row->collapsed = !expanded;
		if (gnt_widget_get_window(GNT_WIDGET(tree))) {
			gnt_widget_draw(GNT_WIDGET(tree));
		}
		g_signal_emit(tree, signals[SIG_COLLAPSED], 0, key, row->collapsed);
	}
}

void gnt_tree_set_show_separator(GntTree *tree, gboolean set)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	priv->show_separator = set;
}

void gnt_tree_adjust_columns(GntTree *tree)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;
	int *widths, i, twidth;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	widths = g_new0(int, priv->ncol);
	row = priv->root;
	while (row) {
		GList *iter;
		for (i = 0, iter = row->columns; iter; iter = iter->next, i++) {
			GntTreeCol *col = iter->data;
			int w = gnt_util_onscreen_width(col->text, NULL);
			if (i == 0 && row->choice)
				w += 4;
			if (i == 0) {
				w += find_depth(row) * TAB_SIZE;
			}
			if (widths[i] < w)
				widths[i] = w;
		}
		row = get_next(row);
	}

	twidth = gnt_widget_get_has_border(GNT_WIDGET(tree)) ? 3 : 1;
	for (i = 0; i < priv->ncol; i++) {
		if (priv->columns[i].flags & GNT_TREE_COLUMN_FIXED_SIZE) {
			widths[i] = priv->columns[i].width;
		}
		gnt_tree_set_col_width(tree, i, widths[i]);
		if (!COLUMN_INVISIBLE(priv, i)) {
			twidth = twidth + widths[i];
			if (priv->lastvisible != i) {
				twidth += 1;
			}
		}
	}
	g_free(widths);

	gnt_widget_set_size(GNT_WIDGET(tree), twidth, -1);
}

void
gnt_tree_set_hash_fns(GntTree *tree, GHashFunc hash, GEqualFunc eq,
                      GDestroyNotify kd)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	g_hash_table_destroy(priv->hash);
	priv->hash = g_hash_table_new_full(hash, eq, kd, free_tree_row);
}

static void
set_column_flag(GntTreePrivate *priv, int col, GntTreeColumnFlag flag, gboolean set)
{
	if (set)
		priv->columns[col].flags |= flag;
	else
		priv->columns[col].flags &= ~flag;
}

void gnt_tree_set_column_visible(GntTree *tree, int col, gboolean vis)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(col < priv->ncol);

	set_column_flag(priv, col, GNT_TREE_COLUMN_INVISIBLE, !vis);
	if (vis) {
		/* the column is visible */
		if (priv->lastvisible < col) {
			priv->lastvisible = col;
		}
	} else {
		if (priv->lastvisible == col) {
			while (priv->lastvisible) {
				priv->lastvisible--;
				if (!COLUMN_INVISIBLE(priv,
				                      priv->lastvisible)) {
					break;
				}
			}
		}
	}
	if (gnt_widget_get_mapped(GNT_WIDGET(tree)))
		readjust_columns(tree);
}

void gnt_tree_set_column_resizable(GntTree *tree, int col, gboolean res)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(col < priv->ncol);

	set_column_flag(priv, col, GNT_TREE_COLUMN_FIXED_SIZE, !res);
}

void gnt_tree_set_column_is_binary(GntTree *tree, int col, gboolean bin)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(col < priv->ncol);

	set_column_flag(priv, col, GNT_TREE_COLUMN_BINARY_DATA, bin);
}

void gnt_tree_set_column_is_right_aligned(GntTree *tree, int col, gboolean right)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(col < priv->ncol);

	set_column_flag(priv, col, GNT_TREE_COLUMN_RIGHT_ALIGNED, right);
}

void gnt_tree_set_column_width_ratio(GntTree *tree, int cols[])
{
	GntTreePrivate *priv = NULL;
	int i;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	for (i = 0; i < priv->ncol && cols[i]; i++) {
		priv->columns[i].width_ratio = cols[i];
	}
}

void gnt_tree_set_search_column(GntTree *tree, int col)
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);
	g_return_if_fail(col < priv->ncol);
	g_return_if_fail(!BINARY_DATA(priv, col));

	priv->search_column = col;
}

gboolean gnt_tree_is_searching(GntTree *tree)
{
	GntTreePrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_TREE(tree), FALSE);
	priv = gnt_tree_get_instance_private(tree);

	return (priv->search != NULL);
}

void gnt_tree_set_search_function(GntTree *tree,
		gboolean (*func)(GntTree *tree, gpointer key, const char *search, const char *current))
{
	GntTreePrivate *priv = NULL;

	g_return_if_fail(GNT_IS_TREE(tree));
	priv = gnt_tree_get_instance_private(tree);

	priv->search_func = func;
}

gpointer gnt_tree_get_parent_key(GntTree *tree, gpointer key)
{
	GntTreePrivate *priv = NULL;
	GntTreeRow *row;

	g_return_val_if_fail(GNT_IS_TREE(tree), NULL);
	priv = gnt_tree_get_instance_private(tree);

	row = g_hash_table_lookup(priv->hash, key);
	return (row && row->parent) ? row->parent->key : NULL;
}

gpointer gnt_tree_row_get_key(GntTree *tree, GntTreeRow *row)
{
	g_return_val_if_fail(row && row->tree == tree, NULL);
	return row->key;
}

GntTreeRow * gnt_tree_row_get_next(GntTree *tree, GntTreeRow *row)
{
	g_return_val_if_fail(row && row->tree == tree, NULL);
	return row->next;
}

GntTreeRow * gnt_tree_row_get_prev(GntTree *tree, GntTreeRow *row)
{
	g_return_val_if_fail(row && row->tree == tree, NULL);
	return row->prev;
}

GntTreeRow * gnt_tree_row_get_child(GntTree *tree, GntTreeRow *row)
{
	g_return_val_if_fail(row && row->tree == tree, NULL);
	return row->child;
}

GntTreeRow * gnt_tree_row_get_parent(GntTree *tree, GntTreeRow *row)
{
	g_return_val_if_fail(row && row->tree == tree, NULL);
	return row->parent;
}

/**************************************************************************
 * GntTreeRow GBoxed API
 **************************************************************************/
static GntTreeRow *
gnt_tree_row_ref(GntTreeRow *row)
{
	g_return_val_if_fail(row != NULL, NULL);

	row->box_count++;

	return row;
}

static void
gnt_tree_row_unref(GntTreeRow *row)
{
	g_return_if_fail(row != NULL);
	g_return_if_fail(row->box_count >= 0);

	if (!row->box_count--)
		free_tree_row(row);
}

GType
gnt_tree_row_get_type(void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static("GntTreeRow",
				(GBoxedCopyFunc)gnt_tree_row_ref,
				(GBoxedFreeFunc)gnt_tree_row_unref);
	}

	return type;
}
