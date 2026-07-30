#include "shortlist.h"
#include "utils.h"

/* Assumes candidates[] is already sorted (RankCandidates() ran first) */
int SelectTopCandidates(int topN)
{
    if (topN <= 0) {
        printf("[shortlist] SelectTopCandidates: invalid count (%d), shortlisting 0 candidates\n", topN);
        topN = 0;
    }

    int selected = 0;
    for (int i = 0; i < candidateCount; i++) {
        if (i < topN) {
            candidates[i].shortlisted = 1;
            selected++;
        } else {
            candidates[i].shortlisted = 0;
        }
    }
    printf("[shortlist] SelectTopCandidates: %d of %d candidate(s) shortlisted\n", selected, candidateCount);
    return selected;
}

int GenerateShortlist(void)
{
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/shortlisted");

    FILE *fp = fopen("output/shortlisted/Top10Candidates.txt", "w");
    if (!fp) {
        printf("[shortlist] GenerateShortlist warning: could not open "
               "'output/shortlisted/Top10Candidates.txt' for writing "
               "(printing results to console only)\n");
    }

    int count = 0;
    printf("\n[shortlist] Shortlisted Candidates:\n");
    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].shortlisted) {
            count++;
            printf("  %d. %s (Score: %d/100)\n", count, candidates[i].name, candidates[i].finalScore);
            if (fp) fprintf(fp, "%d. %s - Score: %d/100 - %s\n",
                             count, candidates[i].name, candidates[i].finalScore, candidates[i].filename);
        }
    }
    if (fp) {
        fclose(fp);
        printf("[shortlist] Written to output/shortlisted/Top10Candidates.txt\n");
    }
    return count;
}
