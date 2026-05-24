#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int get_user_role(const char *username) {
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return -1;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        char *pwd = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        if (u && strcmp(u, username) == 0) {
            fclose(f);
            return atoi(role);
        }
    }
    fclose(f);
    return -1;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", cid_str[32] = "";
    char username[256] = "";
    int cid = 0;
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "id=");
        if (p) { p += 3; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(cid_str,p,n); cid_str[n]=0; } else strcpy(cid_str,p); cid = atoi(cid_str); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }
    
    int role = get_user_role(username);
    int is_admin = (role == 1 || role == 2);
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    
    if (!is_admin) {
        printf("<html><body><h2>Permission denied</h2></body></html>");
        return 0;
    }
    
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<h1>Manage Contest %d</h1>", cid);
    printf("<div><a href='/admin.cgi?token=%s'>Admin</a> | <a href='/index.cgi?token=%s'>Home</a></div>", token, token);
    printf("<hr>");
    
    char title[256] = "";
    FILE *f = fopen(DATA_DIR "/contests.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            int id = atoi(strtok(line, "|"));
            if (id == cid) {
                strcpy(title, strtok(NULL, "|"));
                break;
            }
        }
        fclose(f);
    }
    printf("<h2>%s</h2>", title);
    
    printf("<h2>Participants</h2>");
    printf("<table border='1' cellpadding='5'>");
    printf("<tr><th>UID</th><th>User</th><th>Solved</th><th>Cheat</th><th>Action</th></tr>");
    
    f = fopen(DATA_DIR "/contest_participants.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            int id, uid, cheat;
            char user[256];
            sscanf(line, "%d|%255[^|]|%d|%*d|%*d|%d", &id, user, &uid, &cheat);
            if (id == cid) {
                int ac = 0;
                FILE *sf = fopen(DATA_DIR "/contest_submissions.txt", "r");
                if (sf) {
                    char sline[512];
                    while (fgets(sline, sizeof(sline), sf)) {
                        int c, u;
                        char res[8];
                        sscanf(sline, "%d|%d", &c, &u);
                        char *last = strrchr(sline, '|');
                        if (last) strcpy(res, last + 1);
                        if (c == id && u == uid && strcmp(res, "AC") == 0) ac++;
                    }
                    fclose(sf);
                }
                printf("<tr>");
                printf("<td>%d</a></td>", uid);
                printf("<td>%s</a></td>", user);
                printf("<td>%d</a></td>", ac);
                printf("<td style='color:%s'>%s</a></td>", cheat ? "red" : "green", cheat ? "Cheated" : "OK");
                printf("<td><a href='/view_user_code.cgi?cid=%d&uid=%d&token=%s'>Code</a> | ", cid, uid, token);
                printf("<a href='/toggle_cheat.cgi?cid=%d&uid=%d&token=%s'>Cheat</a></td>", cid, uid, token);
                printf("</tr>");
            }
        }
        fclose(f);
    }
    printf("</table>");
    printf("</body></html>");
    return 0;
}
