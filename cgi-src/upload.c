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
    char *q = getenv("QUERY_STRING");
    char token[256] = "", username[256] = "";
    char title[512] = "", desc[4096] = "", test_data[32768] = "";

    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "title=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>511)n=511; strncpy(title,p,n); title[n]=0; } else strcpy(title,p); }
        p = strstr(q, "desc=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>4095)n=4095; strncpy(desc,p,n); desc[n]=0; } else strcpy(desc,p); }
        p = strstr(q, "test_data=");
        if (p) { p += 10; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>32767)n=32767; strncpy(test_data,p,n); test_data[n]=0; } else strcpy(test_data,p); }
    }

    if (token[0]) {
        char sf[512];
        sprintf(sf, "%s/sessions/%s", DATA_DIR, token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }

    printf("Content-Type: text/html; charset=utf-8\n\n");

    if (username[0] == 0) {
        printf("<html><body><h2>Please <a href='/login.cgi'>login</a></h2></body></html>");
        return 0;
    }

    if (title[0] == 0) {
        printf("<html><body>");
        printf("<h2>Upload Problem</h2>");
        printf("<form method='get'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("Title: <input type='text' name='title' size='50'><br><br>");
        printf("Description:<br><textarea name='desc' rows='8' cols='70'></textarea><br><br>");
        printf("Test cases (input||output, one per line):<br>");
        printf("<textarea name='test_data' rows='10' cols='70'></textarea><br><br>");
        printf("<input type='submit' value='Upload'>");
        printf("</form>");
        printf("</body></html>");
        return 0;
    }

    char title_dec[512], desc_dec[4096], test_dec[32768];
    url_decode(title_dec, title);
    url_decode(desc_dec, desc);
    url_decode(test_dec, test_data);

    // 获取下一个ID
    char cmd[512];
    sprintf(cmd, "tail -1 %s/problems.txt 2>/dev/null | cut -d'|' -f1", DATA_DIR);
    FILE *fp = popen(cmd, "r");
    int last_id = 1000;
    fscanf(fp, "%d", &last_id);
    pclose(fp);
    int id = last_id + 1;

    // 写入文件（用 >> 追加，自动换行）
    char file_path[512];
    sprintf(file_path, "%s/problems.txt", DATA_DIR);
    FILE *f = fopen(file_path, "a");
    if (f) {
        fprintf(f, "%d|%s|%s|%ld\n", id, title_dec, username, time(NULL));
        fclose(f);
    }

    // 创建目录和文件
    sprintf(cmd, "mkdir -p %s/problem_data/%d", DATA_DIR, id);
    system(cmd);
    sprintf(cmd, "mkdir -p %s/testdata/%d", DATA_DIR, id);
    system(cmd);
    sprintf(cmd, "echo '%s' > %s/problem_data/%d/title.txt", title_dec, DATA_DIR, id);
    system(cmd);
    sprintf(cmd, "echo '%s' > %s/problem_data/%d/desc.txt", desc_dec, DATA_DIR, id);
    system(cmd);

    int case_num = 0;
    char *line = strtok(test_dec, "\n");
    while (line && case_num < 50) {
        char *sep = strstr(line, "||");
        if (sep) {
            *sep = '\0';
            case_num++;
            sprintf(cmd, "echo '%s' > %s/testdata/%d/%d.in", line, DATA_DIR, id, case_num);
            system(cmd);
            sprintf(cmd, "echo '%s' > %s/testdata/%d/%d.out", sep+2, DATA_DIR, id, case_num);
            system(cmd);
        }
        line = strtok(NULL, "\n");
    }

    printf("<html><body>");
    printf("<h2>Upload Success!</h2>");
    printf("<p>ID: %d</p>", id);
    printf("<a href='/index.cgi?token=%s'>Home</a>", token);
    printf("</body></html>");
    return 0;
}
