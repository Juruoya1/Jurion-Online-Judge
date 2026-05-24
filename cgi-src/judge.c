#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

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

int count_tc(int id) {
    char path[256];
    sprintf(path, "%s/%d", TESTDATA_DIR, id);
    DIR *d = opendir(path);
    if (!d) return 1;
    int c = 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strstr(e->d_name, ".in")) c++;
    }
    closedir(d);
    return c > 0 ? c : 1;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", name[256] = "", lang[10] = "c", code[65536] = "";
    int id = 0;

    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "id=");
        if (p) id = atoi(p + 3);
        p = strstr(q, "lang=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>9)n=9; strncpy(lang,p,n); lang[n]=0; } else strcpy(lang,p); }
        p = strstr(q, "code=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>65535)n=65535; strncpy(code,p,n); code[n]=0; } else strcpy(code,p); }
    }

    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *fp = fopen(sf, "r");
        if (fp) {
            fgets(name, sizeof(name), fp);
            fclose(fp);
            char *nl = strchr(name, '\n');
            if (nl) *nl = 0;
        }
    }

    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<!DOCTYPE html><html><head><meta charset='UTF-8'></head><body>");
    printf("<div style='text-align:right;padding:10px;background:#f0f0f0;'>");
    printf("Welcome, <b>%s</b> | <a href='/logout.cgi?token=%s'>Logout</a>", name, token);
    printf("</div>");

    if (id == 0 || code[0] == 0) {
        printf("<h2>Missing ID or code</h2></body></html>");
        return 0;
    }

    char dec[65536];
    url_decode(dec, code);

    char cf[256];
    sprintf(cf, "/tmp/c_%d_%ld.%s", id, time(NULL),
            strcmp(lang, "py")==0 ? "py" : (strcmp(lang,"cpp")==0 ? "cpp" : "c"));
    FILE *fp = fopen(cf, "w");
    if (!fp) { printf("<h2>Failed to save code</h2></body></html>"); return 0; }
    fwrite(dec, 1, strlen(dec), fp);
    fclose(fp);

    if (strcmp(lang, "c") == 0 || strcmp(lang, "cpp") == 0) {
        char cmd[512];
        sprintf(cmd, "g++ -o /tmp/prog %s 2>&1", cf);
        FILE *pipe = popen(cmd, "r");
        char out[4096] = "", buf[256];
        while (fgets(buf, sizeof(buf), pipe)) strcat(out, buf);
        if (pclose(pipe) != 0) {
            printf("<h2 style='color:red;'>Compile Error</h2><pre>%s</pre>", out);
            printf("</body></html>");
            return 0;
        }
    }

    int total = count_tc(id);
    int pass = 0;

    for (int i = 1; i <= total; i++) {
        char in[256], out[256];
        sprintf(in, "%s/%d/%d.in", TESTDATA_DIR, id, i);
        sprintf(out, "%s/%d/%d.out", TESTDATA_DIR, id, i);
        if (access(in, F_OK) != 0) break;
        char cmd[512];
        if (strcmp(lang, "py") == 0) {
            sprintf(cmd, "python %s < %s > /tmp/out 2>&1", cf, in);
        } else {
            sprintf(cmd, "/tmp/prog < %s > /tmp/out 2>&1", in);
        }
        system(cmd);
        char diff[512];
        sprintf(diff, "diff -b /tmp/out %s > /dev/null 2>&1", out);
        if (system(diff) == 0) pass++;
    }

    FILE *sub = fopen(DATA_DIR "/submissions.txt", "a");
    if (sub) {
        fprintf(sub, "%ld|%s|%d|%d/%d\n", time(NULL), name, id, pass, total);
        fclose(sub);
    }

    if (pass == total) printf("<h2 style='color:green;'>Accepted! (%d/%d)</h2>", pass, total);
    else if (pass > 0) printf("<h2 style='color:orange;'>Partial (%d/%d)</h2>", pass, total);
    else printf("<h2 style='color:red;'>Wrong Answer (0/%d)</h2>", total);

    printf("<table border='1'>\n");
    printf("<tr><th>Test</th><th>Result</th></tr>\n");
    for (int i = 1; i <= total; i++) {
        char in[256], out[256];
        sprintf(in, "%s/%d/%d.in", TESTDATA_DIR, id, i);
        sprintf(out, "%s/%d/%d.out", TESTDATA_DIR, id, i);
        if (access(in, F_OK) != 0) break;
        char cmd[512];
        if (strcmp(lang, "py") == 0) {
            sprintf(cmd, "python %s < %s > /tmp/out 2>&1", cf, in);
        } else {
            sprintf(cmd, "/tmp/prog < %s > /tmp/out 2>&1", in);
        }
        system(cmd);
        char diff[512];
        sprintf(diff, "diff -w /tmp/out %s > /dev/null 2>&1", out);
        if (system(diff) == 0) {
            printf("<tr><td>%d</td><td style='color:green;'>PASS</td></tr>\n", i);
        } else {
            printf("<tr><td>%d</td><td style='color:red;'>FAIL</td></tr>\n", i);
        }
    }
    printf("</table>\n");
    printf("<a href='/get_problem.cgi?id=%d&token=%s'>Back</a>", id, token);
    printf("</body></html>");
    return 0;
}
