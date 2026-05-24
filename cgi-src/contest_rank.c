#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

typedef struct {
    int uid;
    char username[256];
    int solved;
} RankItem;

int cmp(const void *a, const void *b) {
    return ((RankItem*)b)->solved - ((RankItem*)a)->solved;
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
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<div style='text-align:right; padding:10px; background:#f0f0f0;'>");
    if (username[0]) {
        printf("<a href='/index.cgi?token=%s'>Home</a> | Welcome, <b>%s</b> | <a href='/logout.cgi?token=%s'>Logout</a>", token, username, token);
    } else {
        printf("<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>");
    }
    printf("</div>");
    
    RankItem ranks[100];
    int count = 0;
    
    FILE *f = fopen(DATA_DIR "/contest_participants.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            int id, uid;
            char user[256];
            sscanf(line, "%d|%255[^|]|%d", &id, user, &uid);
            if (id == cid) {
                strcpy(ranks[count].username, user);
                ranks[count].uid = uid;
                ranks[count].solved = 0;
                count++;
            }
        }
        fclose(f);
    }
    
    f = fopen(DATA_DIR "/contest_submissions.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            int c, u, p;
            char res[16];
            // 格式: cid|uid|pid|time|AC
            if (sscanf(line, "%d|%d|%d", &c, &u, &p) == 3) {
                // 查找最后一个 | 后面的内容
                char *last = strrchr(line, '|');
                if (last && strstr(last, "AC")) {
                    if (c == cid) {
                        for (int i = 0; i < count; i++) {
                            if (ranks[i].uid == u) {
                                ranks[i].solved++;
                                break;
                            }
                        }
                    }
                }
            }
        }
        fclose(f);
    }
    
    qsort(ranks, count, sizeof(RankItem), cmp);
    
    printf("<h1>Contest Ranking</h1>");
    printf("<table border='1' cellpadding='5'>");
    printf("<tr><th>Rank</th><th>User</th><th>Solved</th></tr>");
    for (int i = 0; i < count; i++) {
        printf("<tr>");
        printf("<td>%d</a></td>", i + 1);
        printf("<td><a href='/profile.cgi?token=%s&user=%s'>%s</a></td>", token, ranks[i].username, ranks[i].username);
        printf("<td>%d</a></td>", ranks[i].solved);
        printf("</tr>");
    }
    printf("</table>");
    printf("<a href='/contest.cgi?id=%d&token=%s'>Back</a>", cid, token);
    printf("</body></html>");
    return 0;
}
