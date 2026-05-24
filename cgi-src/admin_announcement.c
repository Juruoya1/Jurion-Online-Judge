#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define ANNOUNCE_FILE "/mnt/hdd/jurionoj/data/announcement.txt"

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
    char token[256] = "", content[16384] = "";
    char username[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "content=");
        if (p) { p += 8; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>16383)n=16383; strncpy(content,p,n); content[n]=0; } else strcpy(content,p); }
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
    
    if (!is_admin) {
        printf("<html><body><h2>Permission denied</h2></body></html>");
        return 0;
    }
    
    // 读取当前公告
    FILE *af = fopen(ANNOUNCE_FILE, "r");
    char current[16384] = "";
    if (af) {
        fread(current, 1, sizeof(current)-1, af);
        fclose(af);
    }
    
    // 如果是 POST 请求，保存公告
    if (content[0] != 0) {
        char content_dec[16384];
        url_decode(content_dec, content);
        FILE *fw = fopen(ANNOUNCE_FILE, "w");
        if (fw) {
            fprintf(fw, "%s", content_dec);
            fclose(fw);
        }
        printf("<html><body>");
        printf("<h2>Announcement saved!</h2>");
        printf("<a href='/admin.cgi?token=%s'>Back to Admin</a>", token);
        printf("</body></html>");
        return 0;
    }
    
    // 显示编辑表单
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<h1>Edit Announcement</h1>");
    printf("<form method='get' action='/admin_announcement.cgi'>");
    printf("<input type='hidden' name='token' value='%s'>", token);
    printf("<textarea name='content' rows='10' cols='80' placeholder='Announcement content...'>%s</textarea><br>", current);
    printf("<input type='submit' value='Save'>");
    printf("</form>");
    printf("<a href='/admin.cgi?token=%s'>Back to Admin</a>", token);
    printf("</body></html>");
    return 0;
}
