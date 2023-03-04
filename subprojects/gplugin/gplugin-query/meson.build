if not get_option('install-gplugin-query')
summary('gplugin-query',
        'You have disabled installation of gplugin-query which is a very ' +
        'useful diagnostics tool.',
        section : 'Warnings')
endif

gplugin_query = executable('gplugin-query',
	'gplugin-query.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT],
	install : get_option('install-gplugin-query'))

if get_option('help2man')
	custom_target('gplugin-query.1',
		command : [help2man,
		           '--name=Query installed plugins', '--section=1',
		           '--help-option=--help-all', '--no-info',
		           '--output', '@OUTPUT@',
		           gplugin_query],
		output : 'gplugin-query.1',
		install : get_option('install-gplugin-query'),
		install_dir : get_option('mandir') / 'man1')
endif

