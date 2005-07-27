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
/* includes {{{ */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#ifdef PROFILING
#include <signal.h>
#endif
#if defined(IPTABLES) && defined(__linux__)
#include <limits.h>
#include <linux/netfilter_ipv4.h>
#endif
#include "config.h"
#include "options.h"
#include "mtsp.h"
#include "connection.h"
#include "log.h"
#include "ewrite.h"
/* }}} */

struct global_t global;

#ifdef PROFILING
static void profiling_exit(int n)
/* {{{ */ {
    exit(EXIT_SUCCESS);
} /* }}} */
#endif

int main(int argc, char **argv)
/* {{{ */ {
    int fd;
#ifdef PROFILING
    signal(SIGINT, profiling_exit);
#endif
    global.argv0 = argv[0];
    global.connections = NULL;
    global.pending = NULL;

    parse_opt(argc, argv, &global.options);
    fd = setup_listen(global.options.listen);
    if (fd == -1) {
        exit(EXIT_FAILURE);
    }

    close(0);
    if (global.options.logfile != NULL || global.options.loglevel == 0) {
        close(1);
    }
    close(2);
    
    main_loop(fd);
    return 0;
} /* }}} */

int setup_listen(struct sockaddr_in listenaddr)
/* {{{ */ {
    int listenfd;
    int tmp = 1;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        ewrite(STDERR_FILENO, global.argv0, ": socket() failed for listening ",
            "socket: ", strerror(errno), "\n", NULL);
        close(listenfd);
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) == -1) {
        ewrite(STDERR_FILENO, global.argv0, ": setsockopt() failed for ",
            "listening socket: ", strerror(errno), "\n", NULL);
        close(listenfd);
        return -1;
    }

    if (bind(listenfd, (struct sockaddr*)&listenaddr, sizeof(listenaddr)) == -1) {
        ewrite(STDERR_FILENO, global.argv0, ": bind() failed for listening ",
            "socket: ", strerror(errno), "\n", NULL);
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, SOMAXCONN) == -1) {
        ewrite(STDERR_FILENO, global.argv0, ": listen() failed for listening "
            "socket: ", strerror(errno), "\n", NULL);
        close(listenfd);
        return -1;
    }

    if (fcntl(listenfd, F_SETFL, O_NONBLOCK) == -1) {
        ewrite(STDERR_FILENO, global.argv0, ": fcntl() failed for listening ",
            "socket: ", strerror(errno), "\n", NULL);
        close(listenfd);
        return -1;
    }

    return listenfd;
} /* }}} */

void main_loop(int listenfd)
/* {{{ */ {
    int retval, maxfd;
    fd_set rfds, wfds;

    while (1) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        maxfd = -1;
        if (listenfd != -1) {
            FD_SET(listenfd, &rfds);
            maxfd = listenfd;
        }
        fill_fdset(&rfds, &wfds, &maxfd);

        if ((retval = select(maxfd+1, &rfds, &wfds, NULL, NULL)) == -1) {
            if (errno == EINTR) {
                log_debug("select interrupted", NULL);
                continue;
            }
            log_error("select() failed: ", strerror(errno), NULL);
            exit(EXIT_FAILURE);
        }
        log_debug("select: ", itoa(retval), NULL);

        if (FD_ISSET(listenfd, &rfds)) {
            accept_connections(listenfd);
        }

        process_connections(&rfds, &wfds);
        process_pending(&rfds, &wfds);
    }
} /* }}} */

void process_pending(fd_set *rfds, fd_set *wfds)
/* {{{ */ {
    struct connection_t *i;
    struct connection_t *tmp;
    ssize_t n;

    i = global.pending;
    while (i != NULL) {
        /* send socks connection request */
        if (FD_ISSET(i->socks, wfds)) {
            assert(BUF_WRITE(i->cs_buf));
            n = buf_write(i->socks, &i->cs_buf, 0);
            if (n <= 0) {
                logc_warning(i->client, "DROPPING connection: "
                    "could not send request to socks", NULL);
                tmp = i;
                i = i->next;
                connection_destroy(&global.pending, tmp);
                continue;
            }
#ifdef DEBUG /* {{{ */
            if (!BUF_WRITE(i->cs_buf)) {
                logc_debug(i->client, "sent socks request", NULL);
            } else {
                logc_debug(i->client, "sent partial socks request", NULL);
            }
#endif /* }}} */
        }

        /* recieve reply */
        if (FD_ISSET(i->socks, rfds)) {
            assert(!BUF_WRITE(i->cs_buf));
            assert(sizeof(struct socks_reply) > i->sc_buf.len);
            n = buf_read(i->socks, &i->sc_buf,
                sizeof(struct socks_reply) - i->sc_buf.len);
            if (n <= 0) {
                logc_notice(i->client, "DROPPING connection: "
                    "socks server did not reply", NULL);
                tmp = i;
                i = i->next;
                connection_destroy(&global.pending, tmp);
                continue;
            }
            /* recieved complete reply from socks server */
            if (i->sc_buf.len == sizeof(struct socks_reply)) {
                if (((struct socks_reply*)(i->sc_buf.buf))->result != 90) {
                    logc_warning(i->client, "DROPPING connection: "
                        "socks server denied connection: ",
                        itoa(((struct socks_reply*)(i->sc_buf.buf))->result),
                        NULL);
                    tmp = i;
                    i = i->next;
                    connection_destroy(&global.pending, tmp);
                    continue;
                }
                logc_notice(i->client,
                    "connection to socks server established", NULL);
                buf_init(&i->sc_buf);

                i->state = STATE_CS | STATE_SC;

                tmp = i;
                i = i->next;
                connection_remove(&global.pending, tmp);
                connection_add(&global.connections, tmp);
                continue;
            }
        }

        i = i->next;
    }
} /* }}} */

void process_connections(fd_set *rfds, fd_set *wfds)
/* {{{ */ {
    struct connection_t *i;
    struct connection_t *tmp;
    ssize_t n;

    i = global.connections;
    while (i != NULL) {
        /* CLIENT->socks */
        if (FD_ISSET(i->client, rfds)) {
            assert(i->state & STATE_CS);
            assert(BUF_READ(i->cs_buf));
            n = buf_read(i->client, &i->cs_buf, 0);
            if (n <= 0) {
                i->state &= ~STATE_Cs;
            }
#ifdef DEBUG /* {{{ */
            if (n == 0) {
                logc_debug(i->client, "read from client failed", NULL);
            } else if (n == -1) {
                logc_debug(i->client, "read from client failed: ",
                    strerror(errno), NULL);
            } else {
                logc_debug(i->client, "Cs ", itoa(n), " bytes", NULL);
            }
#endif /* }}} */
        }

        /* SOCKS->client */
        if (FD_ISSET(i->socks, rfds)) {
            assert(i->state & STATE_SC);
            assert(BUF_READ(i->sc_buf));
            n = buf_read(i->socks, &i->sc_buf, 0);
            if (n <= 0) {
                i->state &= ~STATE_Sc;
            }
#ifdef DEBUG /* {{{ */
            if (n == 0) {
                logc_debug(i->client, "read from socks failed", NULL);
            } else if (n == -1) {
                logc_debug(i->client, "read from socks failed: ",
                    strerror(errno), NULL);
            } else {
                logc_debug(i->client, "Sc ", itoa(n) ," bytes", NULL);
            }
#endif /* }}} */
        }

        /* client->SOCKS */
        if (FD_ISSET(i->socks, wfds)) {
            assert(i->state & STATE_cS);
            assert(BUF_WRITE(i->cs_buf));
            n = buf_write(i->socks, &i->cs_buf, 0);
            if (n <= 0) {
                i->state &= ~STATE_CS;
                shutdown(i->client, SHUT_RD);
                shutdown(i->client, SHUT_WR);
                buf_init(&i->cs_buf);
            }
#ifdef DEBUG /* {{{ */
            if (n == 0) {
                logc_debug(i->client, "write to socks failed", NULL);
            } else if (n == -1) {
                logc_debug(i->client, "write to socks failed: ", strerror(errno), NULL);
            } else {
                logc_debug(i->client, "cS ", itoa(n), " bytes", NULL);
            }
#endif /* }}} */
        }

        /* socks->CLIENT */
        if (FD_ISSET(i->client, wfds)) {
            assert(i->state & STATE_sC);
            assert(BUF_WRITE(i->sc_buf));
            n = buf_write(i->client, &i->sc_buf, 0);
            if (n <= 0) {
                if (i->state & STATE_Sc) {
                    shutdown(i->socks, SHUT_RD);
                }
                shutdown(i->socks, SHUT_WR);
                i->state &= ~STATE_SC;
                buf_init(&i->sc_buf);
            }
#ifdef DEBUG /* {{{ */
            if (n == 0) {
                logc_debug(i->client, "write to client failed", NULL);
            } else if (n == -1) {
                logc_debug(i->client, "write to client failed: ",
                    strerror(errno), NULL);
            } else {
                logc_debug(i->client, "sC ", itoa(n), " bytes", NULL);
            }
#endif /* }}} */
        }

        /* no new data will be read and buffer is empty but STATE_cS is still
         * set => disable STATE_cS and shutdown socks connection for writing
         */
        if ((i->state & STATE_CS) == STATE_cS && !BUF_WRITE(i->cs_buf))
        {
            i->state &= ~STATE_cS;
            shutdown(i->socks, SHUT_WR);
        }
        if ((i->state & STATE_SC) == STATE_sC && !BUF_WRITE(i->sc_buf))
        {
            i->state &= ~STATE_sC;
            shutdown(i->client, SHUT_WR);
        }

        if (!(i->state & (STATE_CS | STATE_SC))) {
            logc_notice(i->client, "DROPPING connection", NULL);
            tmp = i;
            i = i->next;
            connection_destroy(&global.connections, tmp);
            continue;
        }

        i = i->next;
    }
} /* }}} */

void fill_fdset(fd_set *rfds, fd_set *wfds, int *maxfd)
/* {{{ */ {
    struct connection_t *i;
    for (i = global.connections; i != NULL; i = i->next) {
        if ((i->state & STATE_CS) && BUF_READ(i->cs_buf)) {
            FD_SET(i->client, rfds);
        }
        if ((i->state & STATE_SC) && BUF_READ(i->sc_buf)) {
            FD_SET(i->socks, rfds);
        }
        if (BUF_WRITE(i->cs_buf)) {
            FD_SET(i->socks, wfds);
        }
        if (BUF_WRITE(i->sc_buf)) {
            FD_SET(i->client, wfds);
        }
        if (i->socks > *maxfd) *maxfd = i->socks;
        if (i->client > *maxfd) *maxfd = i->client;
    }
    for (i = global.pending; i != NULL; i = i->next) {
        if (BUF_WRITE(i->cs_buf)) {
            FD_SET(i->socks, wfds);
        } else {
            FD_SET(i->socks, rfds);
        }
        if (i->socks > *maxfd) *maxfd = i->socks;
    }
    
} /* }}} */

void accept_connections(int listenfd)
/* {{{ */ {
    int newfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
#if MULTI_ACCEPT == 1
    while ((newfd = accept(listenfd, (struct sockaddr*)&addr, &addrlen)) != -1) {
#else
    if ((newfd = accept(listenfd, (struct sockaddr*)&addr, &addrlen)) != -1) {
#endif
        logc_notice(newfd, "NEW connection from ", inet_ntoa(addr.sin_addr),
            ":", itoa(ntohs(addr.sin_port)), NULL);
        setup_connection(newfd);
#if MULTI_ACCEPT == 1
    }
#else
    } else
#endif
    {
        if (errno == EBADF || errno == ENOTSOCK || errno == EINVAL ||
            errno == EOPNOTSUPP || errno == EFAULT)
        {
            log_error("accept failed, stopping to listen: ", strerror(errno), NULL);
            exit(EXIT_FAILURE);
        } else if (errno == EMFILE || errno == ENFILE ||
            errno == ENOBUFS || errno == ENOMEM)
        {
            log_error("accept failed: ", strerror(errno), NULL);
        }
    }
} /* }}} */

void setup_connection(int fd)
/* {{{ */ {
    struct sockaddr_in addr;
    struct socks_request request;
    struct connection_t *con;
    struct iovec iov[3];

    if (get_dst(fd, &addr) == -1) {
        logc_error(fd, "DROPPING connection: get_dst() failed: ",
            strerror(errno), NULL);
        close(fd);
        return;
    }
    logc_notice(fd, "to ", inet_ntoa(addr.sin_addr), ":", itoa(ntohs(addr.sin_port)), NULL);

    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        logc_error(fd, "DROPPING connection: fcntl() failed: ",
            strerror(errno), NULL);
        close(fd);
        return;
    }

    if ((con = connection_alloc()) == NULL) {
        logc_error(fd, "DROPPING connection: connection_alloc() failed: ",
            strerror(errno), NULL);
        close(fd);
        return;
    }
    con->client = fd;

    if ((con->socks = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        logc_error(fd, "DROPPING connection: socket() failed: ",
            strerror(errno), NULL);
        connection_destroy(NULL, con);
        return;
    }

    if (fcntl(con->socks, F_SETFL, O_NONBLOCK) == -1) {
        logc_error(fd, "DROPPING connection: fcntl() failed: ",
            strerror(errno), NULL);
        connection_destroy(NULL, con);
        return;
    }

    request.version = 4;
    request.command = 1;
    request.dstport = addr.sin_port;
    if (global.options.socks4a) {
        request.dstip = htonl(1u);
        iov[2].iov_base = inet_ntoa(addr.sin_addr);
        iov[2].iov_len = strlen(iov[2].iov_base)+1;
    } else {
        iov[2].iov_base = NULL;
        iov[2].iov_len = 0;
        request.dstip = addr.sin_addr.s_addr;
    }
    iov[0].iov_base = &request;
    iov[0].iov_len = sizeof(request);

    iov[1].iov_base = global.options.username;
    iov[1].iov_len = strlen(global.options.username)+1;

    if (buf_appendv(&con->cs_buf, iov, 3) !=
        iov[0].iov_len+iov[1].iov_len+iov[2].iov_len)
    {
        logc_error(fd, "DROPPING connection: buffer failed (too small?)", NULL);
        connection_destroy(NULL, con);
        return;
    }

    if (connect(con->socks, (struct sockaddr*)&global.options.socks,
            sizeof(struct sockaddr_in)) == -1)
    {
        if (errno != EINPROGRESS) {
            logc_error(fd, "DROPPING connection: connect() failed: ",
                strerror(errno), NULL);
            connection_destroy(NULL, con);
            return;
        }
    }

    connection_add(&global.pending, con);
} /* }}} */

int get_dst(int s, struct sockaddr_in *addr)
/* {{{ */ {
    socklen_t addrlen = sizeof(*addr);
#if defined(IPTABLES) && defined(__linux__)
    return getsockopt(s, SOL_IP, SO_ORIGINAL_DST, addr, &addrlen);
#else
    #error I do not know how to get the destination IP/port on this system.
#endif
} /* }}} */
