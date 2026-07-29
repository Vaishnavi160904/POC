#include "tokenizer.h"

int Tokenize(const char *text, char tokens[][MAX_TOKEN_LEN], int maxTokens)
{
    if (!text) return 0;

    char *copy = (char *)malloc(strlen(text) + 1);
    if (!copy) return 0;
    strcpy(copy, text);

    int count = 0;
    char *token = strtok(copy, " \t\r\n");
    while (token != NULL && count < maxTokens) {
        strncpy(tokens[count], token, MAX_TOKEN_LEN - 1);
        tokens[count][MAX_TOKEN_LEN - 1] = '\0';
        count++;
        token = strtok(NULL, " \t\r\n");
    }

    free(copy);
    printf("[tokenizer] Tokenize: %d tokens generated\n", count);
    return count;
}
