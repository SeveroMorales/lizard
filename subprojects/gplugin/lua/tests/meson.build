e = executable('test-lua-loader', 'test-lua-loader.c',
	c_args : [
		'-DLUA_LOADER_DIR="@0@/.."'.format(meson.current_build_dir()),
		'-DLUA_PLUGIN_DIR="@0@/plugins"'.format(
			meson.current_source_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, LUA, gplugin_dep])
test('Lua Loader', e)

# Add the test plugins path to our environment variable
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_source_dir() / 'plugins')
