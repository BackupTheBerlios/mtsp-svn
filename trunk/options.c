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
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "options.h"
#include "config.h"
#include "log.h"
#include "ewrite.h"

void usage(const char *progname)
/* {{{ */ {
    struct in_addr addr;
    ewrite(STDOUT_FILENO,
        "Usage: ", progname, " [OPTION]...\n"
        "Transparent socks proxy.\n"
        "\n", NULL);

    addr.s_addr = htonl(DEFAULT_LISTEN_ADDR);
    ewrite(STDOUT_FILENO,
        "  -l, --listen-port   listening port, default: ", itoa(DEFAULT_LISTEN_PORT), "\n",
        "  -L, --listen-addr   listening address, default: ", inet_ntoa(addr), "\n",
        NULL);

    addr.s_addr = htonl(DEFAULT_SOCKS_ADDR);
    ewrite(STDOUT_FILENO,
        "  -s, --socks-port    socks port, default: ", itoa(DEFAULT_SOCKS_PORT), "\n",
        "  -S, --socks-addr    socks address, default: ", inet_ntoa(addr), "\n"
        "  -a, --socks4a       use socks4a\n"
        "  -o, --output        write information to file\n"
        "  -q, --quiet         no log output\n"
        "  -h, --help          display this help and exit\n",
        NULL);
} /* }}} */

void parse_opt(int argc, char * const argv[], struct options_t *options)
/* {{{ */ {
    int c, error = 0;
    struct hostent *addr;
    struct passwd *pwd;
    const struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"listen-addr", required_argument, 0, 'L'},
        {"listen-port", required_argument, 0, 'l'},
        {"socks-addr", required_argument, 0, 'S'},
        {"socks-port", required_argument, 0, 's'},
        {"socks4a", no_argument, 0, 'a'},
        {"output", required_argument, 0, 'o'},
        {"quiet", no_argument, 0, 'q'},
        {0, 0, 0, 0}
    };

    /* initialize default values */
    options->socks.sin_family = AF_INET;
    options->socks.sin_port = htons(DEFAULT_SOCKS_PORT);
    options->socks.sin_addr.s_addr = htonl(DEFAULT_SOCKS_ADDR);
    bzero(&(options->socks.sin_zero),
        sizeof(options->socks.sin_zero));

    options->listen.sin_family = AF_INET;
    options->listen.sin_port = htons(DEFAULT_LISTEN_PORT);
    options->listen.sin_addr.s_addr = htonl(DEFAULT_LISTEN_ADDR);
    bzero(&(options->listen.sin_zero),
        sizeof(options->listen.sin_zero));

    options->logfile = NULL;
    options->loglevel = 1;
    options->socks4a = 0;

    if ((options->username = getenv("SOCKS_USERNAME")) == NULL) {
        errno = 0;
        if ((pwd = getpwuid(getuid())) == NULL) {
            ewrite(STDERR_FILENO, argv[0], ": getpwuid failed: ",
                strerror(errno), "\n", NULL);
            exit(EXIT_FAILURE);
        }
        options->username = pwd->pw_name;
    }

    /* evaluate arguments */
    while ((c = getopt_long(argc, argv, "hL:l:S:s:ao:q", longopts, 0)) != -1) {
    switch (c) {
    case 'h':
        usage(argv[0]);
        exit(EXIT_SUCCESS);
    case 'l':
        options->listen.sin_port = htons(atoi(optarg));
        if (options->listen.sin_port == 0) {
            error = 1;
            ewrite(STDERR_FILENO, argv[0], ": invalid listen port: ",
                optarg, "\n", NULL);
        }
        break;
    case 'L':
        if ((addr = gethostbyname(optarg)) == NULL) {
            error = 1;
            ewrite(STDERR_FILENO, argv[0], ": invalid listen address: ",
                optarg, "\n", NULL);
            break;
        }
        if (addr->h_addrtype != AF_INET) {
            error = 1;
            ewrite(STDERR_FILENO, argv[0], ": invalid listen address (ipv4 only): ",
                optarg, "\n", NULL);
            break;
        }
        memcpy(&options->listen.sin_addr.s_addr, addr->h_addr_list[0],
            addr->h_length);
        break;
    case 's':
        options->socks.sin_port = htons(atoi(optarg));
        if (options->socks.sin_port == 0) {
            error = 1;
            ewrite(STDERR_FILENO, argv[0], ": invalid socks port: ",
                optarg, "\n", NULL);
        }
        break;
    case 'S':
        if ((addr = gethostbyname(optarg)) == NULL) {
            error = 1;
            ewrite(STDERR_FILENO, argv[0], ": invalid socks address: ",
                optarg, "\n", NULL);
            break;
        }
        if (addr->h_addrtype != AF_INET) {
            error = 1;
            ewrite(STDERR_FILENO, argv[0], ": invalid socks address (ipv4 only): ",
                optarg, "\n", NULL);
            break;
        }
        memcpy(&options->socks.sin_addr.s_addr, addr->h_addr_list[0],
            addr->h_length);
        break;
    case 'a':
        options->socks4a = 1;
        break;
    case 'q':
        options->loglevel = 0;
        break;
    case 'o':
        options->logfile = optarg;
        break;
    case '?':
        break;
    }
    }

    if (error) {
        ewrite(STDERR_FILENO, "Try `", argv[0],
            " --help' for more information.", NULL);
        exit(EXIT_FAILURE);
    }
} /* }}} */
