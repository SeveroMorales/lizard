#include "gntbutton.h"
#include "gntkeys.h"
#include "gnttree.h"
#include "gntbox.h"

static gboolean
key_pressed(GntWidget *widget, const char *text, gpointer null)
{
	GntWidget *w = null;
	GntWidget *box = gnt_box_new(FALSE, FALSE);
	GntWidget *label = gnt_label_new("so wassup!!");

	gnt_box_add_widget(GNT_BOX(box), label);
	gnt_widget_set_has_border(box, TRUE);
	gnt_widget_set_has_shadow(box, TRUE);
	gnt_box_set_title(GNT_BOX(box), "This is a test");

	gnt_widget_show(box);

	return FALSE;
}

static void
button1(GntWidget *widget, gpointer null)
{
	printf("OLAAA");
	gnt_widget_destroy(null);
}

static void
button2(GntWidget *widget, gpointer null)
{
	printf("BOOYAA");
}

static gboolean
w_scroll(GntWidget *tree)
{
	g_return_val_if_fail(GNT_IS_TREE(tree), FALSE);
	gnt_tree_scroll(GNT_TREE(tree), 1);

	return TRUE;
}

int
main(void)
{
	gnt_init();

	GntWidget *widget = gnt_button_new("Button 1");
	GntWidget *widget2 = gnt_button_new("Button 2 has a longish text with a UTF-8 thing …");
	GntWidget *label = gnt_label_new("So wassup dudes and dudettes!!\nSo this is, like,\nthe third line!! \\o/");
	GntWidget *vbox, *hbox, *tree;
	WINDOW *test;

	box(stdscr, 0, 0);
	wrefresh(stdscr);

	vbox = gnt_box_new(FALSE, FALSE);
	hbox = gnt_box_new(FALSE, TRUE);

	gnt_widget_set_name(vbox, "vbox");
	gnt_widget_set_name(hbox, "hbox");
	gnt_widget_set_name(widget, "widget");
	gnt_widget_set_name(widget2, "widget2");

	gnt_box_add_widget(GNT_BOX(vbox), widget);
	gnt_box_add_widget(GNT_BOX(vbox), widget2);

	gnt_box_add_widget(GNT_BOX(hbox), label);

	gnt_box_add_widget(GNT_BOX(hbox), gnt_entry_new("a"));

	tree = gnt_tree_new();
	gnt_box_add_widget(GNT_BOX(hbox), tree);

	gnt_tree_add_row_after(GNT_TREE(tree), "a", "a", NULL, NULL);
	gnt_tree_add_row_after(GNT_TREE(tree), "c", "c", NULL, NULL);
	gnt_tree_add_row_after(GNT_TREE(tree), "d", "d", NULL, NULL);
	gnt_tree_add_row_after(GNT_TREE(tree), "e", "e", "a", NULL);
	gnt_tree_add_row_after(GNT_TREE(tree), "b", "b", "d", NULL);

	gnt_widget_set_has_border(hbox, TRUE);
	gnt_widget_set_has_shadow(hbox, TRUE);
	gnt_box_set_title(GNT_BOX(hbox), "111111111111111111111111111111111111111111111111111111111111111This is the title …");

	gnt_widget_show(hbox);

	g_signal_connect(hbox, "key_pressed", G_CALLBACK(key_pressed), tree);
	g_signal_connect(widget, "activate", G_CALLBACK(button1), hbox);
	g_signal_connect(widget2, "activate", G_CALLBACK(button2), hbox);

	gnt_main();

	return 0;
}

