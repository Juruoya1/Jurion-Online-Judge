#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int main() {
    char *query = getenv("QUERY_STRING");
    char username[256] = "";
    int logged_in = 0;
    
    if (query && strstr(query, "user=")) {
        char *p = strstr(query, "user=") + 5;
        char *end = strchr(p, '&');
        if (end) {
            int n = end - p;
            if (n > 255) n = 255;
            strncpy(username, p, n);
            username[n] = 0;
        } else {
            strcpy(username, p);
        }
        logged_in = 1;
    }
    
    printf("Content-Type: text/html\n\n");
    printf("<html><body>");
    printf("<div style=\"text-align:right; padding:10px; background:#f0f0f0;\">");
    if (logged_in) {
        printf("Welcome, <b>%s</b> | <a href=\"/logout.cgi\">Logout</a>", username);
    } else {
        printf("<a href=\"/login.cgi\">Login</a> | <a href=\"/register.cgi\">Register</a>");
    }
    printf("</div>");
    printf("<h1>Submission Status</h1>");
    printf("<table border='1' cellpadding='5'>");
    printf("<tr><th>Time</th><th>User</th><th>Problem</th><th>Result</th></tr>");
    
    FILE *f = fopen(DATA_DIR "/submissions.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            char *time_str = strtok(line, "|");
            char *user = strtok(NULL, "|");
            char *pid = strtok(NULL, "|");
            char *result = strtok(NULL, "|");
            if (time_str && user && pid && result) {
                time_t t = atoi(time_str);
                printf("<tr>");
                printf("<td>%s", ctime(&t));
                printf("<td>%s</td>", user);
                printf("<td>%s</td>", pid);
                printf("<td>%s</td>", result);
                printf("</tr>");
            }
        }
        fclose(f);
    }
    printf("</table>");
    printf("<a href='/index.cgi?user=%s'>Back</a>", username);
    printf("</body></html>");
    return 0;
}
