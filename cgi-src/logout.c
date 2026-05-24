#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "/mnt/hdd/jurionoj/data"

int main() {
    char *query = getenv("QUERY_STRING");
    if (query && strstr(query, "session=")) {
        char session_id[256];
        char *p = strstr(query, "session=") + 8;
        char *end = strchr(p, '&');
        if (end) { strncpy(session_id, p, end-p); session_id[end-p]=0; }
        else strcpy(session_id, p);
        char session_file[512];
        sprintf(session_file, DATA_DIR "/sessions/%s", session_id);
        remove(session_file);
    }
    printf("Content-Type: text/html\n\n");
    printf("<html><body><h2>Logged out</h2><a href='/login.cgi'>Login</a></body></html>");
    return 0;
}
