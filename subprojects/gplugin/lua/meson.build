LUA = dependency('', required: false)
if get_option('lua')
	if not get_option('introspection')
		error('Lua plugin requires GObject Introspection.')
	endif

	GPLUGIN_LUA_SOURCES = [
		'gplugin-lua-core.c',
		'gplugin-lua-loader.c',
		'gplugin-lua-plugin.c',
	]

	GPLUGIN_LUA_HEADERS = [
		'gplugin-lua-loader.h',
		'gplugin-lua-plugin.h',
	]

	# These are ordered from most to least preferred, which would normally
	# be from the highest to lowest version.
	_LUAS = [
	         ['lua-5.4', '>=5.4.0'],
	         ['lua-5.3', '>=5.3.0'],
	         ['lua5.3',  '>=5.3.0'],
	         ['lua-5.2', '>=5.2.0'],
	         ['lua5.2',  '>=5.2.0'],
	         ['lua52',   '>=5.2.0'],
	         ['luajit',  '>=2.0.0'],
	         ['lua-5.1', '>=5.1.0'],
	         ['lua5.1',  '>=5.1.0'],
	         ['lua',     '>=5.1.0'],
        ]
	LUA_FOUND = false
	foreach _LUA : _LUAS
		LUA = dependency(_LUA[0], version : _LUA[1], required : false)
		if LUA.found()
			# Compile and run our lua-lgi test program
			lua_lgi_test = compiler.run(files('gplugin-lua-test-lgi.c'),
				dependencies : LUA,
				name : 'lua "lgi" module')
			if lua_lgi_test.compiled() and lua_lgi_test.returncode() == 0
				LUA_FOUND = true
				break
			endif
		endif
	endforeach

	if not LUA_FOUND
		error('No usable Lua library was found')
	endif
endif # lua

summary('Lua', LUA, section : 'Loader Support', bool_yn : true)

if not LUA.found()
  subdir_done()
endif

# now add the library
shared_library('gplugin-lua',
	GPLUGIN_LUA_SOURCES,
	GPLUGIN_LUA_HEADERS,
	name_prefix : '',
	c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="GPlugin-Lua"'],
	dependencies : [LUA, gplugin_dep],
	install : true,
	install_dir : get_option('libdir') / 'gplugin'
)

# Add the loader's path to our environment variable
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())

subdir('tests')
