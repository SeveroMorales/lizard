Title: Command Signals
Slug: command-signals

## Command Signals

### Signal List

* [cmd-added](#cmd-added)
* [cmd-removed](#cmd-removed)

### Signal Details

#### cmd-added

```c
void user_function(const gchar *command,
                   PurpleCmdPriority priority,
                   PurpleCmdFlag flag,
                   gpointer user_data);
```

Emitted when a new command is added.

**Parameters:**

**command**
: The new command.

**priority**
: The priority of the new command.

**flag**
: The command flags.

**user_data**
: User data set when the signal handler was connected.

----

#### cmd-removed

```c
void user_function(const gchar *command, gpointer user_data);
```

Emitted when a command is removed.

**Parameters:**

**command**
: The removed command.

**user_data**
: User data set when the signal handler was connected.
