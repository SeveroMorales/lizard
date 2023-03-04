#include <gmodule.h>

#include <gnt.h>

static gboolean
key_pressed(GntEntry *entry, const char *text, G_GNUC_UNUSED gpointer unused)
{
	if (*text != '\r')
		return FALSE;

	{
		const char *cmd;
		void *handle;
		void (*func)(int, char**);

		cmd = gnt_entry_get_text(entry);
		handle = g_module_open(cmd, G_MODULE_BIND_LOCAL);
		if (handle && g_module_symbol(handle, "main", (gpointer)&func))
		{
			char **argv = g_new0(char*, 2);
			argv[0] = g_strdup(cmd);
			gnt_entry_clear(entry);
			func(1, argv);
			g_strfreev(argv);
		}
		else
		{
			GntWidget *widget = gnt_vbox_new(FALSE);
			gnt_box_set_toplevel(GNT_BOX(widget), TRUE);
			gnt_box_set_title(GNT_BOX(widget), "Error");
			gnt_box_add_widget(GNT_BOX(widget), gnt_label_new("Could not execute."));
			gnt_box_add_widget(GNT_BOX(widget), gnt_label_new(g_module_error()));

			gnt_widget_show(widget);
		}
	}

	return TRUE;
}

int
main(void)
{
	GntWidget *window, *entry;

	freopen(".error", "w", stderr);

	gnt_init();

	window = gnt_hbox_new(FALSE);

	gnt_box_add_widget(GNT_BOX(window), gnt_label_new("Command"));

	entry = gnt_entry_new(NULL);
	g_signal_connect(G_OBJECT(entry), "key_pressed", G_CALLBACK(key_pressed), NULL);
	gnt_box_add_widget(GNT_BOX(window), entry);

	gnt_widget_set_position(window, 0, getmaxy(stdscr) - 2);
	gnt_widget_show(window);

	gnt_main();

	gnt_quit();

	return 0;
}

