#include <CUnit/Basic.h>
#include "resume_upload.h"

void test_upload_resume()
{
    /* Use a sample TXT resume */
    int result = UploadResume("sample_resume.txt");

    /* Pass if upload succeeds or the file already exists */
    CU_ASSERT(result == 1 || result == 0);
}

void test_bulk_upload()
{
    /* Folder containing TXT resumes */
    int result = BulkUpload("resumes");

    CU_ASSERT(result == 1 || result == 0);
}

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("Resume Upload Suite", NULL, NULL);

    CU_add_test(suite, "Upload Resume Test", test_upload_resume);
    CU_add_test(suite, "Bulk Upload Test", test_bulk_upload);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();

    return 0;
}