libpurple_doc_content_files = [
	'contributing.md',
	'plugin_i18n.md',
	'plugin_ids.md',
	'signals_account.md',
	'signals_blist.md',
	'signals_cmd.md',
	'signals_connection.md',
	'signals_conversation.md',
	'signals_core.md',
	'signals_jabber.md',
	'signals_savedstatus.md',
	'signals_xfer.md',
	'tut_c_plugins.md',
]

if get_option('doc')
	libpurple_toml = configure_file(
		input : 'libpurple.toml.in',
		output : 'libpurple.toml',
		configuration : version_conf,
		install : true,
		install_dir : docs_dir / 'libpurple',
	)

	libpurple_doc = custom_target('libpurple-doc',
		input : [ libpurple_toml, libpurple_gir[0] ],
		output : 'libpurple',
		command : [
			gidocgen,
			'generate',
			'--quiet',
			'--fatal-warnings',
			'--config=@INPUT0@',
			'--output-dir=@OUTPUT@',
			'--no-namespace-dir',
			'--content-dir=@0@'.format(meson.current_source_dir()),
			'--add-include-path=@0@'.format(meson.global_build_root() / 'subprojects/gplugin/gplugin'),
			'@INPUT1@'
		],
		depend_files : [ libpurple_doc_content_files ],
		build_by_default : true,
		install : true,
		install_dir : docs_dir,
	)
endif

