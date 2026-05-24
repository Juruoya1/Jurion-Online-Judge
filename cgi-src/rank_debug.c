#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int main() {
    printf("Content-Type: text/plain\n\n");
    
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) { printf("No users file\n"); return 0; }
    
    printf("Users:\n");
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        printf("  %s\n", u);
    }
    fclose(f);
    
    f = fopen(DATA_DIR "/submissions.txt", "r");
    if (!f) { printf("No submissions file\n"); return 0; }
    
    printf("\nSubmissions (Accepted only):\n");
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        char *res = strtok(NULL, "|");
        res = strtok(NULL, "|");
        res = strtok(NULL, "|");
        if (u && res && strcmp(res, "Accepted") == 0) {
            printf("  %s -> %s\n", u, res);
        }
    }
    fclose(f);
    
    return 0;
}
