e = executable('test-python3-loader', 'test-python3-loader.c',
	include_directories : include_directories('.'),
	c_args : [
		'-DPYTHON3_LOADER_DIR="@0@/.."'.format(meson.current_build_dir()),
		'-DPYTHON3_PLUGIN_DIR="@0@/plugins"'.format(meson.current_source_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, PYTHON3, PYGOBJECT, gplugin_dep])
test('Python3 loader', e)

# we can't see the symbols in gplugin-python3 externally, so use the static
# version for testing
e = executable('test-python3-utils', 'test-python3-utils.c',
	include_directories : include_directories('.'),
	link_with : [gplugin_loader_tests],
	dependencies : [GLIB, GOBJECT, PYTHON3, PYGOBJECT,
	                gplugin_python3_static_dep])
test('Python3 utils', e)

# Add the test plugins path to our environment variable
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_source_dir() / 'plugins')
