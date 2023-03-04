TESTS = [
	'parser',
]

foreach prog : TESTS
	e = executable(
		f'test_ircv3_@prog@', f'test_ircv3_@prog@.c',
		dependencies : [libpurple_dep, glib, hasl],
		objects : ircv3_prpl.extract_all_objects())

	test(f'ircv3_@prog@', e)
endforeach
