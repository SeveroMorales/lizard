Title: Conversation Signals
Slug: conversation-signals

## Conversation signals

### displaying-im-msg

```c
gboolean
user_function(PurpleAccount *account,
              const gchar *who,
              gchar **message,
              PurpleConversation *conv,
              PurpleMessageFlags flags,
              gpointer user_data);
```

Emitted just before a message is displayed in an IM conversation. `message` is
a pointer to a string, so the plugin can replace the message that will be
displayed. This can also be used to cancel displaying a message by returning
`TRUE`.

> **NOTE:** Make sure to free `*message` before you replace it!

**Parameters:**

**account**
: The account.

**who**
: The name of the user.

**message**
: A pointer to the message.

**conv**
: The conversation.

**flags**
: Flags for this message.

**user_data**
: User data set when the signal handler was connected.

**Returns:**

`TRUE` if the message should be canceled, or `FALSE` otherwise.

## displayed-im-msg

```c
void
user_function(PurpleAccount *account,
              const gchar *who,
              gchar *message,
              PurpleConversation *conv,
              PurpleMessageFlags flags,
              gpointer user_data);
```

Emitted after a message is displayed in an IM conversation.

**Parameters:**

**account**
: The account.

**who**
: The name of the user.

**message**
: The message.

**conv**
: The conversation.

**flags**
: Flags for this message.

**user_data**
: User data set when the signal handler was connected.

### displaying-chat-msg

```c
gboolean
user_function(PurpleAccount *account,
              const gchar *who,
              gchar **message,
              PurpleConversation *conv,
              PurpleMessageFlags flags,
              gpointer user_data);
```

Emitted just before a message is displayed in a chat. `message` is a pointer to
a string, so the plugin can replace the message that will be displayed. This
can also be used to cancel displaying a message by returning `TRUE`.

> **NOTE:** Make sure to free `*message` before you replace it!

**Parameters:**

**account**
: The account the message is being displayed and sent on.

**who**
: The name of the user.

**message**
: A pointer to the message that will be displayed and sent.

**conv**
: The conversation the message is being displayed and sent on.

**flags**
: Flags for this message.

**user_data**
: User data set when the signal handler was connected.

**Returns:**

`TRUE` if the message should be canceled, or `FALSE` otherwise.

### displayed-chat-msg

```c
void
user_function(PurpleAccount *account,
              const gchar *who,
              gchar *message,
              PurpleConversation *conv,
              PurpleMessageFlags flags,
              gpointer user_data);
```

Emitted after a message is displayed in a chat conversation.

**Parameters:**

**account**
: The account the message is being displayed and sent on.

**who**
: The name of the user.

**message**
: A pointer to the message that will be displayed and sent.

**conv**
: The conversation the message is being displayed and sent on.

**flags**
: Flags for this message.

**user_data**
: User data set when the signal handler was connected.

### conversation-switched

```c
void user_function(PurpleConversation *conv, gpointer user_data);
```

Emitted when a window switched from one conversation to another.

**Parameters:**

**new_conv**
: The now active conversation.

**user_data**
: User data set when the signal handler was connected.

### conversation-displayed

```c
void user_function(PidginConversation *gtkconv, gpointer user_data);
```

Emitted right after the Pidgin UI is attached to a new conversation.

**Parameters:**

**gtkconv**
: The PidginConversation.

**user_data**
: User data set when the signal handler was connected.
