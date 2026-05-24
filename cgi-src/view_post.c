#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    
    int role = get_user_role(username);
    int is_admin = (role == 1 || role == 2);
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<div style='text-align:right; padding:10px; background:#f0f0f0;'>");
    if (username[0]) {
        printf("<a href='/index.cgi?token=%s'>Home</a> | Welcome, <b>%s</b> | <a href='/logout.cgi?token=%s'>Logout</a>", token, username, token);
    } else {
        printf("<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>");
    }
    printf("</div>");
    
    FILE *f = fopen(DATA_DIR "/posts.txt", "r");
    if (!f) { printf("<h2>Post not found</h2></body></html>"); return 0; }
    
    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char *id_str = strtok(line, "|");
        char *author = strtok(NULL, "|");
        char *ptitle = strtok(NULL, "|");
        char *pfile = strtok(NULL, "|");
        char *ptime = strtok(NULL, "|");
        if (id_str && atoi(id_str) == pid) {
            time_t t = atol(ptime);
            printf("<h1>%s</h1>", ptitle);
            printf("<p><strong>Author:</strong> %s</p>", author);
            printf("<p><strong>Time:</strong> %s</p>", ctime(&t));
            printf("<hr>");
            // 读取内容文件
            char content_file[512];
            sprintf(content_file, DATA_DIR "/posts/%s", pfile);
            FILE *fc = fopen(content_file, "r");
            if (fc) {
                char content[32768];
                int n = fread(content, 1, sizeof(content)-1, fc);
                content[n] = '\0';
                fclose(fc);
                printf("<pre>%s</pre>", content);
            } else {
                printf("<p>Content not found</p>");
            }
            printf("<hr>");
            if (strcmp(author, username) == 0 || is_admin) {
                printf("<p><a href='/delete_post.cgi?token=%s&id=%d' onclick='return confirm(\"Delete this post?\")'>Delete Post</a></p>", token, pid);
            }
            printf("<p><a href='/profile.cgi?token=%s'>Back</a></p>", token);
            found = 1;
            break;
        }
    }
    fclose(f);
    if (!found) printf("<h2>Post not found</h2>");
    printf("</body></html>");
    return 0;
}

