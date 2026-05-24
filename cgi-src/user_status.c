#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *cookie = getenv("HTTP_COOKIE");
    char username[256] = "";
    
    printf("Content-Type: text/html\n\n");
    
    if (cookie && strstr(cookie, "logged_in=1")) {
        char *p = strstr(cookie, "username=");
        if (p) {
            p += 9;
            char *end = strchr(p, ';');
            if (end) {
                int n = end - p;
                if (n > 255) n = 255;
                strncpy(username, p, n);
                username[n] = 0;
            } else {
                strcpy(username, p);
            }
        }
        printf("<div style='text-align:right; padding:10px; background:#e0e0e0;'>");
        printf("Welcome, <b>%s</b> | <a href='/logout.cgi'>Logout</a>", username);
        printf("</div>");
    } else {
        printf("<div style='text-align:right; padding:10px; background:#e0e0e0;'>");
        printf("<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>");
        printf("</div>");
    }
    return 0;
}
