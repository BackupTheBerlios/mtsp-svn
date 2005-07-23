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
