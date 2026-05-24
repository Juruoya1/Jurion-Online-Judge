#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANNOUNCE_FILE "/mnt/hdd/jurionoj/data/announcement.txt"

int main() {
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<div style='max-height:400px; overflow:auto;'>");

    FILE *f = fopen(ANNOUNCE_FILE, "r");
    if (f) {
        char content[16384];
        fread(content, 1, sizeof(content)-1, f);
        fclose(f);
        if (content[0]) {
            printf("<div style='background:#e8f4e8; border-left:4px solid #4CAF50; padding:10px; margin:10px 0;'>");
            printf("<strong>📢 Announcement:</strong><br>");
            printf("<pre style='white-space:pre-wrap; margin:5px 0 0 0;'>%s</pre>", content);
            printf("</div>");
        }
    }
    printf("</div>");
    return 0;
}
