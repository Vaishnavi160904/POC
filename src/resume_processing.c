#include "resume_processing.h"
#include "utils.h"
#include <ctype.h>

/*
 * Plain C has no PDF parser. If a .pdf is passed we tell the user how to
 * fix it (export/convert to .txt, or pipe it through a tool like pdftotext
 * and read the resulting .txt) instead of pretending to read binary PDF
 * bytes as text.
 */
/*
 * Plain C has no PDF parser of its own, so for .pdf resumes we shell out to
 * `pdftotext` (poppler-utils / Poppler for Windows) via ConvertPdfToText()
 * and process the resulting text exactly like a .txt resume.
 */
char *ReadResume(const char *filepath)
{
    const char *dot = strrchr(filepath, '.');
    if (dot && StrCaseCmp(dot, ".pdf") == 0) {
        printf("[resume_processing] '%s' is a PDF - converting with pdftotext...\n", filepath);
        return ConvertPdfToText(filepath);
    }

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        printf("[resume_processing] ReadResume failed: cannot open '%s'\n", filepath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size <= 0) {
        fclose(fp);
        printf("[resume_processing] '%s' is empty\n", filepath);
        return NULL;
    }

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    size_t readBytes = fread(buffer, 1, (size_t)size, fp);
    buffer[readBytes] = '\0';
    fclose(fp);

    printf("[resume_processing] Read %ld bytes from '%s'\n", size, filepath);
    return buffer;
}

/* Replace punctuation/symbols with spaces, keep letters, digits, and '+', '#'
 * (so C++, C#, etc. survive) and spaces */
void CleanText(char *text)
{
    if (!text) return;
    for (int i = 0; text[i]; i++) {
        unsigned char c = (unsigned char)text[i];
        if (isalnum(c) || c == '+' || c == '#' || isspace(c)) {
            /* keep as is */
        } else {
            text[i] = ' ';
        }
    }
}

void NormalizeWords(char *text)
{
    ToLowerCase(text);
}
