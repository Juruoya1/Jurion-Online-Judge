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

int user_exists(const char *username) {
    FILE *f = fopen(DATA_DIR "/users.txt", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *p = strchr(line, '|');
        if (p) { *p = 0; if (strcmp(line, username) == 0) { fclose(f); return 1; } }
    }
    fclose(f);
    return 0;
}

int get_next_id() {
    FILE *f = fopen(DATA_DIR "/last_id.txt", "r");
    int id = 10001;
    if (f) { fscanf(f, "%d", &id); fclose(f); }
    id++;
    f = fopen(DATA_DIR "/last_id.txt", "w");
    if (f) { fprintf(f, "%d", id); fclose(f); }
    return id;
}

int main() {
    char *query = getenv("QUERY_STRING");
    
    // 如果没有参数，显示表单
    if (!query || query[0] == 0) {
        printf("Content-Type: text/html\n\n");
        printf("<html><body>");
        printf("<h2>Register</h2>");
        printf("<form method='get'>");
        printf("Email: <input type='email' name='email'><br>");
        printf("Username: <input type='text' name='username'><br>");
        printf("Nickname: <input type='text' name='nickname'><br>");
        printf("Password: <input type='text' name='password'><br>");
        printf("Confirm: <input type='text' name='confirm'><br>");
        printf("<input type='submit' value='Register'>");
        printf("</form></body></html>");
        return 0;
    }
    
    // 解析 GET 参数
    char email[256] = "", username[256] = "", nickname[256] = "", password[256] = "", confirm[256] = "";
    char *p = strstr(query, "email=");
    if (p) sscanf(p + 6, "%255[^&]", email);
    p = strstr(query, "username=");
    if (p) sscanf(p + 9, "%255[^&]", username);
    p = strstr(query, "nickname=");
    if (p) sscanf(p + 9, "%255[^&]", nickname);
    p = strstr(query, "password=");
    if (p) sscanf(p + 9, "%255[^&]", password);
    p = strstr(query, "confirm=");
    if (p) sscanf(p + 8, "%255[^&]", confirm);
    
    char email_dec[256], user_dec[256], nick_dec[256];
    url_decode(email_dec, email);
    url_decode(user_dec, username);
    url_decode(nick_dec, nickname);
    
    printf("Content-Type: text/html\n\n");
    
    if (email_dec[0] == 0 || user_dec[0] == 0 || password[0] == 0) {
        printf("<html><body><h2>All fields required</h2><a href='/register.cgi'>Back</a></body></html>");
        return 0;
    }
    if (strcmp(password, confirm) != 0) {
        printf("<html><body><h2>Passwords do not match</h2><a href='/register.cgi'>Back</a></body></html>");
        return 0;
    }
    if (user_exists(user_dec)) {
        printf("<html><body><h2>Username exists</h2><a href='/register.cgi'>Back</a></body></html>");
        return 0;
    }
    
    int user_id = get_next_id();
    FILE *f = fopen(DATA_DIR "/users.txt", "a");
    fprintf(f, "%s|%s|0|%d|%s|%s\n", user_dec, password, user_id, nick_dec[0] ? nick_dec : user_dec, email_dec);
    fclose(f);
    
    printf("<html><head>");
    printf("<meta http-equiv='refresh' content='2;url=/login.cgi'>");
    printf("</head><body>");
    printf("<h2>Registration success!</h2>");
    printf("<p>User ID: %d</p>", user_id);
    printf("<p>Username: %s</p>", user_dec);
    printf("<p>Redirecting to <a href='/login.cgi'>login page</a>...</p>");
    printf("</body></html>");
    return 0;
}
