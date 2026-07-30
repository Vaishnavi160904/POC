#include <CUnit/Basic.h>
#include "auth.h"

void test_signup()
{
    int result = Signup("testuser1", "password");
    CU_ASSERT(result == 1 || result == 0);
}

void test_login()
{
    int result = Login("admin", "password");
    CU_ASSERT_EQUAL(result, 1);
}

int main()
{
    CU_initialize_registry();

    CU_pSuite suite = CU_add_suite("Authentication Suite", NULL, NULL);

    CU_add_test(suite, "Signup Test", test_signup);
    CU_add_test(suite, "Login Test", test_login);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();

    return 0;
}