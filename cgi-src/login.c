#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    char *query = getenv("QUERY_STRING");
    
    if (!query || !strstr(query, "username=") || !strstr(query, "password=")) {
        printf("Content-Type: text/html\n\n");
        printf("<html><body><h2>Login</h2>");
        printf("<form method='get'>");
        printf("Username: <input type='text' name='username'><br>");
        printf("Password: <input type='password' name='password'><br>");
        printf("<input type='submit' value='Login'></form>");
        printf("</body></html>");
        return 0;
    }
    
    char username[256]="", password[256]="";
    char *p = strstr(query, "username="); if(p) { p+=9; char *end=strchr(p,'&'); if(end){strncpy(username,p,end-p);username[end-p]=0;}else strcpy(username,p); }
    p = strstr(query, "password="); if(p) { p+=9; char *end=strchr(p,'&'); if(end){strncpy(password,p,end-p);password[end-p]=0;}else strcpy(password,p); }
    
    char user_dec[256], pass_dec[256];
    url_decode(user_dec, username);
    url_decode(pass_dec, password);
    
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) { printf("<html><body><h2>Login failed</h2></body></html>"); return 0; }
    
    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char *user = strtok(line, "|");
        char *pass = strtok(NULL, "|");
        if (user && pass && strcmp(user, user_dec) == 0 && strcmp(pass, pass_dec) == 0) {
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        printf("<html><body><h2>Login failed</h2><a href='/login.cgi'>Try again</a></body></html>");
        return 0;
    }
    
    // 生成 session ID
    srand(time(NULL));
    char session_id[64];
    sprintf(session_id, "%d_%d_%d", (int)time(NULL), rand(), getpid());
    
    // 保存 session
    char session_file[256];
    sprintf(session_file, DATA_DIR "/sessions/%s", session_id);
    FILE *sf = fopen(session_file, "w");
    fprintf(sf, "%s", user_dec);
    fclose(sf);
    
    printf("Content-Type: text/html\n\n");
    printf("<html><head>");
    printf("<meta http-equiv='refresh' content='0;url=/index.cgi?session=%s'>", session_id);
    printf("</head><body><p>Login success! Redirecting...</p></body></html>");
    return 0;
}
