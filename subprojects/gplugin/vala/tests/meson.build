e = executable('test-vala-loading', 'test-vala-loading.c',
	include_directories : include_directories('.'),
	c_args : [
		'-DVALA_PLUGIN_DIR="@0@/plugins"'.format(meson.current_build_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, gplugin_dep])
test('Vala loading', e)

e = executable('test-genie-loading', 'test-genie-loading.c',
	include_directories : include_directories('.'),
	c_args : [
		'-DGENIE_PLUGIN_DIR="@0@/genie-plugins"'.format(meson.current_build_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, gplugin_dep])
test('Genie loading', e)

subdir('genie-plugins')
subdir('plugins')
