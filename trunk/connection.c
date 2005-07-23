/* $Id: connection.c 36 2005-07-22 09:26:47Z matled $ {{{
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
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "config.h"
#include "buffer.h"
#include "log.h"
#include "connection.h"

struct connection_t *connection_alloc(void)
/* {{{ */ {
    struct connection_t *new_connection = malloc(sizeof(struct connection_t));
    if (new_connection != NULL) {
        buf_init(&new_connection->cs_buf);
        buf_init(&new_connection->sc_buf);
    }
    return new_connection;
} /* }}} */

void connection_free(struct connection_t *connection)
/* {{{ */ {
    assert(connection != NULL);
    free(connection);
} /* }}} */

void connection_add(struct connection_t **list, struct connection_t *element)
/* {{{ */ {
    assert(list != NULL);
    assert(element != NULL);
    element->next = *list;
    *list = element;
} /* }}} */

int connection_remove(struct connection_t **list, struct connection_t *element)
/* {{{ */ {
    struct connection_t *i;
    assert(list != NULL);
    assert(element != NULL);
    if (*list == NULL) {
        return -1;
    }
    if (*list == element) {
        *list = (*list)->next;
        return 0;
    }
    /* iterate through list to the last element or i->next == element */
    for (i = *list; i->next != NULL && i->next != element; i = i->next);
    if (i->next == NULL) return -1;
    assert(i->next == element);
    i->next = i->next->next;
    return 0;
} /* }}} */

void connection_destroy(struct connection_t **list, struct connection_t *element)
/* {{{ */ {
    assert(element != NULL);
    close(element->client);
    close(element->socks);
    if (list != NULL && *list != NULL) {
#ifdef NDEBUG
        connection_remove(list, element);
#else
        assert(connection_remove(list, element) != -1);
#endif
    }
    connection_free(element);
} /* }}} */
