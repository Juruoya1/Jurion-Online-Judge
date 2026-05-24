#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define PROBLEM_DATA_DIR "/mnt/hdd/jurionoj/data/problem_data"

void url_decode(char *dst, const char *src) {
    while (*src) {
        if (*src == '+') { *dst++ = ' '; src++; }
        else if (*src == '%' && src[1] && src[2]) {
            char hex[3] = {src[1], src[2], 0};
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else { *dst++ = *src++; }
    }
    *dst = '\0';
}

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

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", cid_str[32] = "", pid_str[32] = "";
    char username[256] = "";
    int cid = 0, pid = 0;
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "cid=");
        if (p) { p += 4; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(cid_str,p,n); cid_str[n]=0; } else strcpy(cid_str,p); cid = atoi(cid_str); }
        p = strstr(q, "pid=");
        if (p) { p += 4; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(pid_str,p,n); pid_str[n]=0; } else strcpy(pid_str,p); pid = atoi(pid_str); }
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
    
    if (!user_in_contest(cid, username)) {
        printf("<h2>You are not in this contest</h2><a href='/contest.cgi?id=%d&token=%s'>Back</a></body></html>", cid, token);
        return 0;
    }
    
    // 读取标题
    char title_path[512];
    sprintf(title_path, "%s/%d/title.txt", PROBLEM_DATA_DIR, pid);
    FILE *tf = fopen(title_path, "r");
    char title[512] = "";
    if (tf) {
        fgets(title, sizeof(title), tf);
        char *nl = strchr(title, '\n');
        if (nl) *nl = 0;
        fclose(tf);
    } else {
        sprintf(title, "Problem %d", pid);
    }
    
    // 读取描述
    char desc_path[512];
    sprintf(desc_path, "%s/%d/desc.txt", PROBLEM_DATA_DIR, pid);
    FILE *df = fopen(desc_path, "r");
    char desc[16384] = "";
    if (df) {
        fread(desc, 1, sizeof(desc)-1, df);
        fclose(df);
    }
    
    printf("<h1>%d. %s</h1>", pid, title);
    printf("<pre>%s</pre>", desc);
    printf("<hr>");
    
    printf("<h2>Submit Code</h2>");
    printf("<form method='get' action='/contest_submit.cgi'>");
    printf("<input type='hidden' name='id' value='%d'>", cid);
    printf("<input type='hidden' name='pid' value='%d'>", pid);
    printf("<input type='hidden' name='token' value='%s'>", token);
    printf("<p>Language: <select name='lang'>");
    printf("<option value='c'>C</option>");
    printf("<option value='cpp'>C++</option>");
    printf("<option value='py'>Python</option>");
    printf("</select></p>");
    printf("<p>Code:<br><textarea name='code' rows='25' cols='90'></textarea></p>");
    printf("<input type='submit' value='Submit'>");
    printf("</form>");
    printf("<p><a href='/contest.cgi?id=%d&token=%s'>Back to Contest</a></p>", cid, token);
    printf("</body></html>");
    return 0;
}
