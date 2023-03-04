plugindir = get_option('libdir') / 'gnt3'

irssi = library('irssi', 'irssi.c',
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Gnt-IRSSI"'],
    dependencies : [ncurses, gobject, libgnt_dep],
    name_prefix : '',
    install : true, install_dir : plugindir)

s = library('s', 's.c',
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Gnt-S"'],
    dependencies : [ncurses, gobject, libgnt_dep],
    name_prefix : '',
    install : true, install_dir : plugindir)
