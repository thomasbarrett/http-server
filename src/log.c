#include <log.h>

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

void print_log(char *file, int line, char *fmt, ...) {
    time_t now;
    time(&now);
    struct tm *local = gmtime(&now);
    int h = local->tm_hour;
    int m = local->tm_min;
    int s = local->tm_sec;
    printf("%02d:%02d:%02d %s:%d ", h, m, s, file, line);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
}

