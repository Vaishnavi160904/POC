#include <CUnit/Basic.h>
#include "search.h"

void test_search_name()
{
    int result = SearchByName("John");

    CU_ASSERT(result >= 0);
}

void test_search_skill()
{
    int result = SearchBySkill("C");

    CU_ASSERT(result >= 0);
}

int main()
{
    CU_initialize_registry();

    CU_pSuite suite = CU_add_suite("Search Suite", NULL, NULL);

    CU_add_test(suite, "Search Name", test_search_name);
    CU_add_test(suite, "Search Skill", test_search_skill);

    CU_basic_run_tests();

    CU_cleanup_registry();
}