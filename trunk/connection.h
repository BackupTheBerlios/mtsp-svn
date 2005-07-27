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
#ifndef CONNECTION_H
#define CONNECTION_H

#include "config.h"
#include "buffer.h"

struct connection_t {
    struct connection_t *next;
    
    int client;
    struct buffer cs_buf;

    int socks;
    struct buffer sc_buf;
};

struct connection_t *connection_alloc(void);
void connection_free(struct connection_t *connection);

void connection_add(struct connection_t **list, struct connection_t *element);
int connection_remove(struct connection_t **list, struct connection_t *element);

void connection_destroy(struct connection_t **list, struct connection_t *element);

#endif
