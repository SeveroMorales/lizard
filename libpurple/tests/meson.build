PROGS = [
    'account_option',
    'account_manager',
    'authorization_request',
    'circular_buffer',
    'contact',
    'contact_info',
    'contact_manager',
    'conversation',
    'conversation_member',
    'credential_manager',
    'credential_provider',
    'history_adapter',
    'history_manager',
    'image',
    'keyvaluepair',
    'markup',
    'menu',
    'message',
    'notification',
    'notification_manager',
    'person',
    'presence',
    'protocol',
    'protocol_action',
    'protocol_xfer',
    'purplepath',
    'queued_output_stream',
    'str',
    'tags',
    'util',
    'whiteboard_manager',
    'xmlnode',
]

test_ui = static_library(
    'test-ui',
    'test_ui.c',
    'test_ui.h',
    c_args: [
        '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
        '-DG_LOG_USE_STRUCTURED',
        '-DG_LOG_DOMAIN="Purple-TestUI"',
    ],
    dependencies: [libpurple_dep, glib]
)

testenv.set('XDG_CONFIG_HOME', meson.current_build_dir() / 'config')

foreach prog : PROGS
    e = executable(f'test_@prog@', f'test_@prog@.c',
                   c_args : [
                       '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir())
                   ],
                   dependencies : [libpurple_dep, glib],
                   link_with: test_ui,
    )
    test(prog, e,
        env: testenv,
    )
endforeach

subdir('avatar')
subdir('sqlite3')
