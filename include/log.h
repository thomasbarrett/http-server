#ifndef LOG_H
#define LOG_H

#define log(...) print_log(__FILE__, __LINE__, __VA_ARGS__)

void print_log(char *file, int line, char *fmt, ...);

#endif /* LOG_H */

