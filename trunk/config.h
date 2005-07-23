/* $Id: config.h 36 2005-07-22 09:26:47Z matled $ {{{
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
#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_LISTEN_ADDR INADDR_ANY
#define DEFAULT_LISTEN_PORT 1234
#define DEFAULT_SOCKS_ADDR 0x7f000001u /* localhost */
#define DEFAULT_SOCKS_PORT 1080

#define MULTI_ACCEPT 0
/* BUF_SIZE must hold at least one socks request (with username/ip) */
#define BUF_SIZE 2048

#endif
