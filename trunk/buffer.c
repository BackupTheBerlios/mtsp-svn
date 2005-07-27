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
#include <assert.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "buffer.h"

void buf_init(struct buffer *buf)
/* {{{ */ {
    buf->len = 0;
    buf->pos = buf->buf;
} /* }}} */

size_t buf_append(struct buffer *buf, const char *read_buf, size_t len)
/* {{{ */ {
    size_t n = len > BUF_SIZE - buf->len ? BUF_SIZE - buf->len : len;
    memcpy(buf->buf+buf->len, read_buf, n);
    buf->len += n;
    return n;
} /* }}} */

size_t buf_appendv(struct buffer *buf, const struct iovec *vector, size_t count)
/* {{{ */ {
    size_t n = 0, i, len;
    for (i = 0; i < count; ++i) {
        len = vector[i].iov_len > BUF_SIZE - buf->len ?
            BUF_SIZE - buf->len : vector[i].iov_len;
        memcpy(buf->buf+buf->len, vector[i].iov_base, len);
        buf->len += n;
        n += len;
    }
    return n;
} /* }}} */

ssize_t buf_read(int fd, struct buffer *buf, size_t len)
/* {{{ */ {
    ssize_t n;
    assert(BUF_READ(*buf));
    assert(len <= BUF_SIZE - buf->len);

    if (len == 0) len = BUF_SIZE;
    buf->pos = buf->buf;
    n = read(fd, buf->pos, len);
    if (n >= 0) {
        buf->len = n;
    }
    return n;
} /* }}} */

ssize_t buf_write(int fd, struct buffer *buf, size_t len)
/* {{{ */ {
    ssize_t n;
    assert(BUF_WRITE(*buf));
    assert(len <= buf->len);

    if (len == 0) len = buf->len;
    n = write(fd, buf->buf, len);
    if (n > 0) {
        buf->len -= n;
        buf->pos += n;
    }
    return n;
} /* }}} */
