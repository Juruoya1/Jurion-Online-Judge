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

int get_submit(const char *user) {
    int cnt = 0;
    FILE *f = fopen(DATA_DIR "/submissions.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            char *time = strtok(line, "|");
            char *u = strtok(NULL, "|");
            if (u && strcmp(u, user) == 0) cnt++;
        }
        fclose(f);
    }
    return cnt;
}

int get_ac(const char *user) {
    int cnt = 0;
    FILE *f = fopen(DATA_DIR "/submissions.txt", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            char *time = strtok(line, "|");
            char *u = strtok(NULL, "|");
            char *pid = strtok(NULL, "|");
            char *res = strtok(NULL, "|");
            if (u && res && strcmp(u, user) == 0) {
                int a, b;
                if (sscanf(res, "%d/%d", &a, &b) == 2 && a == b) {
                    cnt++;
                } else if (strcmp(res, "Accepted") == 0) {
                    cnt++;
                }
            }
        }
        fclose(f);
    }
    return cnt;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", view[256] = "", user[256] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "user=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(view,p,n); view[n]=0; } else strcpy(view,p); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(user, sizeof(user), f); fclose(f); char *nl = strchr(user, '\n'); if(nl)*nl=0; }
    }
    
    if (view[0] == 0) strcpy(view, user);
    int is_owner = (strcmp(view, user) == 0);
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    
    if (view[0] == 0) {
        printf("<html><body><h2>Please <a href='/login.cgi'>login</a></h2></body></html>");
        return 0;
    }
    
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) { printf("<html><body><h2>User not found</h2></body></html>"); return 0; }
    
    char line[512], uid[32]="", nick[256]="", email[256]="", role_str[32]="";
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        char *pwd = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        char *uidv = strtok(NULL, "|");
        char *nickv = strtok(NULL, "|");
        char *mailv = strtok(NULL, "|");
        if (u && strcmp(u, view) == 0) {
            strcpy(uid, uidv);
            strcpy(nick, nickv ? nickv : u);
            strcpy(email, mailv ? mailv : "");
            if (role) {
                int r = atoi(role);
                if (r == 2) strcpy(role_str, "SU");
                else if (r == 1) strcpy(role_str, "Admin");
                else strcpy(role_str, "User");
            }
            found = 1;
            break;
        }
    }
    fclose(f);
    if (!found) { printf("<html><body><h2>User not found</h2></body></html>"); return 0; }
    
    int sub = get_submit(view);
    int ac = get_ac(view);
    int rate = sub > 0 ? ac * 100 / sub : 0;
    
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<div style='text-align:right; padding:10px; background:#f0f0f0;'>");
    if (user[0]) {
        printf("<a href='/index.cgi?token=%s'>Home</a> | ", token);
        printf("Welcome, <b>%s</b>", user);
        printf(" | <a href='/logout.cgi?token=%s'>Logout</a>", token);
    } else {
        printf("<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>");
    }
    printf("</div>");
    
    printf("<h1>User Profile</h1>");
    printf("<div style='border:1px solid #ccc; padding:15px;'>");
    printf("<p><strong>User ID:</strong> %s</p>", uid);
    printf("<p><strong>Username:</strong> %s</p>", view);
    printf("<p><strong>Nickname:</strong> %s</p>", nick);
    printf("<p><strong>Email:</strong> %s</p>", email);
    printf("<p><strong>Role:</strong> %s</p>", role_str);
    printf("<hr>");
    printf("<p><strong>Submissions:</strong> %d</p>", sub);
    printf("<p><strong>Accepted:</strong> %d</p>", ac);
    printf("<p><strong>AC Rate:</strong> %d%%</p>", rate);
    printf("</div>");
    
    if (is_owner) {
        printf("<h2>Edit Profile</h2>");
        printf("<form method='get' action='/update_profile.cgi'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("Nickname: <input type='text' name='nick' value='%s'><br><br>", nick);
        printf("Email: <input type='email' name='mail' value='%s'><br><br>", email);
        printf("<input type='submit' value='Update'>");
        printf("</form>");
        
        printf("<hr>");
        printf("<h2>Account</h2>");
        printf("<p><a href='/change_password.cgi?token=%s'>Change Password</a></p>", token);
        printf("<p><a href='/create_post.cgi?token=%s'>Write Post</a></p>", token);
    }
    
    printf("<h2>Posts by %s</h2>", view);
    printf("<ul>");
    FILE *pf = fopen(DATA_DIR "/posts.txt", "r");
    if (pf) {
        char pline[4096];
        while (fgets(pline, sizeof(pline), pf)) {
            char *pid = strtok(pline, "|");
            char *pauthor = strtok(NULL, "|");
            char *ptitle = strtok(NULL, "|");
            char *pcontent = strtok(NULL, "|");
            char *ptime = strtok(NULL, "|");
            if (pauthor && strcmp(pauthor, view) == 0) {
                time_t t = atol(ptime);
                printf("<li><a href='/view_post.cgi?token=%s&id=%s'><b>%s</b></a> - %s</li>", token, pid, ptitle, ctime(&t));
            }
        }
        fclose(pf);
    } else {
        printf("<li>No posts yet</li>");
    }
    printf("</ul>");
    
    printf("<p><a href='/index.cgi?token=%s'>Back to Home</a></p>", token);
    printf("</body></html>");
    return 0;
}

