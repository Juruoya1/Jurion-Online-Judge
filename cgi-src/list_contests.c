#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

void format_time(time_t t, char *buf, int size) {
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M", tm);
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
    }
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<h1>Contests</h1>");
    printf("<table border='1' cellpadding='5'>");
    printf("<tr><th>ID</th><th>Title</th><th>Start Time</th><th>End Time</th><th>Status</th><th>Action</th></tr>");
    
    FILE *f = fopen(DATA_DIR "/contests.txt", "r");
    if (f) {
        char line[512];
        time_t now = time(NULL);
        while (fgets(line, sizeof(line), f)) {
            char *cid = strtok(line, "|");
            char *title = strtok(NULL, "|");
            char *desc = strtok(NULL, "|");
            char *st_str = strtok(NULL, "|");
            char *et_str = strtok(NULL, "|");
            char *problems = strtok(NULL, "|");
            char *creator = strtok(NULL, "|");
            if (!cid || !title) continue;
            
            time_t st = atol(st_str);
            time_t et = atol(et_str);
            char status[16] = "";
            char color[16] = "";
            if (now < st) {
                strcpy(status, "Upcoming");
                strcpy(color, "blue");
            } else if (now >= st && now <= et) {
                strcpy(status, "Running");
                strcpy(color, "green");
            } else {
                strcpy(status, "Ended");
                strcpy(color, "red");
            }
            
            char start_buf[32], end_buf[32];
            format_time(st, start_buf, sizeof(start_buf));
            format_time(et, end_buf, sizeof(end_buf));
            
            printf("<tr>");
            printf("<td>%s</a></td>", cid);
            printf("<td>%s</a></td>", title);
            printf("<td>%s</a></td>", start_buf);
            printf("<td>%s</a></td>", end_buf);
            printf("<td style='color:%s'>%s</a></td>", color, status);
            printf("<td><a href='/contest.cgi?id=%s&token=%s' target='_blank'>Enter</a></td>", cid, token);
            printf("</tr>\n");
        }
        fclose(f);
    }
    printf("</tr>");
    return 0;
}
