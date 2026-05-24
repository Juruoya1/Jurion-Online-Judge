#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <dirent.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define TESTDATA_DIR "/mnt/hdd/jurionoj/data/testdata"
#define JAIL_DIR "/tmp/sandbox"

void setup_jail() {
    mkdir(JAIL_DIR, 0755);
    mkdir(JAIL_DIR "/bin", 0755);
    mkdir(JAIL_DIR "/lib", 0755);
    mkdir(JAIL_DIR "/lib/arm-linux-gnueabihf", 0755);
    mkdir(JAIL_DIR "/tmp", 0777);
    system("cp /lib/ld-linux-armhf.so.3 " JAIL_DIR "/lib/ 2>/dev/null");
    system("cp /lib/arm-linux-gnueabihf/libc.so.6 " JAIL_DIR "/lib/arm-linux-gnueabihf/ 2>/dev/null");
}

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
int run_in_jail(const char *exe, const char *in_file, const char *out_file, int *time_used, int *mem_used) {
    pid_t pid = fork();
    if (pid == 0) {
        struct rlimit rl;
        rl.rlim_cur = 2; rl.rlim_max = 2;
        setrlimit(RLIMIT_CPU, &rl);
        rl.rlim_cur = 64 * 1024 * 1024;
        setrlimit(RLIMIT_AS, &rl);
        setgid(65534);
        setuid(65534);
        chroot(JAIL_DIR);
        chdir("/");
        freopen(in_file, "r", stdin);
        freopen(out_file, "w", stdout);
        freopen("/dev/null", "w", stderr);
        execl(exe, "program", NULL);
        exit(1);
    } else if (pid > 0) {
        int status;
        struct rusage usage;
        wait4(pid, &status, 0, &usage);
        *time_used = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
        *mem_used = usage.ru_maxrss;
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        if (WIFSIGNALED(status)) return 2;
    }
    return -1;
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
        FILE *f = fopen(sf, "r");
        if (f) { fgets(name, sizeof(name), f); fclose(f); char *nl = strchr(name, '\n'); if(nl)*nl=0; }
    }
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<div style='text-align:right; padding:10px; background:#f0f0f0;'>");
    printf("Welcome, <b>%s</b> | <a href='/logout.cgi?token=%s'>Logout</a>", name, token);
    printf("</div>");
    
    if (id == 0 || code[0] == 0) {
        printf("<h2>Missing ID or code</h2></body></html>");
        return 0;
    }
    
    char dec[65536];
    url_decode(dec, code);
    
    char cf[256];
    sprintf(cf, "/tmp/code_%d_%ld.%s", id, time(NULL),
            strcmp(lang, "py")==0 ? "py" : (strcmp(lang,"cpp")==0 ? "cpp" : "c"));
    FILE *fp = fopen(cf, "w");
    if (!fp) { printf("<h2>Failed to save</h2></body></html>"); return 0; }
    fwrite(dec, 1, strlen(dec), fp);
    fclose(fp);
    
    char prog[256];
    sprintf(prog, "%s/program", JAIL_DIR);
    
    if (strcmp(lang, "c") == 0 || strcmp(lang, "cpp") == 0) {
        char cmd[512];
        if (strcmp(lang, "cpp") == 0)
            sprintf(cmd, "g++ -static -o %s %s 2>&1", prog, cf);
        else
            sprintf(cmd, "gcc -static -o %s %s 2>&1", prog, cf);
        if (system(cmd) != 0) {
            printf("<h2 style='color:red;'>Compile Error</h2>");
            printf("</body></html>");
            return 0;
        }
    }
    
    setup_jail();
    
    if (strcmp(lang, "c") == 0 || strcmp(lang, "cpp") == 0) {
        char cp_cmd[512];
        sprintf(cp_cmd, "cp %s %s", prog, prog);
        system(cp_cmd);
    }
    
    int total = count_tc(id);
    int pass = 0;
    
    for (int i = 1; i <= total; i++) {
        char in[256], out[256], jin[256], jout[256];
        sprintf(in, "%s/%d/%d.in", TESTDATA_DIR, id, i);
        sprintf(out, "%s/%d/%d.out", TESTDATA_DIR, id, i);
        sprintf(jin, "%s/%d.in", JAIL_DIR, i);
        sprintf(jout, "%s/%d.out", JAIL_DIR, i);
        
        if (access(in, F_OK) != 0) break;
        
        char cp_cmd[512];
        sprintf(cp_cmd, "cp %s %s", in, jin);
        system(cp_cmd);
        
        int tu, mu, ret;
        if (strcmp(lang, "py") == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                setgid(65534);
                setuid(65534);
                chroot(JAIL_DIR);
                freopen(jin, "r", stdin);
                freopen(jout, "w", stdout);
                execl("/usr/bin/python", "python", cf, NULL);
                exit(1);
            } else {
                int status;
                struct rusage usage;
                wait4(pid, &status, 0, &usage);
                tu = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
                mu = usage.ru_maxrss;
                ret = WIFEXITED(status) ? WEXITSTATUS(status) : 2;
            }
        } else {
            ret = run_in_jail(prog, jin, jout, &tu, &mu);
        }
        
        if (ret == 2) {
            printf("<h2>TLE on test %d</h2>", i);
            break;
        }
        
        char diff_cmd[512];
        sprintf(diff_cmd, "diff %s %s > /dev/null 2>&1", jout, out);
        if (system(diff_cmd) == 0) pass++;
    }
    
    FILE *sub = fopen(DATA_DIR "/submissions.txt", "a");
    if (sub) {
        fprintf(sub, "%ld|%s|%d|%d/%d\n", time(NULL), name, id, pass, total);
        fclose(sub);
    }
    
    if (pass == total) printf("<h2 style='color:green;'>Accepted! (%d/%d)</h2>", pass, total);
    else if (pass > 0) printf("<h2 style='color:orange;'>Partial (%d/%d)</h2>", pass, total);
    else printf("<h2 style='color:red;'>Wrong Answer (0/%d)</h2>", total);
    
    printf("<a href='/get_problem.cgi?id=%d&token=%s'>Back</a>", id, token);
    printf("</body></html>");
    return 0;
}

