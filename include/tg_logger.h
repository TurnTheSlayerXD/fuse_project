

#ifndef TG_LOGGER_H
#define TG_LOGGER_H

#include <stdarg.h>
#include <stdio.h>

void tg_log(const char *fmt, ...)
{
    FILE *f = fopen("./tg_logs.txt", "a");
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fclose(f);
}

#endif