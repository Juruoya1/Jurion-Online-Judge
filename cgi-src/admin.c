#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char token[256] = "", username[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
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
        printf("<html><body><h2>Permission denied</h2><a href='/index.cgi?token=%s'>Back</a></body></html>", token);
        return 0;
    }
    
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<h1>Admin Panel</h1>");
    printf("<div><a href='/index.cgi?token=%s'>Home</a> | <a href='/logout.cgi?token=%s'>Logout</a></div>", token, token);
    printf("<hr>");
    
    //公告管理
    printf("<h2>Announcement</h2>");
    printf("<p><a href='/admin_announcement.cgi?token=%s'>Edit Announcement</a></p>", token);
    // 用户列表
    printf("<h2>Users</h2>");
    printf("<table border='1' cellpadding='5'>");
    printf("<tr><th>User ID</th><th>Username</th><th>Nickname</th><th>Email</th><th>Role</th><th>Action</th></tr>");
    
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            char *u = strtok(line, "|");
            char *pwd = strtok(NULL, "|");
            char *role_str = strtok(NULL, "|");
            char *uid = strtok(NULL, "|");
            char *nick = strtok(NULL, "|");
            char *mail = strtok(NULL, "|");
            if (u) {
                int r = atoi(role_str);
                char *rname = r == 2 ? "SU" : (r == 1 ? "Admin" : "User");
                printf("<tr>");
                printf("<td>%s</a></td>", uid);
                printf("<td>%s</a></td>", u);
                printf("<td>%s</a></td>", nick ? nick : u);
                printf("<td>%s</a></td>", mail ? mail : "");
                printf("<td>%s</a></td>", rname);
                if (r != 2 && strcmp(u, username) != 0) {
                    printf("<td><a href='/delete_user.cgi?token=%s&username=%s' onclick='return confirm(\"Delete %s?\")'>Delete</a></a></td>", token, u, u);
                } else {
                    printf("<td>-</a></td>");
                }
                printf("</tr>");
            }
        }
        fclose(f);
    }
    printf("</table>");
    
    // SU 专用
    if (role == 2) {
        printf("<h2>Manage Admins (SU only)</h2>");
        printf("<form method='get' action='/manage_admin.cgi'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("Username: <input type='text' name='username'>");
        printf("<select name='action'>");
        printf("<option value='add'>Add Admin</option>");
        printf("<option value='remove'>Remove Admin</option>");
        printf("</select>");
        printf("<input type='submit' value='Go'>");
        printf("</form>");
        
        printf("<p><a href='/view_logs.cgi?token=%s'>View Server Logs</a></p>", token);
    }
    
    // 删除题目
    printf("<h2>Delete Problem</h2>");
    printf("<form method='get' action='/delete_problem.cgi'>");
    printf("<input type='hidden' name='token' value='%s'>", token);
    printf("Problem ID: <input type='text' name='id'>");
    printf("<input type='submit' value='Delete' onclick='return confirm(\"Delete problem?\")'>");
    printf("</form>");
    
    // 删除比赛
    printf("<h2>Delete Contest</h2>");
    printf("<form method='get' action='/delete_contest.cgi'>");
    printf("<input type='hidden' name='token' value='%s'>", token);
    printf("Contest ID: <input type='text' name='id'>");
    printf("<input type='submit' value='Delete' onclick='return confirm(\"Delete contest?\")'>");
    printf("</form>");
    
    printf("<h2>Manage Contest</h2>");
    printf("<form method='get' action='/admin_contest.cgi'>");
    printf("<input type='hidden' name='token' value='%s'>", token);
    printf("Contest ID: <input type='text' name='id'>");
    printf("<input type='submit' value='Manage'>");
    printf("</form>");

    printf("</body></html>");
    return 0;
}
