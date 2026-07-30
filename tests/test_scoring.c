#include <CUnit/Basic.h>
#include "scoring.h"

void test_skill_score()
{
    int score = CalculateSkillScore();

    CU_ASSERT(score >= 0);
}

void test_final_score()
{
    int score = GenerateFinalScore();

    CU_ASSERT(score >= 0);
}

int main()
{
    CU_initialize_registry();

    CU_pSuite suite = CU_add_suite("Scoring Suite", NULL, NULL);

    CU_add_test(suite, "Skill Score", test_skill_score);
    CU_add_test(suite, "Final Score", test_final_score);

    CU_basic_run_tests();

    CU_cleanup_registry();
}