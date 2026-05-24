#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define TESTDATA_DIR "/mnt/hdd/jurionoj/data/testdata"

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

int get_next_id() {
    FILE *f = fopen(DATA_DIR "/problems.txt", "r");
    int max = 1000;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        int id = atoi(strtok(line, "|"));
        if (id > max) max = id;
    }
    fclose(f);
    return max + 1;
}

int main() {
    char *query = getenv("QUERY_STRING");
    char username[256] = "";
    int logged_in = 0;
    
    if (query && strstr(query, "user=")) {
        char *p = strstr(query, "user=") + 5;
        char *end = strchr(p, '&');
        if (end) {
            int n = end - p;
            if (n > 255) n = 255;
            strncpy(username, p, n);
            username[n] = 0;
        } else {
            strcpy(username, p);
        }
        logged_in = 1;
    }
    
    printf("Content-Type: text/html\n\n");
    
    if (!logged_in) {
        printf("<html><body><h2>Please <a href='/login.cgi'>login</a> first</h2></body></html>");
        return 0;
    }
    
    // 显示表单
    if (!query || !strstr(query, "title=")) {
        printf("<html><body>");
        printf("<h2>Upload Problem</h2>");
        printf("<form method='get'>");
        printf("<input type='hidden' name='user' value='%s'>", username);
        printf("Title: <input type='text' name='title' size='50'><br><br>");
        printf("Description:<br><textarea name='description' rows='8' cols='70'></textarea><br><br>");
        printf("Test cases (format: input|output, one per line, use '|' separator):<br>");
        printf("<textarea name='test_data' rows='10' cols='70' placeholder='1 2|3%0A10 20|30%0A100 200|300'></textarea><br><br>");
        printf("<input type='submit' value='Upload'>");
        printf("</form>");
        printf("<a href='/index.cgi?user=%s'>Back</a>", username);
        printf("</body></html>");
        return 0;
    }
    
    char title[256]="", desc[4096]="", test_data[16384]="";
    char *p = strstr(query, "title="); if(p) { p+=6; char *end=strchr(p,'&'); if(end){strncpy(title,p,end-p);title[end-p]=0;}else strcpy(title,p); }
    p = strstr(query, "description="); if(p) { p+=12; char *end=strchr(p,'&'); if(end){strncpy(desc,p,end-p);desc[end-p]=0;}else strcpy(desc,p); }
    p = strstr(query, "test_data="); if(p) { p+=10; char *end=strchr(p,'&'); if(end){strncpy(test_data,p,end-p);test_data[end-p]=0;}else strcpy(test_data,p); }
    
    char title_dec[256], desc_dec[4096], test_dec[16384];
    url_decode(title_dec, title);
    url_decode(desc_dec, desc);
    url_decode(test_dec, test_data);
    
    int id = get_next_id();
    
    // 保存题目信息
    FILE *f = fopen(DATA_DIR "/problems.txt", "a");
    fprintf(f, "%d|%s|%s|%s|%ld\n", id, title_dec, desc_dec, username, time(NULL));
    fclose(f);
    
    // 创建测试数据目录
    char dir[256];
    sprintf(dir, "%s/%d", TESTDATA_DIR, id);
    mkdir(dir, 0755);
    
    // 解析测试数据
    char *line = strtok(test_dec, "\n");
    int case_num = 1;
    while (line && case_num <= 10) {
        char *sep = strchr(line, '|');
        if (sep) {
            *sep = '\0';
            char in_file[256], out_file[256];
            sprintf(in_file, "%s/%d.in", dir, case_num);
            sprintf(out_file, "%s/%d.out", dir, case_num);
            FILE *fin = fopen(in_file, "w"); fprintf(fin, "%s", line); fclose(fin);
            FILE *fout = fopen(out_file, "w"); fprintf(fout, "%s", sep + 1); fclose(fout);
            case_num++;
        }
        line = strtok(NULL, "\n");
    }
    
    printf("<html><body>");
    printf("<h2>Problem uploaded!</h2>");
    printf("<p>ID: %d, Title: %s</p>", id, title_dec);
    printf("<p>Test cases: %d</p>", case_num - 1);
    printf("<a href='/get_problem.cgi?id=%d&user=%s'>View</a><br>", id, username);
    printf("<a href='/index.cgi?user=%s'>Home</a>", username);
    printf("</body></html>");
    return 0;
}

