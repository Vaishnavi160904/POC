/*
 * HR Resume Screening System using Top Word Frequency Analyzer
 * ---------------------------------------------------------------
 * Menu-driven console application.
 *
 *   Main Menu:   1. Login   2. Signup   3. Exit
 *
 * Per-resume processing pipeline (matches the documented process flow):
 *   HR Login -> Upload Job Requirement -> Upload Candidate Resumes ->
 *   Resume Text Extraction -> Text Cleaning -> Tokenization ->
 *   Stop Word Removal -> Top Word Frequency Analysis -> Skill
 *   Categorization -> Skill Matching -> Candidate Score Calculation ->
 *   Candidate Ranking -> Shortlisting -> Report Generation.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "auth.h"
#include "job_requirement.h"
#include "resume_upload.h"
#include "resume_processing.h"
#include "tokenizer.h"
#include "stopwords.h"
#include "analyzer.h"
#include "keyword_category.h"
#include "info_extractor.h"
#include "skill_match.h"
#include "scoring.h"
#include "ranking.h"
#include "shortlist.h"
#include "search.h"
#include "report.h"
#include "dashboard.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Small console input helpers                                        */
/* ------------------------------------------------------------------ */

static void ReadLine(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) {
        /* EOF or a read error (e.g. stdin piped/redirected and exhausted) -
         * fail safe with an empty string instead of leaving buf uninitialized. */
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* Reads a line and parses it as an integer. Returns -1 on blank/invalid
 * input instead of looping forever, so a stray Enter never hangs the menu. */
static int ReadIntChoice(void)
{
    char line[64];
    ReadLine(line, sizeof(line));
    int value;
    if (sscanf(line, "%d", &value) != 1) return -1;
    return value;
}

static void PressEnterToContinue(void)
{
    char tmp[16];
    printf("\nPress Enter to continue...");
    ReadLine(tmp, sizeof(tmp));
}

/* ------------------------------------------------------------------ */
/* Resume processing pipeline (per-resume steps, diagram-ordered)      */
/* ------------------------------------------------------------------ */

static void ProcessResume(const char *filepath)
{
    printf("\n---------------------------------------------------------\n");
    printf(" Processing: %s\n", filepath);
    printf("---------------------------------------------------------\n");

    /* --- Resume Text Extraction --- */
    char *rawText = ReadResume(filepath);
    if (!rawText) {
        printf("[main] Skipping '%s' (unreadable)\n", filepath);
        return;
    }

    if (currentCandidateIdx >= MAX_CANDIDATES) {
        printf("[main] Candidate limit reached, skipping remaining resumes\n");
        free(rawText);
        return;
    }

    Candidate *c = CURRENT;
    memset(c, 0, sizeof(Candidate));
    strncpy(c->filename, filepath, MAX_PATH_LEN - 1);

    ExtractName(rawText, c->name);
    ExtractEmail(rawText, c->email);
    ExtractPhone(rawText, c->phone);
    ExtractSkills(rawText);
    ExtractProjects(rawText);
    ExtractCertifications(rawText);
    ExtractEducation(rawText);
    ExtractExperience(rawText);

    /* --- Text Cleaning --- */
    char *workText = (char *)malloc(strlen(rawText) + 1);
    if (!workText) {
        printf("[main] Out of memory while processing '%s' - skipping\n", filepath);
        free(rawText);
        return;
    }
    strcpy(workText, rawText);
    CleanText(workText);
    NormalizeWords(workText);

    /* --- Tokenization --- */
    static char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int tokenCount = Tokenize(workText, tokens, MAX_TOKENS);
    c->totalWords = tokenCount;

    /* --- Stop Word Removal --- */
    tokenCount = RemoveStopWords(tokens, tokenCount);

    /* --- Top Word Frequency Analysis --- */
    CountWordFrequency(tokens, tokenCount);
    FindTopKeywords(10);
    KeywordDensity("python", c->totalWords);
    ResumeStatistics();

    /* --- Skill Categorization --- */
    CategorizeKeywords();
    DomainDetection();

    /* --- Skill Matching --- */
    MatchSkills();
    MissingSkills();
    CalculateMatchPercentage(c->matchedSkillCount, jobSkillCount);

    /* --- Candidate Score Calculation --- */
    CalculateSkillScore();
    CalculateExperienceScore();
    CalculateEducationScore();
    CalculateProjectScore();
    CalculateCertificationScore();
    GenerateFinalScore();

    free(workText);
    free(rawText);

    candidateCount++;
    currentCandidateIdx++;
}

static void ResetProcessingState(void)
{
    candidateCount = 0;
    currentCandidateIdx = 0;
    resumeFileCount = 0;
    jobSkillCount = 0;
}

/* ------------------------------------------------------------------ */
/* Job requirement submenu                                            */
/* ------------------------------------------------------------------ */

static void MenuRequirement(void)
{
    printf("\n--- Job Requirement ---\n");
    printf(" 1. Upload/Load Requirement File (.txt or .pdf)\n");
    printf(" 2. View Current Requirement\n");
    printf(" 3. Add a Skill\n");
    printf(" 4. Remove a Skill\n");
    printf(" 5. Delete Requirement File\n");
    printf(" 6. Reload Requirement from Disk\n");
    printf("Enter choice: ");
    int choice = ReadIntChoice();

    char path[MAX_PATH_LEN];
    char skill[MAX_WORD_LEN];

    switch (choice) {
        case 1:
            printf("\nEnter job requirement file path (.txt or .pdf)\n");
            printf("[Enter for default: data/job_requirement.pdf]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/job_requirement.pdf");
            UploadJobRequirement(path);
            break;
        case 2:
            if (jobSkillCount == 0) {
                printf("\nNo requirement loaded yet. Use option 1 first.\n");
                break;
            }
            printf("\nEnter the file path to re-read\n");
            printf("[Enter for default: data/job_requirement.txt]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/job_requirement.txt");
            ReadRequirement(path);
            break;
        case 3:
            printf("Skill to add: ");
            ReadLine(skill, sizeof(skill));
            if (strlen(skill) == 0) {
                printf("No skill entered.\n");
                break;
            }
            AddSkillToRequirement(skill);
            break;
        case 4:
            printf("Skill to remove: ");
            ReadLine(skill, sizeof(skill));
            if (strlen(skill) == 0) {
                printf("No skill entered.\n");
                break;
            }
            RemoveSkillFromRequirement(skill);
            break;
        case 5:
            printf("File path to delete [Enter for default: data/job_requirement.txt]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/job_requirement.txt");
            DeleteRequirement(path);
            break;
        case 6:
            printf("File path to reload from [Enter for default: data/job_requirement.txt]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/job_requirement.txt");
            EditRequirement(path);
            break;
        default:
            printf("\nInvalid choice.\n");
    }
}

/* ------------------------------------------------------------------ */
/* Resume upload submenu                                              */
/* ------------------------------------------------------------------ */

static void MenuUploadResumes(void)
{
    printf("\n--- Upload Resumes ---\n");
    printf(" 1. Bulk Upload (scan a folder)\n");
    printf(" 2. Upload a Single Resume\n");
    printf("Enter choice: ");
    int choice = ReadIntChoice();

    char path[MAX_PATH_LEN];
    switch (choice) {
        case 1:
            printf("\nEnter folder path containing resumes (.txt/.pdf)\n");
            printf("[Enter for default: data/resumes]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/resumes");
            BulkUpload(path);
            break;
        case 2:
            printf("\nEnter path to a single resume file (.txt or .pdf): ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) {
                printf("No path entered.\n");
                break;
            }
            UploadResume(path);
            break;
        default:
            printf("\nInvalid choice.\n");
    }
}

static void MenuProcessResumes(void)
{
    if (resumeFileCount == 0) {
        printf("\nNo resumes uploaded yet. Use 'Upload Resumes' first.\n");
        return;
    }
    if (jobSkillCount == 0) {
        printf("\nNo job requirement loaded yet. Use 'Job Requirement' first.\n");
        return;
    }

    candidateCount = 0;
    currentCandidateIdx = 0;
    for (int i = 0; i < resumeFileCount; i++)
        ProcessResume(resumeFiles[i]);

    /* --- Candidate Ranking (runs once, after every resume is processed) --- */
    if (candidateCount > 0) RankCandidates();
    printf("\n[main] Processed %d resume(s).\n", candidateCount);
}

static void MenuViewRanking(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet. Use 'Process Uploaded Resumes' first.\n");
        return;
    }
    RankCandidates();
}

/* --- Shortlisting --- */
static void MenuShortlist(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }
    printf("\nHow many candidates to shortlist? [Enter for 3]: ");
    int n = ReadIntChoice();
    if (n <= 0) n = 3;
    SelectTopCandidates(n);
    GenerateShortlist();
}

static void MenuSearch(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }

    printf("\n--- Search Candidates ---\n");
    printf(" 1. Search by Name\n");
    printf(" 2. Search by Skill\n");
    printf(" 3. Search by Minimum Experience\n");
    printf(" 4. Search by Email\n");
    printf(" 5. Filter by Degree\n");
    printf("Enter choice: ");
    int choice = ReadIntChoice();

    char buf[100];
    switch (choice) {
        case 1:
            printf("Name: ");
            ReadLine(buf, sizeof(buf));
            SearchByName(buf);
            break;
        case 2:
            printf("Skill: ");
            ReadLine(buf, sizeof(buf));
            SearchBySkill(buf);
            break;
        case 3: {
            printf("Minimum years of experience: ");
            int years = ReadIntChoice();
            if (years < 0) years = 0;
            SearchByExperience(years);
            break;
        }
        case 4:
            printf("Email: ");
            ReadLine(buf, sizeof(buf));
            SearchByEmail(buf);
            break;
        case 5:
            printf("Degree keyword (e.g. B.Tech, BCA): ");
            ReadLine(buf, sizeof(buf));
            FilterByDegree(buf);
            break;
        default:
            printf("Invalid choice.\n");
    }
}

/* --- Report Generation --- */
static void MenuReports(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }
    int csvRows = GenerateCSV("output/reports/Candidate_Report.csv");
    int txtRows = GenerateTXT("output/reports/Candidate_Report.txt");
    int summaryRows = ExportReport("output/reports/Full_Report.txt");

    if (csvRows > 0 && txtRows > 0 && summaryRows > 0)
        printf("\n[main] All reports written successfully to output/reports/\n");
    else
        printf("\n[main] One or more reports could not be written - see messages above.\n");
}

static void MenuDashboard(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }
    DisplayStatistics();
    DisplayTopSkills();
    DisplaySummary();
}

static void MenuChangePassword(void)
{
    char oldPw[100], newPw[100];
    printf("\nCurrent password: ");
    ReadLine(oldPw, sizeof(oldPw));
    printf("New password: ");
    ReadLine(newPw, sizeof(newPw));
    ChangePassword(loggedInUser, oldPw, newPw);
}

/* ------------------------------------------------------------------ */
/* HR menu loop (shown after a successful login)                      */
/* ------------------------------------------------------------------ */

static void HRMenuLoop(void)
{
    for (;;) {
        printf("\n=========================================================\n");
        printf(" HR Resume Screening System - Main Menu   (User: %s)\n", loggedInUser);
        printf("=========================================================\n");
        printf("  1. Job Requirement (upload/view/add/remove/delete)\n");
        printf("  2. Upload Resumes (bulk or single)\n");
        printf("  3. Process Uploaded Resumes\n");
        printf("  4. View Candidate Ranking\n");
        printf("  5. Shortlist Top Candidates\n");
        printf("  6. Search Candidates\n");
        printf("  7. Generate Reports\n");
        printf("  8. View Analytics Dashboard\n");
        printf("  9. Change Password\n");
        printf(" 10. Logout\n");
        printf("  0. Exit Program\n");
        printf("---------------------------------------------------------\n");
        printf("Enter choice: ");

        int choice = ReadIntChoice();
        switch (choice) {
            case 1: MenuRequirement(); break;
            case 2: MenuUploadResumes(); break;
            case 3: MenuProcessResumes(); break;
            case 4: MenuViewRanking(); break;
            case 5: MenuShortlist(); break;
            case 6: MenuSearch(); break;
            case 7: MenuReports(); break;
            case 8: MenuDashboard(); break;
            case 9: MenuChangePassword(); break;
            case 10:
                Logout();
                ResetProcessingState();
                return; /* back to the Login/Signup/Exit main menu */
            case 0:
                Logout();
                printf("\nGoodbye!\n");
                exit(0);
            default:
                printf("\nInvalid choice, please try again.\n");
                continue;
        }
        PressEnterToContinue();
    }
}

/* ------------------------------------------------------------------ */
/* Main menu: Login / Signup / Exit                                   */
/* ------------------------------------------------------------------ */

static void DoLogin(void)
{
    char username[100], password[100];
    printf("\n--- Login ---\n");
    printf("Username: ");
    ReadLine(username, sizeof(username));
    printf("Password: ");
    ReadLine(password, sizeof(password));

    if (Login(username, password)) {
        HRMenuLoop();
    } else {
        PressEnterToContinue();
    }
}

static void DoSignup(void)
{
    char username[100], password[100];
    printf("\n--- Signup ---\n");
    printf("Choose a username: ");
    ReadLine(username, sizeof(username));
    printf("Choose a password: ");
    ReadLine(password, sizeof(password));
    Signup(username, password);
    PressEnterToContinue();
}

int main(void)
{
    /* Make sure the output tree exists up front, regardless of what a
     * fresh git clone or a manually-cleaned folder looks like - report
     * generation should never silently fail just because a folder is missing. */
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/reports");
    EnsureDirectoryExists("output/shortlisted");
    EnsureDirectoryExists("output/analysis");
    EnsureDirectoryExists("output/logs");
    EnsureDirectoryExists("output/extracted_text");

    /* Reference data (skill list, stopwords, categories) is not sensitive,
     * so it's loaded once at startup regardless of login state. */
    LoadTechnicalSkills("data/skills.txt");
    LoadStopWords("data/stopwords.txt");
    LoadCategories("data/categories.txt");

    for (;;) {
        printf("\n=========================================================\n");
        printf(" HR Resume Screening System - Top Word Frequency Analyzer\n");
        printf("=========================================================\n");
        printf("  1. Login\n");
        printf("  2. Signup\n");
        printf("  3. Exit\n");
        printf("---------------------------------------------------------\n");
        printf("Enter choice: ");

        int choice = ReadIntChoice();
        switch (choice) {
            case 1: DoLogin(); break;
            case 2: DoSignup(); break;
            case 3:
                printf("\nGoodbye!\n");
                return 0;
            default:
                printf("\nInvalid choice, please try again.\n");
        }
    }
}
