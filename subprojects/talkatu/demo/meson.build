###############################################################################
# Variables
###############################################################################
sources = [
	'talkatudemo.c',
	'talkatudemowindow.c',
	'talkatudemowindow.h',
]

###############################################################################
# Resources
###############################################################################
talkatudemo_resources = gnome.compile_resources(
	'talkatudemoresources',
	'data/talkatudemo.gresource.xml',
	c_name: 'talkatudemo',
	source_dir: 'data'
)

###############################################################################
# Demo program
###############################################################################
talkatudemo = executable('talkatu-demo',
	sources,
	talkatudemo_resources,
	c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Talkatu-Demo"'],
	dependencies: [GLIB, GTK4, talkatu_dep],
	include_directories : top_srcdir,
	install: get_option('install-demo'),
)

if get_option('help2man')
	custom_target('talkatu-demo.1',
		command : [HELP2MAN,
		           '--name=Talkatu Demo', '--section=1',
		           '--help-option=--help-all', '--no-info',
		           '--output', '@OUTPUT@',
		           talkatudemo],
		output : 'talkatu-demo.1',
		install : get_option('install-demo'),
		install_dir : get_option('mandir') / 'man1')
endif
