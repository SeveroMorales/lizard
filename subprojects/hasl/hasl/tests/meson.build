TESTS = [
  'context',
  'full',
  'mechanism',
  'mechanism-external',
  'mechanism-plain',
]

foreach program : TESTS
  e = executable(f'test-@program@', f'test-@program@.c',
                 dependencies : [hasl_dep])

  test(program, e)
endforeach
