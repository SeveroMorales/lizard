Title: Core Signals
Slug: core-signals

## Core Signals

### Signal List

* [core-uri-handler](#uri-handler)

### Signal Details

#### uri-handler

```c
gboolean user_function(const gchar *proto,
                       const gchar *cmd,
                       GHashTable *params,
                       gpointer user_data);
```

Emitted when handling a registered URI.

**Parameters:**

**proto**
: The protocol of the URI.

**cmd**
: The 'command' of the URI.

**params**
: Any key/value parameters from the URI.

**user_data**
: User data set when the signal handler was connected.
