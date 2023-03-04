Title: Jabber Signals
Slug: jabber-signals

## Jabber Signals

### Signal List

* [jabber-receiving-iq](#jabber-receiving-iq)
* [jabber-receiving-message](#jabber-receiving-message)
* [jabber-receiving-presence](#jabber-receiving-presence)
* [jabber-watched-iq](#jabber-watched-iq)
* [jabber-register-namespace-watcher](#jabber-register-namespace-watcher)
* [jabber-unregister-namespace-watcher](#jabber-unregister-namespace-watcher)
* [jabber-sending-xmlnode](#jabber-sending-xmlnode)
* [jabber-receiving-xmlnode](#jabber-receiving-xmlnode)

### Signal Details

#### jabber-receiving-iq

```c
gboolean user_function(PurpleConnection *gc,
                       const gchar *type,
                       const gchar *id,
                       const gchar *from,
                       PurpleXmlNode *iq,
                       gpointer user_data);
```

Emitted when an XMPP IQ stanza is received. Allows a plugin to process IQ
stanzas.

**Parameters:**

**gc**
: The connection on which the stanza is received.

**type**
: The IQ type ('get', 'set', 'result', or 'error').

**id**
: The ID attribute from the stanza. MUST NOT be NULL.

**from**
: The originator of the stanza. MAY BE NULL if the stanza originated from the
user's server.

**iq**
: The full stanza received.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the plugin processed this stanza and *nobody else* should process it.
`FALSE` otherwise.

----

#### jabber-receiving-message

```c
gboolean user_function(PurpleConnection *gc,
                       const gchar *type,
                       const gchar *id,
                       const gchar *from,
                       const gchar *to,
                       PurpleXmlNode *message,
                       gpointer user_data);
```

Emitted when an XMPP message stanza is received. Allows a plugin to process
message stanzas.

**Parameters:**

**gc**
: The connection on which the stanza is received.

**type**
: The message type (see rfc3921 or rfc3921bis).

**id**
: The ID attribute from the stanza. MAY BE NULL.

**from**
: The originator of the stanza. MAY BE NULL if the stanza originated from the
user's server.

**to**
: The destination of the stanza. This is probably either the full JID of the
receiver or the receiver's bare JID.

**message**
: The full stanza received.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the plugin processed this stanza and *nobody else* should process it.
`FALSE` otherwise.

----

#### jabber-receiving-presence

```c
gboolean user_function(PurpleConnection *gc,
                       const gchar *type,
                       const gchar *from,
                       PurpleXmlNode *presence,
                       gpointer user_data)
```

Emitted when an XMPP presence stanza is received. Allows a plugin to process
presence stanzas.

**Parameters:**

**gc**
: The connection on which the stanza is received.

**type**
: The presence type (see rfc3921 or rfc3921bis). NULL indicates this is an
"available" (i.e. online) presence.

**from**
: The originator of the stanza. MAY BE NULL if the stanza originated from the
user's server.

**presence**
: The full stanza received.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the plugin processed this stanza and *nobody else* should process it.
`FALSE` otherwise.

----

#### jabber-watched-iq

```c
gboolean user_function(PurpleConnection *gc,
                       const gchar *type,
                       const gchar *id,
                       const gchar *from,
                       PurpleXmlNode *child,
                       gpointer user_data);
```

Emitted when an IQ with a watched (child, namespace) pair is received.  See jabber-register-namespace-watcher and jabber-unregister-namespace-watcher.

**Parameters:**

**gc**
: The connection on which the stanza is received.

**type**
: The IQ type ('get', 'set', 'result', or 'error').

**id**
: The ID attribute from the stanza. MUST NOT be NULL.

**from**
: The originator of the stanza. MAY BE NULL if the stanza originated from the user's server.

**child**
: The child node with namespace.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the plugin processed this stanza and *nobody else* should process it.
`FALSE` otherwise.

----

#### jabber-register-namespace-watcher

```c
void user_function(const gchar *node,
                   const gchar *namespace,
                   gpointer user_data)
```

Emit this signal to register your desire to have specific IQ stanzas to be
emitted via the jabber-watched-iq signal when received.

**Parameters:**

**node**
: The IQ child name to longer watch.

**namespace**
: The IQ child namespace to longer watch.

**user_data**
: user data set when the signal handler was connected.

----

#### jabber-unregister-namespace-watcher

```c
void user_function(const gchar *node,
                   const gchar *namespace,
                   gpointer user_data);
```

Emit this signal to unregister your desire to have specific IQ stanzas to be
emitted via the jabber-watched-iq signal when received.

**Parameters:**

**node**
: The IQ child name to no longer watch.

**namespace**
: The IQ child namespace to no longer watch.

**user_data**
: user data set when the signal handler was connected.

----

#### jabber-sending-xmlnode

```c
void user_function(PurpleConnection *gc,
                   PurpleXmlNode **stanza,
                   gpointer user_data);
```

Emit this signal (`purple_signal_emit`) to send a stanza. It is preferred to use this instead of purple_protocol_server_iface_send_raw.

**Parameters:**

**gc**
: The connection on which to send the stanza.

**stanza**
: The stanza to send. If stanza is not NULL after being sent, the emitter should free it.

**user_data**
: user data set when the signal handler was connected.

----

#### jabber-receiving-xmlnode

```c
void user_function(PurpleConnection *gc,
                   PurpleXmlNode **stanza,
                   gpointer user_data);
```

Emitted when an XMPP stanza is received. Allows a plugin to process any stanza.

**Parameters:**

**gc**
: The connection on which the stanza was received.

**stanza**
: The received stanza. Set stanza to NULL (and free it) to stop processing the stanza.

**user_data**
: user data set when the signal handler was connected.
