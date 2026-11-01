#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../common/unarr.c"

void test_version_macros(void **state) {
#if UNARR_API_VERSION >= 120
    assert_true(true);
#else
    fail_msg("Invalid UNARR_API_VERSION");
#endif

#if UNARR_API_VERSION_CHECK(1, 2, 0) == 0x010200
assert_true(true);
#else
    fail_msg("UNARR_API_VERSION_CHECK macro failed");
#endif
}

void test_get_version(void **state) {
    assert_int_equal(ar_get_version(), UNARR_API_VERSION);
}

void test_get_version_str(void **state) {
    assert_string_equal(ar_get_version_str(), UNARR_VERSION);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_version_macros),
        cmocka_unit_test(test_get_version),
        cmocka_unit_test(test_get_version_str)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
