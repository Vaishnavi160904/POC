#include "resume_upload.h"
#include "utils.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

static int HasSupportedExtension(const char *filename)
{
    if (filename == NULL)
        return 0;

    const char *dot = strrchr(filename, '.');

    if (!dot)
        return 0;

    return (StrCaseCmp(dot, ".txt") == 0 ||
            StrCaseCmp(dot, ".pdf") == 0);
}

int UploadResume(const char *filepath)
{
    if (filepath == NULL)
    {
        printf("[resume_upload] Upload failed: NULL file path\n");
        return 0;
    }

    if (!FileExists(filepath))
    {
        printf("[resume_upload] Upload failed: '%s' not found\n",
               filepath);

        return 0;
    }

    if (!HasSupportedExtension(filepath))
    {
        printf("[resume_upload] Unsupported file type: '%s'\n",
               filepath);

        return 0;
    }

    if (resumeFileCount < MAX_RESUME_FILES)
    {
        strncpy(resumeFiles[resumeFileCount],
                filepath,
                MAX_PATH_LEN - 1);

        resumeFiles[resumeFileCount][MAX_PATH_LEN - 1] = '\0';

        resumeFileCount++;
    }

    printf("[resume_upload] Uploaded resume '%s'\n",
           filepath);

    return 1;
}

static int ScanDirectory(const char *folderPath)
{
    if (folderPath == NULL)
    {
        printf("[resume_upload] ScanDirectory failed: NULL folder\n");
        return 0;
    }

    DIR *dir = opendir(folderPath);

    if (!dir)
    {
        printf("[resume_upload] ScanDirectory failed: cannot open '%s'\n",
               folderPath);

        return 0;
    }

    resumeFileCount = 0;

    struct dirent *entry;
    int duplicatesIgnored = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (!HasSupportedExtension(entry->d_name))
            continue;

        char fullPath[MAX_PATH_LEN];

        snprintf(fullPath,
                 sizeof(fullPath),
                 "%s/%s",
                 folderPath,
                 entry->d_name);

        if (GetFileSize(fullPath) <= 0)
            continue;

        int duplicate = 0;

        for (int i = 0; i < resumeFileCount; i++)
        {
            if (strcmp(resumeFiles[i], fullPath) == 0)
            {
                duplicate = 1;
                break;
            }
        }

        if (duplicate)
        {
            duplicatesIgnored++;
            continue;
        }

        if (resumeFileCount < MAX_RESUME_FILES)
        {
            strncpy(resumeFiles[resumeFileCount],
                    fullPath,
                    MAX_PATH_LEN - 1);

            resumeFiles[resumeFileCount][MAX_PATH_LEN - 1] = '\0';

            resumeFileCount++;
        }
    }

    closedir(dir);

    printf("[resume_upload] ScanDirectory: found %d resume(s) (%d duplicates ignored)\n",
           resumeFileCount,
           duplicatesIgnored);

    return resumeFileCount;
}

int BulkUpload(const char *folderPath)
{
    if (folderPath == NULL)
    {
        printf("[resume_upload] BulkUpload failed: NULL folder\n");
        return 0;
    }

    int count = ScanDirectory(folderPath);

    for (int i = 0; i < count; i++)
    {
        printf("[resume_upload] Queued for processing: %s\n",
               resumeFiles[i]);
    }

    return count;
}