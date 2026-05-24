#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int user_in_contest(int cid, const char *username) {
    FILE *f = fopen(DATA_DIR "/contest_participants.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *id_str = strtok(line, "|");
        char *user = strtok(NULL, "|");
        if (id_str && user && atoi(id_str) == cid && strcmp(user, username) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int get_user_id(const char *username) {
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        char *pwd = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        char *uid = strtok(NULL, "|");
        if (u && strcmp(u, username) == 0) {
            fclose(f);
            return atoi(uid);
        }
    }
    fclose(f);
    return 0;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", id_str[32] = "";
    char username[256] = "";
    int cid = 0;
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "id=");
        if (p) { p += 3; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(id_str,p,n); id_str[n]=0; } else strcpy(id_str,p); cid = atoi(id_str); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    
    if (username[0] == 0) {
        printf("<html><body><h2>Please <a href='/login.cgi'>login</a></h2></body></html>");
        return 0;
    }
    
    if (cid == 0) {
        printf("<html><body><h2>Invalid contest</h2><a href='/list_contests.cgi?token=%s'>Back</a></body></html>", token);
        return 0;
    }
    
    // 检查比赛是否存在且正在进行
    FILE *f = fopen(DATA_DIR "/contests.txt", "r");
    if (!f) { printf("<html><body><h2>Contest not found</h2></body></html>"); return 0; }
    
    char line[512];
    int found = 0;
    time_t start = 0, end = 0;
    while (fgets(line, sizeof(line), f)) {
        char *pid = strtok(line, "|");
        if (pid && atoi(pid) == cid) {
            strtok(NULL, "|"); // title
            strtok(NULL, "|"); // desc
            start = atol(strtok(NULL, "|"));
            end = atol(strtok(NULL, "|"));
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        printf("<html><body><h2>Contest not found</h2></body></html>");
        return 0;
    }
    
    time_t now = time(NULL);
    if (now < start) {
        printf("<html><body><h2>Contest not started yet</h2><a href='/contest.cgi?id=%d&token=%s'>Back</a></body></html>", cid, token);
        return 0;
    }
    if (now > end) {
        printf("<html><body><h2>Contest ended</h2><a href='/contest.cgi?id=%d&token=%s'>Back</a></body></html>", cid, token);
        return 0;
    }
    
    if (user_in_contest(cid, username)) {
        printf("<html><body><h2>You have already joined</h2><a href='/contest.cgi?id=%d&token=%s'>Back</a></body></html>", cid, token);
        return 0;
    }
    
    int uid = get_user_id(username);
    FILE *fp = fopen(DATA_DIR "/contest_participants.txt", "a");
    fprintf(fp, "%d|%s|%d|%ld|0|0\n", cid, username, uid, now);
    fclose(fp);
    
    printf("<html><body>");
    printf("<h2>You have joined contest %d!</h2>", cid);
    printf("<p>Your personal timer starts now.</p>");
    printf("<a href='/contest.cgi?id=%d&token=%s'>Go to Contest</a><br>", cid, token);
    printf("<a href='/index.cgi?token=%s'>Home</a>", token);
    printf("</body></html>");
    return 0;
}
