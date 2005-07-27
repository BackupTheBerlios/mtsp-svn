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
#ifndef BUFFER_H
#define BUFFER_H

#include <sys/uio.h>
#include <unistd.h>
#include "config.h"

/* BUF_READ may evaluate to false even though the buffer is not full */
#define BUF_READ(x) ((x).len == 0)
/* BUF_WRITE evaluates to true whenever there is some data to write */
#define BUF_WRITE(x) ((x).len != 0)

struct buffer {
    size_t len;
    char buf[BUF_SIZE];
    char *pos;
    struct iovec iov[2];
};

void buf_init(struct buffer *buf);
size_t buf_append(struct buffer *buf, const char *read_buf, size_t len);
size_t buf_appendv(struct buffer *buf, const struct iovec *vector, size_t count);
ssize_t buf_read(int fd, struct buffer *buf, size_t len);
ssize_t buf_write(int fd, struct buffer *buf, size_t len);

#endif
