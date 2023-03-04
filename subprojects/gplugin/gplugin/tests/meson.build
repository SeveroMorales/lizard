###############################################################################
# Subdirectories
###############################################################################
subdir('bad-plugins')
subdir('bind-global')
subdir('dynamic-type')
subdir('id-collision')
subdir('auto-load-fail')
subdir('auto-load-pass')
subdir('newest-version')
subdir('plugins')
subdir('unresolved-symbol')
subdir('versioned-dependencies')

# Add the normal plugins to the plugin path envvar
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir() / 'plugins')

#######################################
# Simple Tests (single file)
#######################################
current_build_dir = meson.current_build_dir()
TESTS = {
  # name -> c_args
  'core': [
    f'-DTEST_DIR="@current_build_dir@/plugins/"',
    f'-DTEST_ID_DIR="@current_build_dir@/id-collision/"',
  ],
  'auto-load': [
    f'-DTEST_DIR="@current_build_dir@/plugins/"',
    f'-DTEST_AUTO_LOAD_PASS_DIR="@current_build_dir@/auto-load-pass"',
    f'-DTEST_AUTO_LOAD_FAIL_DIR="@current_build_dir@/auto-load-fail"',
  ],
  'bind-global': [
    f'-DTEST_BIND_GLOBAL_DIR="@current_build_dir@/bind-global/"',
    f'-DTEST_BAD_DIR="@current_build_dir@/bad-plugins/"',
    f'-DTEST_ID_DIR="@current_build_dir@/id-collision/"',
  ],
  'find-plugins': [
    f'-DTEST_DIR="@current_build_dir@/plugins/"',
  ],
  'id-collision': [
    f'-DTEST_ID_DIR="@current_build_dir@/id-collision/"',
  ],
  'id-normalize': [],
  'loader': [],
  'loader-registration': [],
  'newest-version': [
    f'-DTEST_DIR="@current_build_dir@/newest-version/"',
  ],
  'option-group': [],
  'plugin-manager-paths': [],
  'plugin-info': [],
  'signals': [
    f'-DTEST_DIR="@current_build_dir@/plugins/"',
  ],
  'source': [],
  'unresolved-symbol': [
    f'-DPLUGIN_DIR="@current_build_dir@/unresolved-symbol/"',
  ],
  'version-compare': [],
  'versioned-dependencies': [
    f'-DTEST_VERSIONED_DEPENDENCY_DIR="@current_build_dir@/versioned-dependencies"',
  ],
}

# dynamic-type-provider and dynamic-type-user libs are not built on Windows
# so no need to test them there either
if host_machine.system() != 'windows'
TESTS += {
  'dynamic-type': [
    f'-DTEST_DYNAMIC_DIR="@current_build_dir@/dynamic-type/"',
  ],
}
endif

###############################################################################
# Tests
###############################################################################

foreach prog, c_args : TESTS
	e = executable(f'test-@prog@', f'test-@prog@.c',
	               c_args: c_args,
	               dependencies: [gplugin_dep, GLIB, GOBJECT])
	if prog == 'unresolved-symbol'
		continue
	endif
	test(prog, e)
endforeach

#######################################
# Native Loader
#######################################
e = executable('test-native-loader', 'test-native-loader.c',
	c_args : [
		f'-DTEST_DIR="@current_build_dir@/plugins/"',
		f'-DTEST_BAD_DIR="@current_build_dir@/bad-plugins/"',
		f'-DTEST_ID_DIR="@current_build_dir@/id-collision/"',
	],
	link_with : gplugin_loader_tests,
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Native Loader', e)
