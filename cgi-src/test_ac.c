#include <stdio.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int user_ac(int pid, const char *username) {
    FILE *f = fopen(DATA_DIR "/submissions.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *user = strtok(line, "|");
        char *p = strtok(NULL, "|");
        char *pid_str = strtok(NULL, "|");
        char *res = strtok(NULL, "|");
        if (user && pid_str && res && strcmp(user, username) == 0) {
            int p = atoi(pid_str);
            if (p == pid) {
                if (strstr(res, "Accepted")) return 1;
                int a, b;
                if (sscanf(res, "%d/%d", &a, &b) == 2 && a == b) return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int ac = user_ac(1001, "Juruoya1");
    printf("AC for 1001: %d\n", ac);
    return 0;
}
