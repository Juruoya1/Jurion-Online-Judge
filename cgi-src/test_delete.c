#include <stdio.h>
#include <string.h>

int main() {
    char line[] = "1|Juruoya1|test|1.txt|123456\n";
    char *id_str = strtok(line, "|");
    char *author = strtok(NULL, "|");
    printf("id=%s, author=%s\n", id_str, author);
    return 0;
}
