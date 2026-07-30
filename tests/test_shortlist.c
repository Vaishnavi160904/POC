#include <CUnit/Basic.h>
#include "shortlist.h"

void test_select_candidates()
{
    int result = SelectTopCandidates(5);

    CU_ASSERT(result >= 0);
}

void test_generate_shortlist()
{
    int result = GenerateShortlist();

    CU_ASSERT(result >= 0);
}

int main()
{
    CU_initialize_registry();

    CU_pSuite suite = CU_add_suite("Shortlist Suite", NULL, NULL);

    CU_add_test(suite, "Select Candidates", test_select_candidates);
    CU_add_test(suite, "Generate Shortlist", test_generate_shortlist);

    CU_basic_run_tests();

    CU_cleanup_registry();
}