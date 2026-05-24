#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"
#define TESTDATA_DIR "/mnt/hdd/jurionoj/data/testdata"
#define PROBLEM_DATA_DIR "/mnt/hdd/jurionoj/data/problem_data"

void remove_directory(const char *path) {
    DIR *d = opendir(path);
    if (!d) return;
    
    struct dirent *e;
    char fullpath[512];
    struct stat st;
    
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, e->d_name);
        
        if (stat(fullpath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                remove_directory(fullpath);
                rmdir(fullpath);
            } else {
                remove(fullpath);
            }
        }
    }
    closedir(d);
}

int get_user_role(const char *username) {
    if (!username || username[0] == '\0') return -1;
    
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return -1;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        if (u && role && strcmp(u, username) == 0) {
            fclose(f);
            return atoi(role);
        }
    }
    fclose(f);
    return -1;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", username[256] = "";
    int pid = 0;

    // 解析参数
    if (q) {
        char *p = strstr(q, "token=");
        if (p) {
            p += 6;
            char *e = strchr(p, '&');
            if (e) {
                int n = e - p;
                if (n > 255) n = 255;
                strncpy(token, p, n);
                token[n] = 0;
            } else {
                strcpy(token, p);
            }
        }
        
        p = strstr(q, "id=");
        if (p) {
            p += 3;
            pid = atoi(p);
        }
    }

    // 验证登录
    if (token[0]) {
        char sf[512];
        sprintf(sf, "%s/sessions/%s", DATA_DIR, token);
        FILE *f = fopen(sf, "r");
        if (f) {
            fgets(username, sizeof(username), f);
            fclose(f);
            char *nl = strchr(username, '\n');
            if (nl) *nl = 0;
        }
    }

    int role = get_user_role(username);
    int is_admin = (role == 1 || role == 2);

    printf("Content-Type: text/html; charset=utf-8\n\n");

    if (!is_admin) {
        printf("<html><body>");
        printf("<h2>Permission denied</h2>");
        printf("<a href='/admin.cgi?token=%s'>Back</a>", token);
        printf("</body></html>");
        return 0;
    }

    if (pid < 1000) {
        printf("<html><body>");
        printf("<h2>Invalid problem ID: %d</h2>", pid);
        printf("<a href='/admin.cgi?token=%s'>Back</a>", token);
        printf("</body></html>");
        return 0;
    }

    // 1. 从 problems.txt 删除该行
    char problems_path[512];
    sprintf(problems_path, "%s/problems.txt", DATA_DIR);
    FILE *fin = fopen(problems_path, "r");
    if (!fin) {
        printf("<html><body><h2>Error: Cannot open problems.txt</h2></body></html>");
        return 0;
    }

    char temp_path[512];
    sprintf(temp_path, "%s/problems.txt.tmp", DATA_DIR);
    FILE *ftmp = fopen(temp_path, "w");
    if (!ftmp) {
        fclose(fin);
        printf("<html><body><h2>Error: Cannot create temp file</h2></body></html>");
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), fin)) {
        int current_id = 0;
        sscanf(line, "%d", &current_id);
        if (current_id == pid) {
            found = 1;
            continue;
        }
        fputs(line, ftmp);
    }
    fclose(fin);
    fclose(ftmp);

    if (!found) {
        remove(temp_path);
        printf("<html><body>");
        printf("<h2>Problem %d not found</h2>", pid);
        printf("<a href='/admin.cgi?token=%s'>Back</a>", token);
        printf("</body></html>");
        return 0;
    }

    rename(temp_path, problems_path);

    // 2. 删除 testdata 目录
    char testdata_dir[512];
    sprintf(testdata_dir, "%s/%d", TESTDATA_DIR, pid);
    remove_directory(testdata_dir);
    rmdir(testdata_dir);

    // 3. 删除 problem_data 目录
    char probdata_dir[512];
    sprintf(probdata_dir, "%s/%d", PROBLEM_DATA_DIR, pid);
    remove_directory(probdata_dir);
    rmdir(probdata_dir);

    printf("<html><body>");
    printf("<h2>Problem %d deleted successfully!</h2>", pid);
    printf("<a href='/admin.cgi?token=%s'>Back to Admin</a><br>", token);
    printf("<a href='/index.cgi?token=%s'>Home</a>", token);
    printf("</body></html>");
    
    return 0;
}
