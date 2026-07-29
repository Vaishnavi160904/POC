#include "auth.h"
#include "utils.h"

#define USERS_FILE "data/users.dat"

/* users.dat format: one record per line -> username:password */

int Signup(const char *username, const char *password)
{
    FILE *fp = fopen(USERS_FILE, "r");
    char line[MAX_LINE];
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            char existing[100];
            sscanf(line, "%99[^:]", existing);
            if (StrCaseCmp(existing, username) == 0) {
                fclose(fp);
                printf("[auth] Signup failed: user '%s' already exists\n", username);
                return 0;
            }
        }
        fclose(fp);
    }
    fp = fopen(USERS_FILE, "a");
    if (!fp) {
        printf("[auth] Signup failed: could not open %s\n", USERS_FILE);
        return 0;
    }
    fprintf(fp, "%s:%s\n", username, password);
    fclose(fp);
    printf("[auth] Signup successful for user '%s'\n", username);
    return 1;
}

int Login(const char *username, const char *password)
{
    if (!FileExists(USERS_FILE)) {
        /* First run: seed a default HR account so the system is usable out of the box */
        Signup("hr_admin", "password123");
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("[auth] Login failed: user database unavailable\n");
        return 0;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char existing[100], pw[100];
        if (sscanf(line, "%99[^:]:%99[^\n]", existing, pw) == 2) {
            if (StrCaseCmp(existing, username) == 0 && strcmp(pw, password) == 0) {
                fclose(fp);
                strncpy(loggedInUser, username, sizeof(loggedInUser) - 1);
                isLoggedIn = 1;
                printf("[auth] Login successful. Welcome, %s!\n", username);
                return 1;
            }
        }
    }
    fclose(fp);
    printf("[auth] Login failed: invalid username or password\n");
    return 0;
}

void Logout(void)
{
    if (isLoggedIn)
        printf("[auth] User '%s' logged out\n", loggedInUser);
    else
        printf("[auth] No user was logged in\n");
    isLoggedIn = 0;
    loggedInUser[0] = '\0';
}

int ChangePassword(const char *username, const char *old_pw, const char *new_pw)
{
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("[auth] ChangePassword failed: no user database\n");
        return 0;
    }

    char lines[500][MAX_LINE];
    int total = 0;
    int found = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp) && total < 500) {
        char existing[100], pw[100];
        if (sscanf(line, "%99[^:]:%99[^\n]", existing, pw) == 2 &&
            StrCaseCmp(existing, username) == 0 && strcmp(pw, old_pw) == 0) {
            snprintf(lines[total], MAX_LINE, "%s:%s\n", username, new_pw);
            found = 1;
        } else {
            strncpy(lines[total], line, MAX_LINE - 1);
            lines[total][MAX_LINE - 1] = '\0';
        }
        total++;
    }
    fclose(fp);

    if (!found) {
        printf("[auth] ChangePassword failed: old password incorrect or user not found\n");
        return 0;
    }

    fp = fopen(USERS_FILE, "w");
    if (!fp) return 0;
    for (int i = 0; i < total; i++) fputs(lines[i], fp);
    fclose(fp);

    printf("[auth] Password changed successfully for '%s'\n", username);
    return 1;
}
