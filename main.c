/*
 * HR Resume Screening System using Top Word Frequency Analyzer
 * ---------------------------------------------------------------
 * Menu-driven console application.
 *
 *   Main Menu:      1. Login   2. Signup   3. Exit
 *   After Login:    Upload requirement, bulk upload resumes, process them,
 *                    view ranking, shortlist, search, reports, dashboard,
 *                    change password, logout, exit.
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
/* Resume processing pipeline (unchanged logic, now menu-triggered)    */
/* ------------------------------------------------------------------ */

static void ProcessResume(const char *filepath)
{
    printf("\n---------------------------------------------------------\n");
    printf(" Processing: %s\n", filepath);
    printf("---------------------------------------------------------\n");

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

    char *workText = (char *)malloc(strlen(rawText) + 1);
    strcpy(workText, rawText);
    CleanText(workText);
    NormalizeWords(workText);

    static char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int tokenCount = Tokenize(workText, tokens, MAX_TOKENS);
    c->totalWords = tokenCount;
    tokenCount = RemoveStopWords(tokens, tokenCount);

    CountWordFrequency(tokens, tokenCount);
    FindTopKeywords(10);
    KeywordDensity("python", c->totalWords);
    CategorizeKeywords();
    DomainDetection();
    ResumeStatistics();

    MatchSkills();
    MissingSkills();
    CalculateMatchPercentage(c->matchedSkillCount, jobSkillCount);

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
/* HR feature menu actions                                            */
/* ------------------------------------------------------------------ */

static void MenuUploadRequirement(void)
{
    char path[MAX_PATH_LEN];
    printf("\nEnter job requirement file path (.txt or .pdf)\n");
    printf("[Enter for default: data/job_requirement.pdf]: ");
    ReadLine(path, sizeof(path));
    if (strlen(path) == 0) strcpy(path, "data/job_requirement.pdf");

    UploadJobRequirement(path);
}

static void MenuBulkUploadResumes(void)
{
    char path[MAX_PATH_LEN];
    printf("\nEnter folder path containing resumes (.txt/.pdf)\n");
    printf("[Enter for default: data/resumes]: ");
    ReadLine(path, sizeof(path));
    if (strlen(path) == 0) strcpy(path, "data/resumes");

    BulkUpload(path);
}

static void MenuProcessResumes(void)
{
    if (resumeFileCount == 0) {
        printf("\nNo resumes uploaded yet. Use 'Bulk Upload Resumes' first.\n");
        return;
    }
    if (jobSkillCount == 0) {
        printf("\nNo job requirement loaded yet. Use 'Upload Job Requirement' first.\n");
        return;
    }

    candidateCount = 0;
    currentCandidateIdx = 0;
    for (int i = 0; i < resumeFileCount; i++)
        ProcessResume(resumeFiles[i]);

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
        default:
            printf("Invalid choice.\n");
    }
}

static void MenuReports(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }
    GenerateCSV("output/reports/Candidate_Report.csv");
    GenerateTXT("output/reports/Candidate_Report.txt");
    ExportReport("output/reports/Full_Report.txt");
    printf("\n[main] Reports written to output/reports/\n");
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
    int running = 1;
    while (running) {
        printf("\n=========================================================\n");
        printf(" HR Resume Screening System - Main Menu   (User: %s)\n", loggedInUser);
        printf("=========================================================\n");
        printf("  1. Upload Job Requirement\n");
        printf("  2. Bulk Upload Resumes\n");
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
            case 1: MenuUploadRequirement(); break;
            case 2: MenuBulkUploadResumes(); break;
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
                running = 0;
                return;
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
    /* Reference data (skill list, stopwords, categories) is not sensitive,
     * so it's loaded once at startup regardless of login state. */
    LoadTechnicalSkills("data/skills.txt");
    LoadStopWords("data/stopwords.txt");
    LoadCategories("data/categories.txt");

    int running = 1;
    while (running) {
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
                running = 0;
                break;
            default:
                printf("\nInvalid choice, please try again.\n");
        }
    }
    return 0;
}
