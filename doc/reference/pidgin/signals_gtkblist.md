Title: Buddy List Signals
Slug: blist-signals

## Buddy List Signals

### gtkblist-created

```c
void user_function(PurpleBuddyList *blist, gpointer user_data);
```

Emitted when the buddy list is created.

**Parameters:**

**blist**
: The buddy list.

**user_data**
: User data set when the signal handler was connected.

### drawing-tooltip

```c
void user_function(PurpleBlistNode *node, GString *text, gboolean full, gpointer user_data);
```

Emitted just before a tooltip is displayed. `text` is a standard GString, so
the plugin can modify the text that will be displayed.

**Parameters:**

**node**
: The blist node for the tooltip.

**text**
: A pointer to the text that will be displayed.

**full**
: Whether we're doing a full tooltip for the priority buddy or a compact
tooltip for a non-priority buddy.

**user_data**
: User data set when the signal handler was connected.
