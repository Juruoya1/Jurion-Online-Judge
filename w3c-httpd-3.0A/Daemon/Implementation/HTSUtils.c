/* HTSUtils.c - Simplified version for JurionOJ */
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define PUBLIC
#define PRIVATE static
#define BOOL int
#define YES 1
#define NO 0
#define ARGS1(x,y) (y)
#define ARGS2(x,y,z) (y,z)
#define ARGS3(x,y,z,w) (y,z,w)
#define CONST const

PUBLIC char * month_names[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

PUBLIC BOOL HTIsNumber(CONST char *s) {
    if (!s || !*s) return NO;
    if (*s == '-') s++;
    while (*s) {
        if (*s < '0' || *s > '9') return NO;
        s++;
    }
    return YES;
}

PUBLIC time_t parse_http_time(char *str) {
    return time(NULL);
}

PUBLIC char * http_time(time_t *t) {
    static char buf[40];
    struct tm *gmt = gmtime(t);
    strftime(buf, 40, "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return buf;
}

PUBLIC char * make_time_spec(time_t *t) {
    static char buf[20];
    sprintf(buf, "%ld", t ? *t : 0);
    return buf;
}

PUBLIC BOOL parse_time(char *str, int def, time_t *tp) {
    *tp = def;
    return YES;
}

PUBLIC int parse_bytes(char *str, char def) {
    return atoi(str) * 1024;
}

PUBLIC char * get_http_reason(int status) {
    switch (status) {
        case 200: return "OK";
        case 404: return "Not Found";
        default: return "Unknown";
    }
}

PUBLIC int HTWriteASCII(int soc, char *s) {
    if (!s) return -1;
    return write(soc, s, strlen(s));
}
