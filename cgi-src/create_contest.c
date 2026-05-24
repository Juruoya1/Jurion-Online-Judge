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

int get_next_id() {
    FILE *f = fopen(DATA_DIR "/contests.txt", "r");
    int max = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        int id = atoi(strtok(line, "|"));
        if (id > max) max = id;
    }
    fclose(f);
    return max + 1;
}

time_t parse_datetime(const char *str) {
    struct tm tm = {0};
    int year, month, day, hour, min;
    char decoded[64];
    strcpy(decoded, str);
    for (int i = 0; decoded[i]; i++) {
        if (decoded[i] == '+') decoded[i] = ' ';
    }
    if (sscanf(decoded, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &min) == 5) {
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = min;
        tm.tm_sec = 0;
        return mktime(&tm);
    }
    return 0;
}

int main() {
    char *q = getenv("QUERY_STRING");
    char token[256] = "", username[256] = "";
    char title[512] = "", desc[2048] = "", problems[512] = "";
    char start_str[64] = "", end_str[64] = "", duration_str[16] = "";
    
    if (q) {
        char *p = strstr(q, "token=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>255)n=255; strncpy(token,p,n); token[n]=0; } else strcpy(token,p); }
        p = strstr(q, "title=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>511)n=511; strncpy(title,p,n); title[n]=0; } else strcpy(title,p); }
        p = strstr(q, "desc=");
        if (p) { p += 5; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>2047)n=2047; strncpy(desc,p,n); desc[n]=0; } else strcpy(desc,p); }
        p = strstr(q, "problems=");
        if (p) { p += 9; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>511)n=511; strncpy(problems,p,n); problems[n]=0; } else strcpy(problems,p); }
        p = strstr(q, "start=");
        if (p) { p += 6; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>63)n=63; strncpy(start_str,p,n); start_str[n]=0; } else strcpy(start_str,p); }
        p = strstr(q, "end=");
        if (p) { p += 4; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>63)n=63; strncpy(end_str,p,n); end_str[n]=0; } else strcpy(end_str,p); }
        p = strstr(q, "duration=");
        if (p) { p += 9; char *e = strchr(p, '&'); if (e) { int n = e-p; if(n>15)n=15; strncpy(duration_str,p,n); duration_str[n]=0; } else strcpy(duration_str,p); }
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
        printf("<html><body><h2>Permission denied. Admin only.</h2><a href='/index.cgi?token=%s'>Back</a></body></html>", token);
        return 0;
    }
    
    if (title[0] == 0) {
        printf("<html><head><meta charset='UTF-8'></head><body>");
        printf("<h1>Create Contest</h1>");
        printf("<form method='get'>");
        printf("<input type='hidden' name='token' value='%s'>", token);
        printf("Title: <input type='text' name='title' size='50'><br><br>");
        printf("Description:<br><textarea name='desc' rows='5' cols='70'></textarea><br><br>");
        printf("Problem IDs (comma separated): <input type='text' name='problems' size='50'><br><br>");
        printf("Start Time (YYYY-MM-DD HH:MM): <input type='text' name='start' placeholder='2026-01-01 10:00'><br><br>");
        printf("End Time (YYYY-MM-DD HH:MM): <input type='text' name='end' placeholder='2026-01-03 18:00'><br><br>");
        printf("User Duration (minutes, 0=unlimited): <input type='text' name='duration' value='0'><br><br>");
        printf("<input type='submit' value='Create'>");
        printf("</form>");
        printf("<a href='/index.cgi?token=%s'>Back</a>", token);
        printf("</body></html>");
        return 0;
    }
    
    char title_dec[512], desc_dec[2048], problems_dec[512];
    url_decode(title_dec, title);
    url_decode(desc_dec, desc);
    url_decode(problems_dec, problems);
    
    char start_dec[64], end_dec[64];
    url_decode(start_dec, start_str);
    url_decode(end_dec, end_str);
    
    time_t start = parse_datetime(start_dec);
    time_t end = parse_datetime(end_dec);
    int user_duration = atoi(duration_str);
    
    if (start == 0 || end == 0 || start >= end) {
        printf("<html><body><h2>Invalid time range. Use format: YYYY-MM-DD HH:MM</h2><a href='/create_contest.cgi?token=%s'>Back</a></body></html>", token);
        return 0;
    }
    
    int id = get_next_id();
    FILE *f = fopen(DATA_DIR "/contests.txt", "a");
    fprintf(f, "%d|%s|%s|%ld|%ld|%s|%s|%d\n", id, title_dec, desc_dec, start, end, problems_dec, username, user_duration);
    fclose(f);
    
    printf("<html><body>");
    printf("<h2>Contest created!</h2>");
    printf("<p>ID: %d, Title: %s</p>", id, title_dec);
    printf("<p>Start: %s, End: %s</p>", start_str, end_str);
    printf("<p>User Duration: %d minutes</p>", user_duration);
    printf("<a href='/list_contests.cgi?token=%s'>View Contests</a><br>", token);
    printf("<a href='/index.cgi?token=%s'>Home</a>", token);
    printf("</body></html>");
    return 0;
}
