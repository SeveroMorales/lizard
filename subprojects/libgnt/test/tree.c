#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gnt.h>

int
main(void)
{
	GntWidget *vbox, *tree;
	GntTreeRow *row;

#ifdef STANDALONE
	freopen(".error", "w", stderr);

	gnt_init();
#endif

	vbox = gnt_box_new(FALSE, TRUE);
	gnt_widget_set_name(vbox, "vbox");
	gnt_box_set_toplevel(GNT_BOX(vbox), TRUE);
	gnt_box_set_fill(GNT_BOX(vbox), FALSE);
	gnt_box_set_title(GNT_BOX(vbox), "Tree test");
	gnt_box_set_alignment(GNT_BOX(vbox), GNT_ALIGN_MID);

	tree = gnt_tree_new();
	gnt_widget_set_has_border(tree, FALSE);
	gnt_box_add_widget(GNT_BOX(vbox), tree);
	row = gnt_tree_create_row(GNT_TREE(tree), "foo");
	gnt_tree_add_row_last(GNT_TREE(tree), GINT_TO_POINTER(1), row, NULL);
	row = gnt_tree_create_row(GNT_TREE(tree), "bar");
	gnt_tree_add_row_last(GNT_TREE(tree), GINT_TO_POINTER(2), row, NULL);
	row = gnt_tree_create_row(GNT_TREE(tree), "baz");
	gnt_tree_add_row_last(GNT_TREE(tree), GINT_TO_POINTER(3), row, NULL);

	gnt_widget_show(vbox);

#ifdef STANDALONE
	gnt_main();

	gnt_quit();
#endif

	return 0;
}

