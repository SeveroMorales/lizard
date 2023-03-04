TEST_SQLITE3_SOURCES = [
	'test_sqlite3.c'
]

TEST_SQLITE3_RESOURCES = gnome.compile_resources('test_sqlite3_resources',
	'test_sqlite3.gresource.xml',
	source_dir : '.',
	c_name : 'test_sqlite3')
TEST_SQLITE3_SOURCES += TEST_SQLITE3_RESOURCES

test_sqlite3 = executable(
	'test_sqlite3',
	TEST_SQLITE3_SOURCES,
	dependencies : [libpurple_dep, glib, sqlite3])

test('sqlite3', test_sqlite3)
