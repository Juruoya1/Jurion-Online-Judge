#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

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

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", new_nick[256] = "", new_mail[256] = "", user[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "nick=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(new_nick,p,n); new_nick[n]=0; } else strcpy(new_nick,p); }
        p = strstr(q, "mail=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(new_mail,p,n); new_mail[n]=0; } else strcpy(new_mail,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(user, sizeof(user), f); fclose(f); char *nl = strchr(user, '\n'); if(nl)*nl=0; }
    }
    
    printf("Content-Type: text/html\n\n");
    
    if (user[0] == 0) {
        printf("<html><body><h2>Please login</h2></body></html>");
        return 0;
    }
    
    char nick_dec[256], mail_dec[256];
    url_decode(nick_dec, new_nick);
    url_decode(mail_dec, new_mail);
    
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) { printf("<html><body><h2>Error</h2></body></html>"); return 0; }
    
    FILE *tmp = fopen("/tmp/users.txt", "w");
    char line[512];
    int updated = 0;
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        char *pwd = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        char *uid = strtok(NULL, "|");
        char *nick = strtok(NULL, "|");
        char *mail = strtok(NULL, "|");
        if (u && strcmp(u, user) == 0) {
            fprintf(tmp, "%s|%s|%s|%s|%s|%s\n", u, pwd, role, uid,
                    nick_dec[0] ? nick_dec : nick,
                    mail_dec[0] ? mail_dec : (mail ? mail : ""));
            updated = 1;
        } else {
            fprintf(tmp, "%s", line);
        }
    }
    fclose(f);
    fclose(tmp);
    
    rename("/tmp/users.txt", DATA_DIR "/users.txt");
    
    printf("<html><body>");
    if (updated) printf("<h2>Profile updated!</h2>");
    else printf("<h2>Failed</h2>");
    printf("<a href='/profile.cgi?token=%s'>Back</a>", token);
    printf("</body></html>");
    return 0;
}
