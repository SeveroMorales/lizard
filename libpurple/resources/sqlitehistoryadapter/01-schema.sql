CREATE TABLE message_log
(
        protocol TEXT NOT NULL, -- examples: slack, xmpp, irc, discord
        account TEXT NOT NULL, -- example: grim@reaperworld.com@milwaukee.slack.com
        conversation_id TEXT NOT NULL, -- example: #general
        message_id TEXT NOT NULL, -- exampe: 14fdjakafjakl1155
        author TEXT NULL, -- could be null for status messages
        author_name_color TEXT NULL,
        author_alias TEXT NULL,
        recipient TEXT NULL,
        content_type TEXT NULL CHECK(content_type IN ('plain', 'html', 'markdown', 'bbcode')),
        content TEXT NULL, -- must be UTF8 string
        raw_content TEXT NULL, -- the message as came from the protocol
        protocol_timestamp TEXT, -- according to protocol, could be wrong
        client_timestamp DATETIME, -- when it "landed" in libpurple
        log_version INTEGER DEFAULT 1 NOT NULL
);
