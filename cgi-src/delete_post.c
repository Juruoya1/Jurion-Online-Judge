#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", pid_str[32] = "";
    char username[256] = "";
    int pid = 0;
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "id=");
        if (p) { p += 3; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(pid_str,p,n); pid_str[n]=0; } else strcpy(pid_str,p); pid = atoi(pid_str); }
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
    
    // 直接用 sed 删除匹配的行
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sed -i '/^%d|%s|/d' %s/posts.txt", pid, username, DATA_DIR);
    system(cmd);
    
    // 删除内容文件
    char content_file[512];
    snprintf(content_file, sizeof(content_file), "%s/posts/%d.txt", DATA_DIR, pid);
    remove(content_file);
    
    printf("<html><body><h2>Post deleted</h2><a href='/profile.cgi?token=%s'>Back</a></body></html>", token);
    return 0;
}
