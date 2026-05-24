#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int main() {
    char *query = getenv("QUERY_STRING");
    char token[256] = "";
    char username[256] = "";
    
    if (query) {
        char *p = strstr(query, "token=");
        if (p) { p += 6; char *end = strchr(p, '&'); if (end) { int n = end-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) {
            fgets(username, sizeof(username), f);
            fclose(f);
            char *nl = strchr(username, '\n');
            if (nl) *nl = 0;
        }
    }
    
    printf("Content-Type: text/html\n\n");
    printf("<html><body>");
    printf("<h1>Sent Messages - %s</h1>", username);
    printf("<div><a href='/index.cgi?token=%s'>Home</a> | <a href='/send_message.cgi?token=%s'>Send</a> | <a href='/inbox.cgi?token=%s'>Inbox</a></div>", token, token, token);
    printf("<hr>");
    printf("<table border='1' cellpadding='5'>");
    printf("<tr><th>To</th><th>Message</th><th>Time</th></tr>");
    
    FILE *f = fopen(DATA_DIR "/messages.txt", "r");
    if (f) {
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            int id = atoi(strtok(line, "|"));
            char *from = strtok(NULL, "|");
            char *to = strtok(NULL, "|");
            char *content = strtok(NULL, "|");
            char *time_str = strtok(NULL, "|");
            if (from && strcmp(from, username) == 0) {
                printf("<tr>");
                printf("<td>%s</a></td>", to);
                printf("<td>%s</a></td>", content);
                printf("<td>%s</a></td>", time_str);
                printf("</tr>");
            }
        }
        fclose(f);
    }
    printf("</table>");
    printf("</body></html>");
    return 0;
}
