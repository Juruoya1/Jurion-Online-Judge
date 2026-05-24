#include <stdio.h>
#include <string.h>

int main() {
    char line[] = "1|测试赛|测试比赛功能|1779478800|1779480000|1001,1000|Juruoya1|0\n";
    char *pid = strtok(line, "|");
    strtok(NULL, "|");
    strtok(NULL, "|");
    strtok(NULL, "|");
    strtok(NULL, "|");
    char *probs = strtok(NULL, "|");
    printf("probs='%s'\n", probs);
    return 0;
}
