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

void get_time_str(time_t t, char *buf, int size) {
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M", tm);
}

void format_duration(long seconds, char *buf, int size) {
    int hours = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;
    if (hours > 0)
        snprintf(buf, size, "%d小时%d分", hours, mins);
    else if (mins > 0)
        snprintf(buf, size, "%d分%d秒", mins, secs);
    else
        snprintf(buf, size, "%d秒", secs);
}

int user_in_contest(int cid, const char *username, time_t *start_time) {
    FILE *f = fopen(DATA_DIR "/contest_participants.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *id_str = strtok(line, "|");
        char *user = strtok(NULL, "|");
        char *uid = strtok(NULL, "|");
        char *st = strtok(NULL, "|");
        if (id_str && user && atoi(id_str) == cid && strcmp(user, username) == 0) {
            if (start_time) *start_time = atol(st);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int get_user_duration(int cid) {
    FILE *f = fopen(DATA_DIR "/contests.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *pid = strtok(line, "|");
        if (pid && atoi(pid) == cid) {
            strtok(NULL, "|"); // title
            strtok(NULL, "|"); // desc
            strtok(NULL, "|"); // start
            strtok(NULL, "|"); // end
            strtok(NULL, "|"); // problems
            strtok(NULL, "|"); // creator
            char *dur = strtok(NULL, "|");
            fclose(f);
            return dur ? atoi(dur) : 0;
        }
    }
    fclose(f);
    return 0;
}

int get_problem_status(int cid, int uid, int pid) {
    FILE *f = fopen(DATA_DIR "/contest_submissions.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        int c, u, p;
        if (sscanf(line, "%d|%d|%d", &c, &u, &p) == 3) {
            if (c == cid && u == uid && p == pid && strstr(line, "|AC") != NULL) {
                fclose(f);
                return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

int get_user_uid(const char *username) {
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *u = strtok(line, "|");
        char *pwd = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        char *uid = strtok(NULL, "|");
        if (u && strcmp(u, username) == 0) {
            fclose(f);
            return atoi(uid);
        }
    }
    fclose(f);
    return 0;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", id_str[32] = "";
    char username[256] = "";
    int cid = 0;
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "id=");
        if (p) { p += 3; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>31)n=31; strncpy(id_str,p,n); id_str[n]=0; } else strcpy(id_str,p); cid = atoi(id_str); }
    }
    
    if (token[0]) {
        char sf[512];
        sprintf(sf, DATA_DIR "/sessions/%s", token);
        FILE *f = fopen(sf, "r");
        if (f) { fgets(username, sizeof(username), f); fclose(f); char *nl = strchr(username, '\n'); if(nl)*nl=0; }
    }
    
    int uid = get_user_uid(username);
    
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<html><head><meta charset='UTF-8'></head><body>");
    printf("<div style='text-align:right; padding:10px; background:#f0f0f0;'>");
    if (username[0]) {
        printf("<a href='/index.cgi?token=%s'>Home</a> | Welcome, <b>%s</b> | <a href='/logout.cgi?token=%s'>Logout</a>", token, username, token);
    } else {
        printf("<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>");
    }
    printf("</div>");
    
    FILE *f = fopen(DATA_DIR "/contests.txt", "r");
    if (!f) { printf("<h2>Contest not found</h2></body></html>"); return 0; }
    
    char line[512];
    int found = 0;
    char title_raw[256]="", desc_raw[2048]="", problems_str[512]="";
    time_t start=0, end=0;
    int user_duration_min = 0;
    
    while (fgets(line, sizeof(line), f)) {
        char *pid = strtok(line, "|");
        if (pid && atoi(pid) == cid) {
            strcpy(title_raw, strtok(NULL, "|"));
            strcpy(desc_raw, strtok(NULL, "|"));
            start = atol(strtok(NULL, "|"));
            end = atol(strtok(NULL, "|"));
            char *probs = strtok(NULL, "|");
            if (probs) strcpy(problems_str, probs);
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) { printf("<h2>Contest not found</h2></body></html>"); return 0; }
    
    char title[256], desc[4096];
    url_decode(title, title_raw);
    url_decode(desc, desc_raw);
    
    char start_buf[32], end_buf[32];
    get_time_str(start, start_buf, sizeof(start_buf));
    get_time_str(end, end_buf, sizeof(end_buf));
    
    printf("<h1>%s</h1>", title);
    printf("<pre>%s</pre>", desc);
    printf("<p><strong>Start:</strong> %s | <strong>End:</strong> %s</p>", start_buf, end_buf);
    
    long duration = end - start;
    char duration_str[64];
    format_duration(duration, duration_str, sizeof(duration_str));
    printf("<p><strong>Duration:</strong> %s</p>", duration_str);
    
    time_t now = time(NULL);
    time_t user_start = 0;
    int joined = user_in_contest(cid, username, &user_start);
    
    printf("<hr>");
    
    if (now < start) {
        printf("<p style='color:blue;'>Contest not started yet.</p>");
        if (username[0]) printf("<p><a href='/join_contest.cgi?id=%d&token=%s'>Join Contest</a></p>", cid, token);
    } else if (now >= start && now <= end) {
        long remaining = end - now;
        char remaining_str[64];
        format_duration(remaining, remaining_str, sizeof(remaining_str));
        printf("<p style='color:green;'>Contest is running! Remaining: %s</p>", remaining_str);
        
        if (!joined && username[0]) {
            printf("<p><a href='/join_contest.cgi?id=%d&token=%s'>Join Contest</a></p>", cid, token);
        } else if (joined) {
            long user_allowed = duration;
            if (user_duration_min > 0) {
                user_allowed = user_duration_min * 60;
            }
            long user_elapsed = now - user_start;
            long user_remaining = user_allowed - user_elapsed;
            if (user_remaining < 0) user_remaining = 0;
            char user_remaining_str[64];
            format_duration(user_remaining, user_remaining_str, sizeof(user_remaining_str));
            printf("<p>You are participating! Your remaining time: <strong>%s</strong></p>", user_remaining_str);
            
            printf("<h2>Problems</h2>");
            printf("<table border='1' cellpadding='5'>");
            printf("<tr><th>#</th><th>ID</th><th>Title</th><th>Status</th><th>Action</th></tr>");
            
            char probs_list[512];
            strcpy(probs_list, problems_str);
            char *prob = strtok(probs_list, ",");
            int idx = 1;
            while (prob != NULL) {
                int pid = atoi(prob);
                char ptitle[256] = "";
                char title_path[512];
                sprintf(title_path, "%s/problem_data/%d/title.txt", DATA_DIR, pid);
                FILE *tf = fopen(title_path, "r");
                if (tf) {
                    fgets(ptitle, sizeof(ptitle), tf);
                    char *nl = strchr(ptitle, '\n');
                    if (nl) *nl = 0;
                    fclose(tf);
                } else {
                    sprintf(ptitle, "Problem %d", pid);
                }
                
                int ac = get_problem_status(cid, uid, pid);
                
                printf("<tr>");
                printf("<td>%d</a></td>", idx);
                printf("<td>%d</a></td>", pid);
                printf("<td><a href='/contest_problem.cgi?cid=%d&pid=%d&token=%s' target='_blank'>%s</a></td>", cid, pid, token, ptitle);
                if (ac) {
                    printf("<td style='color:green;'>AC</a></td>");
                } else {
                    printf("<td>-</a></td>");
                }
                printf("<td><a href='/contest_problem.cgi?cid=%d&pid=%d&token=%s' target='_blank'>Submit</a></td>", cid, pid, token);
                printf("</tr>");
                idx++;
                prob = strtok(NULL, ",");
            }
            printf("</table>");
            printf("<p><a href='/contest_rank.cgi?id=%d&token=%s'>Ranking</a></p>", cid, token);
        }
    } else {
        printf("<p style='color:red;'>Contest ended.</p>");
        printf("<p><a href='/contest_rank.cgi?id=%d&token=%s'>View Final Ranking</a></p>", cid, token);
    }
    
    printf("<p><a href='/list_contests.cgi?token=%s'>Back</a></p>", token);
    printf("</body></html>");
    return 0;
}
