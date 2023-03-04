if not get_option('tests')
  subdir_done()
endif

TEST_WRAPPER = find_program('./test-wrapper.py', required : true)

testenv = environment()
testenv.set('XDG_CONFIG_HOME', meson.current_build_dir() / 'config')

e = executable(
	'test-html-serialization',
	'talkatutesthtmlserialization.c',
	dependencies : [talkatu_dep, GLIB, GTK4]
)
test('html', TEST_WRAPPER, args : e, is_parallel : false, env : testenv)

e = executable(
	'test-action-group',
	'talkatutestactiongroup.c',
	dependencies : [talkatu_dep, GLIB, GTK4]
)
test('action-group', TEST_WRAPPER, args : e, is_parallel : false, env : testenv)

e = executable(
	'test-html-pango-renderer',
	'talkatutesthtmlpangorenderer.c',
	dependencies : [talkatu_dep, GLIB]
)
test('html-pango-renderer', TEST_WRAPPER, args : e, is_parallel : false,
	env : testenv)

e = executable(
	'test-html-renderer',
	'talkatutesthtmlrenderer.c',
	dependencies : [talkatu_dep, GLIB]
)
test('html-renderer', TEST_WRAPPER, args : e, is_parallel : false,
	env : testenv)
