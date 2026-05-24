#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define PROBLEM_DATA_DIR "/mnt/hdd/jurionoj/data/problem_data"

int user_ac(int pid, const char *username) {
    FILE *f = fopen(DATA_DIR "/submissions.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *time = strtok(line, "|");
        char *user = strtok(NULL, "|");
        char *pid_str = strtok(NULL, "|");
        char *res = strtok(NULL, "|");
        if (user && pid_str && res && strcmp(user, username) == 0 && atoi(pid_str) == pid) {
            if (strstr(res, "Accepted")) {
                fclose(f);
                return 1;
            }
            int a, b;
            if (sscanf(res, "%d/%d", &a, &b) == 2 && a == b) {
                fclose(f);
                return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "";
    char keyword[256] = "";
    char username[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "q=");
        if (p) { p += 2; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(keyword,p,n); keyword[n]=0; } else strcpy(keyword,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    
    printf("<form method='get'>");
    printf("<input type='hidden' name='token' value='%s'>", token);
    printf("<input type='text' name='q' placeholder='Search' value='%s'>", keyword);
    printf("<input type='submit' value='Search'>");
    printf("</form>");
    
    printf("<table border='1' cellpadding='5' width='100%%'>");
    printf("<tr><th>ID</th><th>Title</th><th>Status</th><th>Action</th></tr>");
    
    DIR *d = opendir(PROBLEM_DATA_DIR);
    if (d) {
        struct dirent *ent;
        while ((ent = readdir(d)) != NULL) {
            if (ent->d_name[0] == '.') continue;
            int id = atoi(ent->d_name);
            if (id == 0) continue;
            
            char title_path[512];
            sprintf(title_path, "%s/%d/title.txt", PROBLEM_DATA_DIR, id);
            FILE *tf = fopen(title_path, "r");
            char title[256] = "";
            if (tf) {
                fgets(title, sizeof(title), tf);
                char *nl = strchr(title, '\n');
                if (nl) *nl = 0;
                fclose(tf);
            } else {
                sprintf(title, "Problem %d", id);
            }
            
            if (keyword[0]) {
                char id_str[16];
                sprintf(id_str, "%d", id);
                if (!strstr(id_str, keyword) && !strstr(title, keyword)) continue;
            }
            
            int ac = (username[0] && user_ac(id, username));
            
            printf("<tr>");
            printf("<td>%d</a></td>", id);
            printf("<td>%s</a></td>", title);
            if (ac) {
                printf("<td style='color:green;'>AC</a></td>");
            } else {
                printf("<td>-</a></td>");
            }
            printf("<td><a target='_blank' href='/get_problem.cgi?id=%d&token=%s'>View</a></td>", id, token);
            printf("</tr>");
        }
        closedir(d);
    }
    printf("</table>");
    return 0;
}
