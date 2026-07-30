#include <CUnit/Basic.h>
#include <string.h>
#include "resume_processing.h"

void test_read_resume()
{
    /* Read a TXT resume */
    char *text = ReadResume("sample_resume.txt");

    CU_ASSERT_PTR_NOT_NULL(text);
}

void test_clean_text()
{
    char text[] = "HELLO!!! Resume### C Programming@@";

    CleanText(text);

    /* Verify that text is still available after cleaning */
    CU_ASSERT(strlen(text) > 0);
}

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("Resume Processing Suite", NULL, NULL);

    CU_add_test(suite, "Read Resume Test", test_read_resume);
    CU_add_test(suite, "Clean Text Test", test_clean_text);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();

    return 0;
}