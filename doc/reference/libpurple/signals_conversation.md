Title: Conversation Signals
Slug: conversation-signals

## Conversation Signals

### Signal List

* [writing-im-msg](#writing-im-msg)
* [wrote-im-msg](#wrote-im-msg)
* [sending-im-msg](#sending-im-msg)
* [sent-im-msg](#sent-im-msg)
* [receiving-im-msg](#receiving-im-msg)
* [received-im-msg](#received-im-msg)
* [blocked-im-msg](#blocked-im-msg)
* [writing-chat-msg](#writing-chat-msg)
* [wrote-chat-msg](#wrote-chat-msg)
* [sending-chat-msg](#sending-chat-msg)
* [sent-chat-msg](#sent-chat-msg)
* [receiving-chat-msg](#receiving-chat-msg)
* [received-chat-msg](#received-chat-msg)
* [conversation-created](#conversation-created)
* [conversation-updated](#conversation-updated)
* [deleting-conversation](#deleting-conversation)
* [buddy-typing](#buddy-typing)
* [buddy-typing-stopped](#buddy-typing-stopped)
* [chat-user-joining](#chat-user-joining)
* [chat-user-joined](#chat-user-joined)
* [chat-user-flags](#chat-user-flags)
* [chat-user-leaving](#chat-user-leaving)
* [chat-user-left](#chat-user-left)
* [chat-inviting-user](#chat-inviting-user)
* [chat-invited-user](#chat-invited-user)
* [chat-invited](#chat-invited)
* [chat-invite-blocked](#chat-invite-blocked)
* [chat-joined](#chat-joined)
* [chat-join-failed](#chat-join-failed)
* [chat-left](#chat-left)
* [chat-topic-changed](#chat-topic-changed)
* [cleared-message-history](#cleared-message-history)
* [conversation-extended-menu](#conversation-extended-menu)

### Signal Details

#### writing-im-msg

```c
gboolean user_function(PurpleAccount *account,
                       const gchar *who,
                       gchar **message,
                       PurpleIMConversation *im,
                       PurpleMessageFlags flags,
                       gpointer user_data);
```

Emitted before a message is written in an IM conversation. If the message is changed, then the changed message is displayed and logged instead of the original message.

> Make sure to free `*message` before you replace it.

**Parameters:**

**account**
: The account.

**who**
: The name of the user.

**message**
: A pointer to the message.

**im**
: The IM conversation.

**flags**
: Flags for this message.

**user_data**
: user data set when the signal handler was connected.

**Returns:**
`TRUE` if the message should be canceled, or `FALSE` otherwise.

----

#### wrote-im-msg

```c
void user_function(PurpleAccount *account,
                   const gchar *who,
                   gchar *message,
                   PurpleIMConversation *im,
                   PurpleMessageFlags flags,
                   gpointer user_data);
```

Emitted after a message is written and possibly displayed in a conversation.

**Parameters:**

**account**
: The account.

**who**
: The name of the user.

**message**
: The message.

**im**
: The IM conversation.

**flags**
: Flags for this message.

**user_data**
: user data set when the signal handler was connected.

----

#### sending-im-msg

```c
void user_function(PurpleAccount *account,
                   const gchar *receiver,
                   gchar **message,
                   gpointer user_data);
```

Emitted before sending an IM to a user. `message` is a pointer to the message
string, so the plugin can replace the message before being sent.

> Make sure to free `*message` before you replace it!

**Parameters:**

**account**
: The account the message is being sent on.

**receiver**
: The username of the receiver.

**message**
: A pointer to the outgoing message. This can be modified.

**user_data**
: user data set when the signal handler was connected.

----

#### sent-im-msg

```c
void user_function(PurpleAccount *account,
                   const gchar *receiver,
                   const gchar *message,
                   gpointer user_data);
```

Emitted after sending an IM to a user.

**Parameters:**

**account**
: The account the message was sent on.

**receiver**
: The username of the receiver.

**message**
: The message that was sent.

**user_data**
: user data set when the signal handler was connected.

----

#### receiving-im-msg

```c
gboolean user_function(PurpleAccount *account,
                       gchar **sender,
                       gchar **message,
                       PurpleIMConversation *im,
                       PurpleMessageFlags *flags,
                       gpointer user_data)
```

Emitted when an IM is received. The callback can replace the name of the sender, the message, or the flags by modifying the pointer to the strings and integer. This can also be used to cancel a message by returning `TRUE`.

> Make sure to free `*sender` and `*message` before you replace them!

**Parameters:**

**account**
: The account the message was received on.

**sender**
: A pointer to the username of the sender.

**message**
: A pointer to the message that was sent.

**im**
: The IM conversation.

**flags**
: A pointer to the IM message flags.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the message should be canceled, or `FALSE` otherwise.

----

#### received-im-msg

```c
void user_function(PurpleAccount *account,
                   gchar *sender,
                   gchar *message,
                   PurpleIMConversation *im,
                   PurpleMessageFlags flags,
                   gpointer user_data);
```

Emitted after an IM is received.

**Parameters:**

**account**
: The account the message was received on.

**sender**
: The username of the sender.

**message**
: The message that was sent.

**im**
: The IM conversation.

**flags**
: The IM message flags.

**user_data**
: user data set when the signal handler was connected.

----

#### blocked-im-msg

```c
void user_function(PurpleAccount *account,
                   const gchar *sender,
                   const gchar *message,
                   PurpleMessageFlags flags,
                   time_t when,
                   gpointer user_data);
```

Emitted after an IM is blocked due to privacy settings.

**Parameters:**

**account**
: The account the message was received on.

**sender**
: The username of the sender.

**message**
: The message that was blocked.

**flags**
: The IM message flags.

**when**
: The time the message was sent.

**user_data**
: user data set when the signal handler was connected.

----

#### writing-chat-msg

```c
gboolean user_function(PurpleAccount *account,
                       const gchar *who,
                       gchar **message,
                       PurpleChatConversation *chat,
                       PurpleMessageFlags flags,
                       gpointer user_data);
```

Emitted before a message is written in a chat conversation. If the message is changed, then the changed message is displayed and logged instead of the original message.

> Make sure to free `*message` before you replace it!

**Parameters:**

**account**
: The account.

**who**
: The name of the user.

**message**
: A pointer to the message.

**chat**
: The chat conversation.

**flags**
: Flags for this message.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the message should be canceled, or `FALSE` otherwise.

----

#### wrote-chat-msg

```c
void user_function(PurpleAccount *account,
                   const gchar *who,
                   gchar *message,
                   PurpleChatConversation *chat,
                   PurpleMessageFlags flags,
                   gpointer user_data);
```

Emitted after a message is written and possibly displayed in a chat.

**Parameters:**

**account**
: The account.

**who**
: The name of the user.

**message**
: The message.

**chat**
: The chat conversation.

**flags**
: Flags for this message.

**user_data**
: user data set when the signal handler was connected.

----

#### sending-chat-msg

```c
void user_function(PurpleAccount *account,
                   gchar **message,
                   gint id,
                   gpointer user_data);
```

Emitted before sending a message to a chat. `message` is a pointer to the
message string, so the plugin can replace the message before being sent.

> Make sure to free `*message` before you replace it!

**Parameters:**

**account**
: The account the message is being sent on.

**message**
: A pointer to the message that will be sent.

**id**
: The ID of the chat.

**user_data**
: user data set when the signal handler was connected.

----

#### sent-chat-msg

```c
void user_function(PurpleAccount *account,
                   const gchar *message,
                   gint id,
                   gpointer user_data)
```

Emitted after sending a message to a chat.

**Parameters:**

**account**
: The account the message was sent on.

**message**
: The message that was sent.

**id**
: The ID of the chat.

**user_data**
: user data set when the signal handler was connected.

----

#### receiving-chat-msg

```c
gboolean user_function(PurpleAccount *account,
                       gchar **sender,
                       gchar **message,
                       PurpleChatConversation *chat,
                       gint *flags,
                       gpointer user_data);
```

Emitted when a chat message is received. The callback can replace the name of the sender, the message, or the flags by modifying the pointer to the strings. This can also be used to cancel displaying a message by returning `TRUE`.

> Make sure to free `*sender` and `*message` before you replace them!

**Parameters:**

**account**
: The account the message was received on.

**sender**
: A pointer to the username of the sender.

**message**
: A pointer to the message that was sent.

**chat**
: The chat conversation.

**flags**
: A pointer to the chat message flags.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the message should be canceled, or `FALSE` otherwise.

----

#### received-chat-msg

```c
void user_function(PurpleAccount *account,
                   gchar *sender,
                   gchar *message,
                   PurpleChatConversation *chat,
                   PurpleMessageFlags flags,
                   gpointer user_data)
```

Emitted after a chat message is received.

**Parameters:**

**account**
: The account the message was received on.

**sender**
: The username of the sender.

**message**
: The message that was sent.

**chat**
: The chat conversation.

**flags**
: The chat message flags.

**user_data**
: user data set when the signal handler was connected.

----

#### conversation-created

```c
void user_function(PurpleConversation *conv, gpointer user_data);
```

Emitted when a new conversation is created.

**Parameters:**

**conv**
: The new conversation.

**user_data**
: user data set when the signal handler was connected.

----

#### conversation-updated

```c
void user_function(PurpleConversation *conv,
                   PurpleConvUpdateType type,
                   gpointer user_data);
```

Emitted when a conversation is updated.

**Parameters:**

**conv**
: The conversation that was updated.

**type**
: The type of update that was made.

**user_data**
: user data set when the signal handler was connected.

----

#### deleting-conversation

```c
void user_function(PurpleConversation *conv, gpointer user_data);
```

Emitted just before a conversation is to be destroyed.

**Parameters:**

**conv**
: The conversation that's about to be destroyed.

**user_data**
: user data set when the signal handler was connected.

----

#### buddy-typing

```c
void user_function(PurpleAccount *account,
                   const gchar *name,
                   gpointer user_data);
```

Emitted when a buddy starts typing in a conversation window.

**Parameters:**

**account**
: The account of the user which is typing.

**name**
: The name of the user which is typing.

**user_data**
: user data set when the signal handler was connected.

----

#### buddy-typing-stopped

```c
void user_function(PurpleAccount *account,
                   const gchar *name,
                   gpointer user_data);
```

Emitted when a buddy stops typing in a conversation window.

**Parameters:**

**account**
: The account of the user which stopped typing.

**name**
: The name of the user which stopped typing.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-user-joining

```c
gboolean user_function(PurpleChatConversation *chat,
                       const gchar *name,
                       PurpleChatUserFlags flags,
                       gpointer user_data);
```

Emitted when a buddy is joining a chat, before the list of users in the chat updates to include the new user.

**Parameters:**

**chat**
: The chat conversation.

**name**
: The name of the user that is joining the conversation.

**flags**
: The flags of the user that is joining the conversation.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the join should be hidden, or `FALSE` otherwise.

----

#### chat-user-joined

```c
void user_function(PurpleChatConversation *chat,
                   const gchar *name,
                   PurpleChatUserFlags flags,
                   gboolean new_arrival,
                   gpointer user_data);
```

Emitted when a buddy joined a chat, after the users list is updated.

**Parameters:**

**chat**
: The chat conversation.

**name**
: The name of the user that has joined the conversation.

**flags**
: The flags of the user that has joined the conversation.

**new_arrival**
: If the buddy is a new arrival.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-join-failed

```c
void user_function(PurpleConnection *gc,
                   GHashTable *components,
                   gpointer user_data);
```

Emitted when an account fails to join a chat room.

**Parameters:**

**gc**
: The PurpleConnection of the account which failed to join the chat.

**data**
: The components passed to purple_serv_join_chat() originally. The hash function should be g_str_hash() and the equal function should be g_str_equal().

**user_data**
: user data set when the signal handler was connected.

----

#### chat-user-flags

```c
void user_function(PurpleChatUser *chatuser,
                   PurpleChatUserFlags oldflags,
                   PurpleChatUserFlags newflags,
                   gpointer user_data);
```

Emitted when a user in a chat changes flags.

**Parameters:**

**chatuser**
: The chat user whose flags changed.

**oldflags**
: The old flags.

**newflags**
: The new flags.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-user-leaving

```c
gboolean user_function(PurpleChatConversation *chat,
                       const gchar *name,
                       const gchar *reason,
                       gpointer user_data);
```

Emitted when a user is leaving a chat, before the user list is updated. This
may include an optional reason why the user is leaving.

**Parameters:**

**chat**
: The chat conversation.

**name**
: The name of the user that is leaving the chat.

**reason**
: The optional reason why the user is leaving.

**user_data**
: user data set when the signal handler was connected.

**Returns:**

`TRUE` if the leave should be hidden, or `FALSE` otherwise.

----

#### chat-user-left


```c
void user_function(PurpleChatConversation *chat,
                   const gchar *name,
                   const gchar *reason,
                   gpointer user_data);
```

Emitted when a user leaves a chat, after the user list is updated. This may include an optional reason why the user is leaving.

**Parameters:**

**chat**
: The chat conversation.

**name**
: The name of the user that left the chat.

**reason**
: The optional reason why the user left the chat.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-inviting-user


```c
void user_function(PurpleChatConversation *chat,
                   const gchar *name,
                   gchar **invite_message,
                   gpointer user_data);
```

Emitted when a user is being invited to the chat. The callback can replace the invite message to the invitee by modifying the pointer to the invite message.

> Make sure to free `*invite_message` before you replace it!

**Parameters:**

**chat**
: The chat conversation.

**name**
: The name of the user being invited.

**invite_message**
: A pointer to the reason why a user is being invited.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-invited-user

```c
void user_function(PurpleChatConversation *conv,
                   const gchar *name,
                   const gchar *invite_message,
                   gpointer user_data);
```

Emitted when a user invited another user to a chat.

**Parameters:**

**chat**
: The chat conversation.

**name**
: The name of the user that was invited.

**invite_message**
: The message to be sent to the user when invited.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-invited

```c
gint user_function(PurpleAccount *account,
                   const gchar *inviter,
                   const gchar *chat,
                   const gchar *invite_message,
                   GHashTable *components,
                   gpointer user_data);
```

Emitted when an account was invited to a chat.

**Parameters:**

**account**
: The account being invited.

**inviter**
: The username of the person inviting the account.

**chat**
: The name of the chat you're being invited to.

**invite_message**
: The optional invite message.

**components**
: The components necessary if you want to call  purple_serv_join_chat().

**user_data**
: user data set when the signal handler was connected.

**Returns:**

Less than zero if the invitation should be rejected, greater than zero if the
invitation should be accepted. If zero is returned, the default behavior will
be maintained: the user will be prompted.

----

#### chat-invite-blocked

```c
void user_function(PurpleAccount *account,
                   const gchar *inviter,
                   const gchar *name,
                   const gchar *message,
                   GHashTable *data);
```

Emitted when an invitation to join a chat is blocked.

**Parameters:**

**account**
: The account the invitation was sent to.

**inviter**
: The name of the person sending the invitation.

**name**
: The name of the chat invited to.

**message**
: The invitation message sent.

**data**
: Hashtable containing data about the invited chat.

----

#### chat-joined

```c
void user_function(PurpleChatConversation *chat, gpointer user_data);
```

Emitted when an account joins a chat room.

**Parameters:**

**chat**
: The conversation that joined the chat room.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-left

```c
void user_function(PurpleChatConversation *chat, gpointer user_data);
```

Emitted when an account leaves a chat room.

**Parameters:**

**chat**
: The conversation that left the chat room.

**user_data**
: user data set when the signal handler was connected.

----

#### chat-topic-changed

```c
void user_function(PurpleChatConversation *chat,
                   const gchar *who,
                   const gchar *topic,
                   gpointer user_data);
```

Emitted when the topic is changed in a chat.

**Parameters:**

**chat**
: The chat conversation whose topic changed.

**who**
: The name of the person that changed the topic.

**topic**
: The new topic.

**user_data**
: user data set when the signal handler was connected.

----

#### conversation-extended-menu

```c
void user_function(PurpleConversation *conv,
                   GList **list,
                   gpointer user_data);
```

Emitted when the UI requests a list of plugin actions for a conversation.

**Parameters:**

**conv**
: The conversation.

**list**
: A pointer to the list of actions.

**user_data**
: user data set when the signal handler was connected.

----

#### cleared-message-history

```c
void user_function(PurpleConversation *conv, gpointer user_data);
```

Emitted when the conversation history is cleared.

**Parameters:**

**conv**
: The conversation.

**user_data**
: user data set when the signal handler was connected.
