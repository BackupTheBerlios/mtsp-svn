/* $Id: log.h 37 2005-07-22 21:44:12Z matled $ {{{
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
#ifndef LOG_H
#define LOG_H 1

/* 8 are used by logc_* */
#define LOG_MAX_IOVEC 15

#ifdef DEBUG
#define logc_debug logc_notice
#define log_debug log_notice
#else
#define logc_debug(...) ((void)0)
#define log_debug(...)  ((void)0)
#endif

char *itoa(int n);

void log_notice(const char *arg, ...);
void log_warning(const char *arg, ...);
void log_error(const char *arg, ...);

void logc_notice(int clientfd, const char *arg, ...);
void logc_warning(int clientfd, const char *arg, ...);
void logc_error(int clientfd, const char *arg, ...);

#endif
