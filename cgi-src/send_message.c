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

int user_exists(const char *username) {
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        if (u && strcmp(u, username) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int get_next_msg_id() {
    FILE *f = fopen(DATA_DIR "/messages.txt", "r");
    int max = 0;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        int id = atoi(strtok(line, "|"));
        if (id > max) max = id;
    }
    fclose(f);
    return max + 1;
}

int main() {
    char *query = getenv("QUERY_STRING");
    char token[256] = "";
    char to[256] = "";
    char content[4096] = "";
    char username[256] = "";
    
    if (query) {
        char *p = strstr(query, "token=");
        if (p) { p += 6; char *end = strchr(p, '&'); if (end) { int n = end-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(query, "to=");
        if (p) { p += 3; char *end = strchr(p, '&'); if (end) { int n = end-p; if(n>255)n=255; strncpy(to,p,n); to[n]=0; } else strcpy(to,p); }
        p = strstr(query, "content=");
        if (p) { p += 8; char *end = strchr(p, '&'); if (end) { int n = end-p; if(n>4095)n=4095; strncpy(content,p,n); content[n]=0; } else strcpy(content,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) {
            fgets(username, sizeof(username), f);
            fclose(f);
            char *nl = strchr(username, '\n');
            if (nl) *nl = 0;
        }
    }
    
    printf("Content-Type: text/html\n\n");
    
    if (username[0] == 0) {
        printf("<html><body><h2>Please <a href='/login.cgi'>login</a> first</h2></body></html>");
        return 0;
    }
    
    // 显示表单
    if (to[0] == 0) {
        printf("<html><body>");
        printf("<h2>Send Message</h2>");
        printf("<form method='get'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("To: <input type='text' name='to'><br><br>");
        printf("Content:<br><textarea name='content' rows='10' cols='60'></textarea><br><br>");
        printf("<input type='submit' value='Send'>");
        printf("</form>");
        printf("<a href='/index.cgi?token=%s'>Back</a>", token);
        printf("</body></html>");
        return 0;
    }
    
    char to_dec[256], content_dec[4096];
    url_decode(to_dec, to);
    url_decode(content_dec, content);
    
    if (!user_exists(to_dec)) {
        printf("<html><body><h2>User '%s' does not exist</h2><a href='/send_message.cgi?token=%s'>Back</a></body></html>", to_dec, token);
        return 0;
    }
    
    int msg_id = get_next_msg_id();
    FILE *f = fopen(DATA_DIR "/messages.txt", "a");
    fprintf(f, "%d|%s|%s|%s|%ld|0\n", msg_id, username, to_dec, content_dec, time(NULL));
    fclose(f);
    
    printf("<html><body>");
    printf("<h2>Message sent to %s!</h2>", to_dec);
    printf("<a href='/inbox.cgi?token=%s'>Inbox</a> | <a href='/index.cgi?token=%s'>Home</a>", token, token);
    printf("</body></html>");
    return 0;
}
