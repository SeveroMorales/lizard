Title: Buddy List Signals
Slug: buddy-list-signals

## Buddy List Signals

### Signal List

* [buddy-status-changed](#buddy-status-changed)
* [buddy-idle-changed](#buddy-idle-changed)
* [buddy-signed-on](#buddy-signed-on)
* [buddy-signed-off](#buddy-signed-off)
* [update-idle](#update-idle)
* [blist-node-extended-menu](#blist-node-extended-menu)
* [buddy-icon-changed](#buddy-icon-changed)
* [blist-node-aliased](#blist-node-aliased)
* [buddy-caps-changed](#buddy-caps-changed)
* [ui-caps-changed](#ui-caps-changed)

### Signal Details

#### buddy-status-changed

```c
void user_function(PurpleBuddy *buddy,
                   PurpleStatus *old_status,
                   PurpleStatus *status,
                   gpointer user_data);
```

Emitted when a buddy on your buddy list goes away.

**Parameters:**

**buddy**
: The buddy whose status changed.

**old_status**
: The status that the buddy just changed from.

**status**
: The status that the buddy just changed to.

**user_data**
: user data set when the signal handler was connected.

----

#### buddy-idle-changed

```c
void user_function(PurpleBuddy *buddy,
                   gboolean old_idle,
                   gboolean idle,
                   gpointer user_data);
```

Emitted when a buddy on your buddy list becomes idle.

**Parameters:**

**buddy**
: The buddy whose idle status changed.

**old_idle**
: Whether the buddy was idle.

**idle**
: Whether the buddy is currently idle.

**user_data**
: user data set when the signal handler was connected.

----

#### buddy-signed-on

```c
void user_function(PurpleBuddy *buddy, gpointer user_data);
```

Emitted when a buddy on your buddy list signs on.

**Parameters:**

**buddy**
: The buddy that signed on.

**user_data**
: user data set when the signal handler was connected.

----

#### buddy-signed-off

```c
void user_function(PurpleBuddy *buddy, gpointer user_data);
```

Emitted when a buddy on your buddy list signs off.

**Parameters:**

**buddy**
: The buddy that signed off.

**user_data**
: user data set when the signal handler was connected.

----

#### update-idle

```c
void user_function(gpointer user_data);
```

Emitted when the buddy list is refreshed and the idle times are updated.

**Parameters:**

**user_data**
: user data set when the signal handler was connected.

----

#### blist-node-extended-menu

```c
void user_function(PurpleBlistNode *node, GList **menu, gpointer user_data);
```

Emitted when a buddylist menu is being constructed `menu` is a pointer to a
GList of PurpleMenuAction's allowing a plugin to add menu items.

----

#### blist-node-added

```c
void user_function(PurpleBlistNode *node, gpointer user_data);
```

Emitted when a new blist node is added to the buddy list.

----

#### blist-node-removed

```c
void user_function(PurpleBlistNode *node, gpointer user_data);
```

Emitted when a blist node is removed from the buddy list.

----

#### buddy-icon-changed

```c
void user_function(PurpleBuddy *buddy, gpointer user_data);
```

Emitted when a buddy's icon is set.

----

#### blist-node-aliased

```c
void user_function(PurpleBlistNode *node,
                   const gchar *old_alias,
                   gpointer user_data);
```

Emitted when a blist node (buddy, chat, or contact) is aliased.

----

#### buddy-caps-changed

```c
void user_function(PurpleBuddy *buddy,
                   PurpleMediaCaps newcaps,
                   PurpleMediaCaps oldcaps,
                   gpointer user_data);
```

Emitted when updating a buddy's media capabilities.

**Parameters:**

**buddy**
: The buddy.

**newcaps**
: .

**oldcaps**
: .

**user_data**
: user data set when the signal handler was connected.

----

#### ui-caps-changed

```c
void user_function(PurpleMediaCaps newcaps,
                   PurpleMediaCaps oldcaps,
                   gpointer user_data);
```

Emitted when updating the media capabilities of the UI.

**Parameters:**

**newcaps**
: .

**oldcaps**
: .

**user_data**
: user data set when the signal handler was connected.
