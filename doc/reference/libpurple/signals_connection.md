Title: Connection Signals
Slug: connection-signals

## Connection Signals

### Signal List

* [online](#online)
* [offline](#offline)
* [signing-on](#signing-on)
* [signed-on](#signed-on)
* [autojoin](#autojoin)
* [signing-off](#signing-off)
* [signed-off](#signed-off)
* [connection-error](#connection-error)

### Signal Details

#### online

```c
void user_function(gpointer user_data);
```

Emitted when the first connection has connected when all connections were
previously not connected.

**Parameters:**

**user_data**
: user data set when the signal handler was connected.

----

#### offline

```c
void user_function(gpointer user_data);
```

Emitted when the last connected connection has disconnected.


**Parameters:**

**user_data**
: user data set when the signal handler was connected.

----

#### signing-on

```c
void user_function(PurpleConnection *gc, gpointer user_data);
```

Emitted when a connection is about to sign on.

**Parameters:**

**gc**
: The connection that is about to sign on.

**user_data**
: user data set when the signal handler was connected.

----

#### signed-on

```c
void user_function(PurpleConnection *gc, gpointer user_data);
```

Emitted when a connection has signed on.

**Parameters:**

**gc**
: The connection that has signed on.

**user_data**
: user data set when the signal handler was connected.

----

#### autojoin

```c
gboolean user_function(PurpleConnection *gc, gpointer user_data);
```

Emitted when a connection has signed on, after the signed-on signal, to signal
UIs to autojoin chats if they wish. UIs should connect to this with
`PURPLE_SIGNAL_PRIORITY_HIGHEST` to allow plugins to block this signal before
the UI sees it and then re-emit it later.

**Parameters:**

**gc**
: The connection that has signed on.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the signal was handled or `FALSE` otherwise.  In practice, the return
value is irrelevant, as it really only exists so plugins can block the UI's
autojoin.

----

#### signing-off

```c
void user_function(PurpleConnection *gc, gpointer user_data);
```

Emitted when a connection is about to sign off.

**Parameters:**

**gc**
: The connection that is about to sign off.

**user_data**
: user data set when the signal handler was connected.

----

#### signed-off

```c
void user_function(PurpleConnection *gc, gpointer user_data);
```

Emitted when a connection has signed off.

**Parameters:**

**gc**
: The connection that has signed off.

**user_data**
: user data set when the signal handler was connected.

----

#### connection-error

```c
void user_function(PurpleConnection *gc,
                   PurpleConnectionError err,
                   const gchar *desc,
                   gpointer user_data);
```

Emitted when a connection error occurs, before `"signed"`-off.

**Parameters:**

**gc**
: The connection on which the error has occurred.

**err**
: The error that occurred.

**desc**
: A description of the error, giving more information.

**user_data**
: user data set when the signal handler was connected.
