#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define TESTDATA_DIR "/mnt/hdd/jurionoj/data/testdata"

void url_decode(char *dst, const char *src) {
    while (*src) {
        if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else if (*src == '%' && src[1] && src[2]) {
            char hex[3] = {src[1], src[2], 0};
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

int user_in_contest(int cid, const char *username) {
    FILE *f = fopen(DATA_DIR "/contest_participants.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *id_str = strtok(line, "|");
        char *user = strtok(NULL, "|");
        if (id_str && user && atoi(id_str) == cid && strcmp(user, username) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int get_user_id(const char *username) {
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        strtok(NULL, "|");
        strtok(NULL, "|");
        char *uid = strtok(NULL, "|");
        if (u && uid && strcmp(u, username) == 0) {
            fclose(f);
            return atoi(uid);
        }
    }
    fclose(f);
    return 0;
}

int count_testcases(int pid) {
    int count = 0;
    for (int i = 1; i <= 50; i++) {
        char path[512];
        sprintf(path, "%s/%d/%d.in", TESTDATA_DIR, pid, i);
        if (access(path, F_OK) == 0) {
            count++;
        } else {
            break;
        }
    }
    return count > 0 ? count : 1;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", cid_str[32] = "", pid_str[32] = "", lang[10] = "c", code[65536] = "";
    char username[256] = "";
    int cid = 0, pid = 0;
    long long timestamp = time(NULL);

    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "id=");
        if (p) { p += 3; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(cid_str,p,n); cid_str[n]=0; } else strcpy(cid_str,p); cid = atoi(cid_str); }
        p = strstr(q, "pid=");
        if (p) { p += 4; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(pid_str,p,n); pid_str[n]=0; } else strcpy(pid_str,p); pid = atoi(pid_str); }
        p = strstr(q, "lang=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>9)n=9; strncpy(lang,p,n); lang[n]=0; } else strcpy(lang,p); }
        p = strstr(q, "code=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>65535)n=65535; strncpy(code,p,n); code[n]=0; } else strcpy(code,p); }
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

    printf("Content-Type: text/html; charset=utf-8\n\n");

    if (username[0] == 0) {
        printf("<html><body><h2>Please <a href='/login.cgi'>login</a></h2></body></html>");
        return 0;
    }

    if (!user_in_contest(cid, username)) {
        printf("<html><body><h2>You are not in this contest</h2><a href='/contest.cgi?id=%d&token=%s'>Back</a></body></html>", cid, token);
        return 0;
    }

    if (pid == 0 || code[0] == 0) {
        printf("<html><body><h2>Missing problem ID or code</h2></body></html>");
        return 0;
    }

    char decoded_code[65536];
    url_decode(decoded_code, code);

    char code_file[256];
    if (strcmp(lang, "c") == 0) {
        sprintf(code_file, "/tmp/contest_%d_%d_%ld.c", cid, pid, timestamp);
    } else if (strcmp(lang, "cpp") == 0) {
        sprintf(code_file, "/tmp/contest_%d_%d_%ld.cpp", cid, pid, timestamp);
    } else {
        sprintf(code_file, "/tmp/contest_%d_%d_%ld.py", cid, pid, timestamp);
    }

    FILE *fc = fopen(code_file, "w");
    if (!fc) {
        printf("<html><body><h2>Failed to save code</h2></body></html>");
        return 0;
    }
    fwrite(decoded_code, 1, strlen(decoded_code), fc);
    fclose(fc);

    char program_file[256];
    sprintf(program_file, "/tmp/program_%d_%d_%ld", cid, pid, timestamp);

    // 编译
    int compile_ret = 0;
    char compile_out[4096] = "";
    if (strcmp(lang, "c") == 0 || strcmp(lang, "cpp") == 0) {
        char cmd[512];
        if (strcmp(lang, "cpp") == 0) {
            sprintf(cmd, "g++ -o %s %s 2>&1", program_file, code_file);
        } else {
            sprintf(cmd, "gcc -o %s %s 2>&1", program_file, code_file);
        }
        FILE *pipe = popen(cmd, "r");
        char buf[256];
        while (fgets(buf, sizeof(buf), pipe)) strcat(compile_out, buf);
        compile_ret = pclose(pipe);
    }

    if (compile_ret != 0 && strcmp(lang, "py") != 0) {
        printf("<html><body><h2 style='color:red;'>Compile Error</h2><pre>%s</pre><a href='/contest.cgi?id=%d&token=%s'>Back</a></body></html>", compile_out, cid, token);
        unlink(code_file);
        return 0;
    }

    // 统计测试点
    int total = count_testcases(pid);
    int pass = 0;

    // 逐个判题
    for (int i = 1; i <= total; i++) {
        char in_file[256], out_file[256];
        sprintf(in_file, "%s/%d/%d.in", TESTDATA_DIR, pid, i);
        sprintf(out_file, "%s/%d/%d.out", TESTDATA_DIR, pid, i);

        char output_file[256];
        sprintf(output_file, "/tmp/output_%d_%d_%d_%d.txt", cid, pid, i, (int)timestamp);

        char run_cmd[512];
        if (strcmp(lang, "py") == 0) {
            sprintf(run_cmd, "timeout 5 python %s < %s > %s 2>&1", code_file, in_file, output_file);
        } else {
            sprintf(run_cmd, "timeout 5 %s < %s > %s 2>&1", program_file, in_file, output_file);
        }
        system(run_cmd);

        char diff_cmd[512];
        sprintf(diff_cmd, "diff -w %s %s > /dev/null 2>&1", output_file, out_file);
        if (system(diff_cmd) == 0) {
            pass++;
        }
        
        unlink(output_file);
    }

    int uid = get_user_id(username);
    FILE *subf = fopen(DATA_DIR "/contest_submissions.txt", "a");
    if (subf) {
        fprintf(subf, "%d|%d|%d|%ld|%s\n", cid, uid, pid, time(NULL), (pass == total) ? "AC" : "WA");
        fclose(subf);
    }

    // 清理
    unlink(code_file);
    if (strcmp(lang, "c") != 0 && strcmp(lang, "py") != 0) {
        unlink(program_file);
    }

    if (pass == total) {
        printf("<html><body><h2 style='color:green;'>Accepted! (%d/%d)</h2>", pass, total);
    } else {
        printf("<html><body><h2 style='color:red;'>Wrong Answer (%d/%d)</h2>", pass, total);
    }
    printf("<a href='/contest.cgi?id=%d&token=%s'>Back to Contest</a></body></html>", cid, token);

    return 0;
}
