#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

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

int main() {
    char *query = getenv("QUERY_STRING");
    char username[256] = "";
    int problem_id = 0;
    char code[65536] = "";
    char lang[10] = "c";
    
    if (query) {
        char *p = strstr(query, "user=");
        if (p) { p += 5; char *end = strchr(p, '&'); if (end) { int n = end-p; if(n>255)n=255; strncpy(username,p,n); username[n]=0; } else strcpy(username,p); }
        p = strstr(query, "id=");
        if (p) problem_id = atoi(p + 3);
        p = strstr(query, "lang=");
        if (p) { p += 5; char *end = strchr(p,'&'); if(end){ int n=end-p; if(n>9)n=9; strncpy(lang,p,n); lang[n]=0; } else strcpy(lang,p); }
        p = strstr(query, "code=");
        if (p) { p += 5; char *end = strchr(p,'&'); if(end){ int n=end-p; if(n>65535)n=65535; strncpy(code,p,n); code[n]=0; } else strcpy(code,p); }
    }
    
    printf("Content-Type: text/html\n\n");
    printf("<html><body>");
    printf("<div style=\"text-align:right; padding:10px; background:#f0f0f0;\">");
    printf("Welcome, <b>%s</b> | <a href=\"/logout.cgi\">Logout</a>", username);
    printf("</div>");
    
    if (problem_id == 0 || code[0] == 0) {
        printf("<h2>Missing problem ID or code</h2>");
        printf("<a href='/list_problems.cgi?user=%s'>Back</a>", username);
        printf("</body></html>");
        return 0;
    }
    
    char decoded_code[65536];
    url_decode(decoded_code, code);
    
    char code_file[256];
    char exe_file[256];
    if (strcmp(lang, "c") == 0) {
        sprintf(code_file, "/tmp/code_%d_%d.c", problem_id, (int)time(NULL));
        sprintf(exe_file, "/tmp/program_%d_%d", problem_id, (int)time(NULL));
    } else if (strcmp(lang, "cpp") == 0) {
        sprintf(code_file, "/tmp/code_%d_%d.cpp", problem_id, (int)time(NULL));
        sprintf(exe_file, "/tmp/program_%d_%d", problem_id, (int)time(NULL));
    } else if (strcmp(lang, "py") == 0) {
        sprintf(code_file, "/tmp/code_%d_%d.py", problem_id, (int)time(NULL));
    } else {
        printf("<h2>Unsupported language</h2></body></html>");
        return 0;
    }
    
    FILE *fc = fopen(code_file, "w");
    if (!fc) { printf("<h2>Failed to save code</h2></body></html>"); return 0; }
    fwrite(decoded_code, 1, strlen(decoded_code), fc);
    fclose(fc);
    
    int compile_ret = 0;
    char compile_out[4096] = "";
    
    if (strcmp(lang, "c") == 0) {
        char cmd[512];
        sprintf(cmd, "gcc -o %s %s 2>&1", exe_file, code_file);
        FILE *pipe = popen(cmd, "r");
        char buf[256];
        while (fgets(buf, sizeof(buf), pipe)) strcat(compile_out, buf);
        compile_ret = pclose(pipe);
    } else if (strcmp(lang, "cpp") == 0) {
        char cmd[512];
        sprintf(cmd, "g++ -o %s %s 2>&1", exe_file, code_file);
        FILE *pipe = popen(cmd, "r");
        char buf[256];
        while (fgets(buf, sizeof(buf), pipe)) strcat(compile_out, buf);
        compile_ret = pclose(pipe);
    }
    
    if (compile_ret != 0 && strcmp(lang, "py") != 0) {
        printf("<h2>Compile Error</h2><pre>%s</pre>", compile_out);
        printf("<a href='/get_problem.cgi?id=%d&user=%s'>Back</a>", problem_id, username);
        printf("</body></html>");
        return 0;
    }
    
    // 测试第1个点
    char in_file[256], out_file[256];
    sprintf(in_file, "%s/%d/1.in", TESTDATA_DIR, problem_id);
    sprintf(out_file, "%s/%d/1.out", TESTDATA_DIR, problem_id);
    
    FILE *ftest = fopen(in_file, "r");
    if (!ftest) {
        printf("<h2>Test data not found</h2><
        return 0;
    }
    f
    
    char run_
    if (strcmp(lang
        sprintf(run
    } else {
    
    }
    system(ru
    
    char diff_cm

    int diff_ret = system(diff_cmd);
    
    // 保存记录
    FILE *fsub = fopen(DATA_DIR "/

        fprintf(fsub, "%ld|%s|%d|%s\n", time(
        fclose(fsu
    }
    
    if (diff_ret == 0) {
        print
        printf("<p>Your solution passed.</p>");
   
        printf("<h2 style='color:red;'>Wrong A
        printf("<p>Output does not match.</p>");

    
    printf("<a h
    printf("</body></html>");
    r
}
