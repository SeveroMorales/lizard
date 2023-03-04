# IRCv3

This is a brand new from-scratch protocol plugin which is the first protocol
plugin to be 100% code reviewed. It uses regex to tokenize messages.

We are intending for it to be subclass-able so other networks like Twitch.tv can
be supported but we're not quite there yet.

We also are intending to support subclassing in other languages but we've run
into some issues with dynamic GObject types and GObject introspection that have
slowed us down.

## Capability Support

This is a list of capabilities that we currently support. We'll do our best to
keep this list up to date, but if you notice we've missed something please let
us know!

* cap-notify
* sasl (right now just PLAIN works)
* message-tags
* msgid
