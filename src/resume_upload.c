#include "resume_upload.h"
#include "utils.h"
#include <dirent.h>

static int HasSupportedExtension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0;
    return (StrCaseCmp(dot, ".txt") == 0 || StrCaseCmp(dot, ".pdf") == 0);
}

int UploadResume(const char *filepath)
{
    if (!FileExists(filepath)) {
        printf("[resume_upload] Upload failed: '%s' not found\n", filepath);
        return 0;
    }
    if (!HasSupportedExtension(filepath)) {
        printf("[resume_upload] Unsupported file type: '%s' (use .txt or .pdf)\n", filepath);
        return 0;
    }
    if (resumeFileCount < MAX_RESUME_FILES) {
        strncpy(resumeFiles[resumeFileCount], filepath, MAX_PATH_LEN - 1);
        resumeFiles[resumeFileCount][MAX_PATH_LEN - 1] = '\0';
        resumeFileCount++;
    }
    printf("[resume_upload] Uploaded resume '%s'\n", filepath);
    return 1;
}

/* Only used internally by BulkUpload() - not part of the public module API. */
static int ScanDirectory(const char *folderPath)
{
    DIR *dir = opendir(folderPath);
    if (!dir) {
        printf("[resume_upload] ScanDirectory failed: cannot open '%s'\n", folderPath);
        return 0;
    }

    resumeFileCount = 0;
    const struct dirent *entry;
    int duplicatesIgnored = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; /* skip . .. and hidden files */
        if (!HasSupportedExtension(entry->d_name)) continue;

        char fullPath[MAX_PATH_LEN];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", folderPath, entry->d_name);

        /* skip zero-byte placeholder files (e.g. empty .pdf stubs) */
        long size = GetFileSize(fullPath);
        if (size <= 0) continue;

        int isDuplicate = 0;
        for (int i = 0; i < resumeFileCount; i++)
            if (strcmp(resumeFiles[i], fullPath) == 0) { isDuplicate = 1; break; }

        if (isDuplicate) {
            duplicatesIgnored++;
            continue;
        }

        if (resumeFileCount < MAX_RESUME_FILES) {
            strncpy(resumeFiles[resumeFileCount], fullPath, MAX_PATH_LEN - 1);
            resumeFiles[resumeFileCount][MAX_PATH_LEN - 1] = '\0';
            resumeFileCount++;
        }
    }
    closedir(dir);

    printf("[resume_upload] ScanDirectory: found %d resume(s) in '%s' (%d duplicate(s) ignored)\n",
           resumeFileCount, folderPath, duplicatesIgnored);
    return resumeFileCount;
}

int BulkUpload(const char *folderPath)
{
    int count = ScanDirectory(folderPath);
    for (int i = 0; i < count; i++)
        printf("[resume_upload] Queued for processing: %s\n", resumeFiles[i]);
    return count;
}
