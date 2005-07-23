/* $Id: log.c 40 2005-07-23 16:40:40Z matled $ {{{
   vim:sw=4:sts=4:et:
   Copyright (C) 2005 Matthias Lederhofer <matled@gmx.net>

   This file is part of mtsp.

   mtsp is free software; you can redistribute it and/or modify it under the
   terms of the GNU General Public License as published by the Free Software
   Foundation; either version 2 of the License, or (at your option) any later
   version.

   mtsp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License along with
   mtsp; if not, write to the Free Software Foundation, Inc., 51 Franklin St,
   Fifth Floor, Boston, MA  02110-1301  USA  }}} */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include "log.h"
#include "mtsp.h"

extern struct global_t global;

char *itoa(int n)
/* {{{ */ {
    static char buf[40];
    char *pos = buf+39;
    short int negative;
    *pos-- = '\0';
    if (n == 0) {
        *pos = '0';
        return pos;
    }
    if (n < 0) {
        negative = 1;
        n = -n;
    } else {
        negative = 0;
    }
    while (pos > buf && n) {
        *pos-- = n%10+'0';
        n /= 10;
    }
    if (negative) {
        *pos = '-';
        return pos;
    }
    return pos+1;
} /* }}} */

static char *__itoa(int n)
/* {{{ */ {
    static char buf[40];
    char *pos = buf+39;
    short int negative;
    *pos-- = '\0';
    if (n == 0) {
        *pos = '0';
        return pos;
    }
    if (n < 0) {
        negative = 1;
        n = -n;
    } else {
        negative = 0;
    }
    while (pos > buf && n) {
        *pos-- = n%10+'0';
        n /= 10;
    }
    if (negative) {
        *pos = '-';
        return pos;
    }
    return pos+1;
} /* }}} */

static char *log_time(void)
/* {{{ */ {
    static char buf[5+3+3+3+3+3];
    static time_t t = 0;
    time_t newtime = time(NULL);
    struct tm *lt;
    char *pos;

    if (t == newtime) return buf;

    t = newtime;
    lt = localtime(&t);
    lt->tm_year += 1900;
    lt->tm_mon += 1;
    pos = buf;
    *pos++ = lt->tm_year/1000%10+'0';
    *pos++ = lt->tm_year/100 %10+'0';
    *pos++ = lt->tm_year/10  %10+'0';
    *pos++ = lt->tm_year     %10+'0';
    *pos++ = '-';
    *pos++ = lt->tm_mon/10   %10+'0';
    *pos++ = lt->tm_mon      %10+'0';
    *pos++ = '-';
    *pos++ = lt->tm_mday/10  %10+'0';
    *pos++ = lt->tm_mday     %10+'0';
    *pos++ = ' ';
    *pos++ = lt->tm_hour/10  %10+'0';
    *pos++ = lt->tm_hour     %10+'0';
    *pos++ = ':';
    *pos++ = lt->tm_min/10   %10+'0';
    *pos++ = lt->tm_min      %10+'0';
    *pos++ = ':';
    *pos++ = lt->tm_sec/10   %10+'0';
    *pos++ = lt->tm_sec      %10+'0';
    *pos++ = '\0';
    return buf;
} /* }}} */

static int log_open(void)
/* {{{ */ {
    if (!global.options.loglevel) return 0;
    if (global.options.logfile == NULL) return 1;
    return open(global.options.logfile, O_APPEND);
} /*}}} */

static void log_close(int fd)
/* {{{ */ {
    if (global.options.logfile != NULL) close(fd);
} /*}}} */

static void __log(char type, int clientfd, const char *arg, va_list ap)
/* {{{ */ {
    struct iovec iov[LOG_MAX_IOVEC];
    int fd = log_open();
    int n = -1;
    char *str;

    if (fd == -1) return;

    iov[++n].iov_base = log_time();
    iov[n].iov_len = strlen(iov[n].iov_base);
    iov[++n].iov_base = (void*)" [";
    iov[n].iov_len = 2;
    iov[++n].iov_base = (void*)&type;
    iov[n].iov_len = 1;
    if (clientfd >= 0) {
        iov[++n].iov_base = (void*)":";
        iov[n].iov_len = 1;
        iov[++n].iov_base = __itoa(clientfd);
        iov[n].iov_len = strlen(iov[n].iov_base);
    }
    iov[++n].iov_base = (void*)"] ";
    iov[n].iov_len = 2;

    iov[++n].iov_base = (void*)arg;
    iov[n].iov_len = strlen(iov[n].iov_base);

    while (n+1 < LOG_MAX_IOVEC && (str = va_arg(ap, char*)) != NULL) {
        iov[++n].iov_base = str;
        iov[n].iov_len = strlen(iov[n].iov_base);
    }
    iov[++n].iov_base = (void*)"\n";
    iov[n].iov_len = 1;

    writev(fd, iov, n+1);
    log_close(fd);
} /* }}} */

void log_notice(const char *arg, ...)
/* {{{ */ {
    va_list ap;
    va_start(ap, arg);
    __log('n', -1, arg, ap);
    va_end(ap);
} /* }}} */

void log_warning(const char *arg, ...)
/* {{{ */ {
    va_list ap;
    va_start(ap, arg);
    __log('w', -1, arg, ap);
    va_end(ap);
} /* }}} */

void log_error(const char *arg, ...)
/* {{{ */ {
    va_list ap;
    va_start(ap, arg);
    __log('e', -1, arg, ap);
    va_end(ap);
} /* }}} */

void logc_notice(int clientfd, const char *arg, ...)
/* {{{ */ {
    va_list ap;
    va_start(ap, arg);
    __log('n', clientfd, arg, ap);
    va_end(ap);
} /* }}} */

void logc_warning(int clientfd, const char *arg, ...)
/* {{{ */ {
    va_list ap;
    va_start(ap, arg);
    __log('w', clientfd, arg, ap);
    va_end(ap);
} /* }}} */

void logc_error(int clientfd, const char *arg, ...)
/* {{{ */ {
    va_list ap;
    va_start(ap, arg);
    __log('e', clientfd, arg, ap);
    va_end(ap);
} /* }}} */
