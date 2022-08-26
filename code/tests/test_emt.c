// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): Instead of providing a header, could just extern functions
#include "emt.h"

#define ARRAY_LEN(arr) \
  ((sizeof(arr)) / (sizeof(arr[0])))

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>
#include <cmocka.h>

static void
test_TheThird(void **cmocka_state)
{
  // TODO(Ryan): What episilon value should be chosen?
  assert_float_equal(0.1f, 0.2f, 0.001f);
}

static void
test_Element0ShouldReturn1(void **cmocka_state)
{
  assert_int_equal(get_fib(0), 1);
}

static void
test_Element2ShouldReturn2(void **cmocka_state)
{
  assert_int_equal(get_fib(2), 2);
}

static void
test_ElementNShouldReturnPartOfSequence(void **cmocka_state)
{
  int expected_output[] = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};
  for (int element = 0; element < ARRAY_LEN(expected_output); ++element)
  {
    assert_int_equal(get_fib(element), expected_output[element]);
  }
}

static void
test_NegativeElementsShouldReturn0(void **cmocka_state)
{
  assert_int_equal(get_fib(-1), 0);
  assert_int_equal(get_fib(-333), 0);
  assert_int_equal(get_fib(INT_MIN), 0);
}


static void
test_ElementThatOverflowsShouldReturn0(void **cmocka_state)
{
  assert_int_equal(sizeof(int), 4);

  assert_true(get_fib(FIB_OVERFLOW_NUM - 1) != 0);
  assert_int_equal(get_fib(FIB_OVERFLOW_NUM), 0);
  assert_int_equal(get_fib(FIB_OVERFLOW_NUM + 5), 0);
  assert_int_equal(get_fib(INT_MAX), 0);
}

int
cmocka_group_setup(void **state)
{
  TestState *test_state = calloc(1, sizeof(TestState));
  *state = test_state;
  return 0;
}

int
cmocka_test_setup(void **state)
{
  TestState *test_state = *state;
  test_state->val = 0;
  return 0;
}


int 
main(void)
{
	const struct CMUnitTest tests[] = {
    // cmocka_unit_test_setup_teardown(test, setup, teardown),
    cmocka_unit_test(test_Element0ShouldReturn1),
    cmocka_unit_test(test_Element2ShouldReturn2),
    cmocka_unit_test(test_ElementNShouldReturnPartOfSequence),
    cmocka_unit_test(test_NegativeElementsShouldReturn0),
    cmocka_unit_test(test_ElementThatOverflowsShouldReturn0),
  };

  return cmocka_run_group_tests(tests, group_setup, group_teardown);
}
