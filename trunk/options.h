/* $Id: options.h 36 2005-07-22 09:26:47Z matled $ {{{
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
#ifndef OPTIONS_H
#define OPTIONS_H 1

#include <netinet/in.h>

struct options_t {
    struct sockaddr_in socks;
    struct sockaddr_in listen;
    char *username;
    char *logfile;
    unsigned int socks4a : 1;
    unsigned int loglevel : 1;
};

void usage(const char *progname);
void parse_opt(int argc, char * const argv[], struct options_t *options);

#endif
