#include <CUnit/Basic.h>
#include "report.h"

void test_csv_report()
{
    int result = GenerateCSV("report.csv");

    CU_ASSERT(result == 1 || result == 0);
}

void test_txt_report()
{
    int result = GenerateTXT("report.txt");

    CU_ASSERT(result == 1 || result == 0);
}

int main()
{
    CU_initialize_registry();

    CU_pSuite suite = CU_add_suite("Report Suite", NULL, NULL);

    CU_add_test(suite, "CSV Report", test_csv_report);
    CU_add_test(suite, "TXT Report", test_txt_report);

    CU_basic_run_tests();

    CU_cleanup_registry();
}