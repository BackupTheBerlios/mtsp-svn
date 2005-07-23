/* $Id: mtsp.h 46 2005-07-23 17:30:22Z matled $ {{{
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
#ifndef MAIN_H
#define MAIN_H
#include <netinet/in.h>
#include <stdint.h>
#include <sys/select.h>
#include "options.h"

struct socks_request {
    int8_t version;
    int8_t command;
    in_port_t dstport;
    in_addr_t dstip;
};

struct socks_reply {
    int8_t version;
    int8_t result;
    in_port_t dstport;
    in_addr_t dstip;
};

struct global_t {
    char *argv0;
    struct connection_t *connections;
    struct connection_t *pending;
    struct options_t options;
};

int setup_listen(struct sockaddr_in addr);

void main_loop(int listenfd);

void process_pending(fd_set *rfds, fd_set *wfds);

void process_connections(fd_set *rfds, fd_set *wfds);

void fill_fdset(fd_set *rfds, fd_set *wfds, int *maxfd);

void accept_connections(int listenfd);

void setup_connection(int fd);

int get_dst(int s, struct sockaddr_in *addr);

#endif
