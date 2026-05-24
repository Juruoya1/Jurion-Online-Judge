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
    char token[256] = "", old_pwd[256] = "", new_pwd[256] = "";
    char username[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "old=");
        if (p) { p += 4; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(old_pwd,p,n); old_pwd[n]=0; } else strcpy(old_pwd,p); }
        p = strstr(q, "new=");
        if (p) { p += 4; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(new_pwd,p,n); new_pwd[n]=0; } else strcpy(new_pwd,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }
    
    printf("Content-Type: text/html\n\n");
    
    if (username[0] == 0) {
        printf("<html><body><h2>Please login</h2></body></html>");
        return 0;
    }
    
    // 显示表单
    if (old_pwd[0] == 0) {
        printf("<html><body>");
        printf("<h2>Change Password</h2>");
        printf("<form method='get' action='/change_password.cgi'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("Old Password: <input type='password' name='old'><br><br>");
        printf("New Password: <input type='password' name='new'><br><br>");
        printf("<input type='submit' value='Change'>");
        printf("</form>");
        printf("<a href='/profile.cgi?token=%s'>Back to Profile</a>", token);
        printf("</body></html>");
        return 0;
    }
    
    char old_dec[256], new_dec[256];
    url_decode(old_dec, old_pwd);
    url_decode(new_dec, new_pwd);
    
    // 验证旧密码
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
        if (u && strcmp(u, username) == 0) {
            if (strcmp(pwd, old_dec) != 0) {
                printf("<html><body><h2>Wrong old password</h2><a href='/change_password.cgi?token=%s'>Back</a></body></html>", token);
                fclose(f);
                fclose(tmp);
                return 0;
            }
            fprintf(tmp, "%s|%s|%s|%s|%s|%s\n", u, new_dec, role, uid, nick, mail);
            updated = 1;
        } else {
            fprintf(tmp, "%s", line);
        }
    }
    fclose(f);
    fclose(tmp);
    
    if (updated) {
        rename("/tmp/users.txt", DATA_DIR "/users.txt");
        printf("<html><body><h2>Password changed successfully!</h2>");
    } else {
        printf("<html><body><h2>Failed to change password</h2>");
    }
    printf("<a href='/profile.cgi?token=%s'>Back to Profile</a></body></html>", token);
    return 0;
}
