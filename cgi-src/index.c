#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int main() {
    char *query = getenv("QUERY_STRING");
    char token[256] = "";
    char username[256] = "";
    int logged_in = 0;
    int is_admin = 0;

    if (query && strstr(query, "token=")) {
        char *p = strstr(query, "token=") + 6;
        char *end = strchr(p, '&');
        if (end) {
            int n = end - p;
            if (n > 255) n = 255;
            strncpy(token, p, n);
            token[n] = 0;
        } else {
            strcpy(token, p);
        }
        char session_file[512];
        sprintf(session_file, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(session_file, "r");
        if (f) {
            fgets(username, sizeof(username), f);
            fclose(f);
            char *nl = strchr(username, '\n');
            if (nl) *nl = 0;
            logged_in = 1;
        }
    }

    if (logged_in) {
        FILE *f = fopen(DATA_DIR "/users.txt", "r");
        if (f) {
            char line[512];
            while (fgets(line, sizeof(line), f)) {
                char *u = line;
                char *p = strchr(u, '|');
                if (!p) continue;
                *p = '\0';
                if (strcmp(u, username) == 0) {
                    p++;
                    p = strchr(p, '|');
                    if (p) {
                        p++;
                        int role = atoi(p);
                        if (role >= 1) is_admin = 1;
                    }
                    break;
                }
            }
            fclose(f);
        }
    }

    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<html><body>");
    printf("<div style=\"text-align:right; padding:10px; background:#f0f0f0;\">");
    if (logged_in) {
        printf("<a href=\"/index.cgi?token=%s\">Home</a> | ", token);
        printf("<a href=\"/profile.cgi?token=%s\">Profile</a> | ", token);
        printf("Welcome, <b>%s</b>", username);
        if (is_admin) {
            printf(" | <a href=\"/admin.cgi?token=%s\">Admin</a>", token);
            printf(" | <a href=\"/create_contest.cgi?token=%s\">Create Contest</a>", token);
        }
        printf(" | <a href=\"/inbox.cgi?token=%s\">Messages</a>", token);
        printf(" | <a href=\"/rank.cgi?token=%s\">Rank</a>", token);
        printf(" | <a href=\"/logout.cgi?token=%s\">Logout</a>", token);
    } else {
        printf("<a href=\"/index.cgi\">Home</a> | ");
        printf("<a href=\"/login.cgi\">Login</a> | <a href=\"/register.cgi\">Register</a>");
    }
    printf("</div>");

    //Show Texts
    printf("<h1>Jurion Online Judge-Jurion计算机在线测评系统");
    printf("<hr>");
    //公告
    printf("<iframe src='/announcement.cgi?token=%s' width='100%%' height='300' frameborder='0' scrolling='yes'></iframe>", token);
    // 比赛列表
    printf("<iframe src=\"/list_contests.cgi?token=%s\" width=\"100%%\" height=\"300\" frameborder=\"0\"></iframe>", token);
    printf("<hr>");

    //题目列表
    printf("<h2>Problems</h2>");
    printf("<iframe src=\"/list_problems.cgi?token=%s\" width=\"100%%\" height=\"600\" frameborder=\"0\"></iframe>", token);
    printf("<hr>");
    printf("<p><a href=\"/upload.cgi?token=%s\">Upload Problem</a> | <a href=\"/status.cgi?token=%s\">Status</a></p>", token, token);
    printf("</body></html>");
    return 0;
}
