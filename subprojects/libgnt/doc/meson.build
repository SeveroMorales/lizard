if not get_option('doc')
	subdir_done()
endif

if not get_option('introspection')
	error('Documentation requires GObject Introspection.')
endif

libgnt_doc_content_files = [
	'migrating-2to3.md',
]

gidocgen_dep = dependency(
	'gi-docgen', version: '>= 2021.1',
	fallback: ['gi-docgen', 'dummy_dep'],
	required: get_option('doc')
)

gidocgen = find_program('gi-docgen', required : get_option('doc'))
docs_dir = get_option('prefix') / get_option('datadir') / 'doc'

libgnt_toml = configure_file(
	input : 'libgnt.toml.in',
	output : 'libgnt.toml',
	configuration : gnt_config,
	install : true,
	install_dir : docs_dir / 'libgnt',
)

libgnt_doc = custom_target('libgnt-doc',
	input : [ libgnt_toml, libgnt_gir[0] ],
	output : 'libgnt',
	command : [
		gidocgen,
		'generate',
		'--quiet',
		'--fatal-warnings',
		'--config=@INPUT0@',
		'--output-dir=@OUTPUT@',
		'--no-namespace-dir',
		'--content-dir=@0@'.format(meson.current_source_dir()),
		'@INPUT1@'
	],
	depend_files : [ libgnt_doc_content_files ],
	build_by_default : true,
	install : true,
	install_dir : docs_dir,
)

alias_target('doc', [libgnt_doc])
