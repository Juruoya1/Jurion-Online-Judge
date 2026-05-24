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

void escape_html(char *dst, const char *src) {
    while (*src) {
        if (*src == '<') { *dst++ = '&'; *dst++ = 'l'; *dst++ = 't'; *dst++ = ';'; src++; }
        else if (*src == '>') { *dst++ = '&'; *dst++ = 'g'; *dst++ = 't'; *dst++ = ';'; src++; }
        else if (*src == '&') { *dst++ = '&'; *dst++ = 'a'; *dst++ = 'm'; *dst++ = 'p'; *dst++ = ';'; src++; }
        else { *dst++ = *src++; }
    }
    *dst = '\0';
}

int get_next_id() {
    FILE *f = fopen(DATA_DIR "/posts.txt", "r");
    int max = 0;
    char line[512];
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            int id = atoi(strtok(line, "|"));
            if (id > max) max = id;
        }
        fclose(f);
    }
    return max + 1;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", title[512] = "", content[8192] = "";
    char username[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "title=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>511)n=511; strncpy(title,p,n); title[n]=0; } else strcpy(title,p); }
        p = strstr(q, "content=");
        if (p) { p += 8; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>8191)n=8191; strncpy(content,p,n); content[n]=0; } else strcpy(content,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    
    if (username[0] == 0) {
        printf("<html><body><h2>Please login</h2></body></html>");
        return 0;
    }
    
    if (title[0] == 0) {
        printf("<html><head><meta charset='UTF-8'></head><body>");
        printf("<h2>Create New Post</h2>");
        printf("<form method='get' action='/create_post.cgi'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("Title: <input type='text' name='title' size='60'><br><br>");
        printf("Content:<br><textarea name='content' rows='15' cols='80'></textarea><br><br>");
        printf("<input type='submit' value='Publish'>");
        printf("</form>");
        printf("<a href='/profile.cgi?token=%s'>Back</a>", token);
        printf("</body></html>");
        return 0;
    }
    
    char title_dec[512], content_dec[8192];
    url_decode(title_dec, title);
    url_decode(content_dec, content);
    
    char content_escaped[16384];
    escape_html(content_escaped, content_dec);
    
    int id = get_next_id();
    
    // 保存内容到单独文件
    char content_file[512];
    sprintf(content_file, DATA_DIR "/posts/%d.txt", id);
    FILE *fc = fopen(content_file, "w");
    if (fc) {
        fwrite(content_escaped, 1, strlen(content_escaped), fc);
        fclose(fc);
    }
    
    // 保存索引
    FILE *f = fopen(DATA_DIR "/posts.txt", "a");
    fprintf(f, "%d|%s|%s|%d.txt|%ld\n", id, username, title_dec, id, time(NULL));
    fclose(f);
    
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<h2>Post published!</h2>");
    printf("<a href='/profile.cgi?token=%s'>Back to Profile</a>", token);
    printf("</body></html>");
    return 0;
}
