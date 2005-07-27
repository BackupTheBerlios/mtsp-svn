/* $Id$ {{{
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
#include <stdarg.h>
#include <string.h>
#include <sys/uio.h>

#include "ewrite.h"

int ewrite(int fd, ...)
/* {{{ */ {
    int n = 0;
    char *buf;
    struct iovec iov[EWRITE_MAX_IOVEC];
    va_list ap;
    va_start(ap, fd);
    while (n < EWRITE_MAX_IOVEC && (buf = va_arg(ap, char*)) != NULL) {
        iov[n].iov_base = buf;
        iov[n].iov_len = strlen(buf);
        ++n;
    }
    va_end(ap);
    if (n > 0) {
        return writev(fd, iov, n);
    }
    return 0;
} /* }}} */
