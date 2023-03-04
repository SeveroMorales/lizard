TEST_PURPLE_AVATAR_SOURCES = [
	'test_purple_avatar.c'
]

TEST_PURPLE_AVATAR_RESOURCES = gnome.compile_resources(
	'test_purple_avatar_resources',
	'test_purple_avatar.gresource.xml',
	source_dir : '.',
	c_name : 'test_purple_avatar')
TEST_PURPLE_AVATAR_SOURCES += TEST_PURPLE_AVATAR_RESOURCES

test_purple_avatar = executable(
	'test_purple_avatar',
	TEST_PURPLE_AVATAR_SOURCES,
	dependencies : [libpurple_dep, glib, gdk_pixbuf])

test('avatar', test_purple_avatar)
