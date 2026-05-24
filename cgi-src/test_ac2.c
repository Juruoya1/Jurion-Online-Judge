#include <stdio.h>
#include <string.h>

int main() {
    FILE *f = fopen("/mnt/hdd/jurionoj/data/submissions.txt", "r");
    if (!f) { printf("open fail\n"); return 0; }
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        printf("Line: %s", line);
        char *user = strtok(line, "|");
        char *p1 = strtok(NULL, "|");
        char *pid = strtok(NULL, "|");
        char *res = strtok(NULL, "|");
        printf("  user=%s, pid=%s, res=%s\n", user, pid, res);
        if (user && strcmp(user, "Juruoya1") == 0 && pid && atoi(pid) == 1001) {
            printf("  FOUND! res=%s\n", res);
        }
    }
    fclose(f);
    return 0;
}
