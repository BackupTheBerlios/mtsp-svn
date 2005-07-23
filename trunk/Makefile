# $Id: Makefile 45 2005-07-23 17:27:11Z matled $
TARGET=mtsp
OBJECTS=buffer.o connection.o ewrite.o log.o options.o $(TARGET).o

CC=gcc
LD=gcc
STRIP=strip
RM=rm -f

LDFLAGS+=
CFLAGS+=-DIPTABLES=1

ifeq ($(DEBUG),1)
	#CFLAGS+=-std=c99 -pedantic-errors
	CFLAGS+=-ggdb3

	CFLAGS+=-Wall -W -Wchar-subscripts -Wmissing-prototypes
	CFLAGS+=-Wmissing-declarations -Wredundant-decls
	CFLAGS+=-Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS+=-Winline -Wpointer-arith -Wsign-compare -Werror
	CFLAGS+=-Wunreachable-code -Wdisabled-optimization
	CFLAGS+=-Wcast-align -Wwrite-strings -Wnested-externs -Wundef

	CFLAGS+=-DDEBUG
	LDFLAGS+=
else
	CFLAGS+=-O2 -DNDEBUG
	LDFLAGS+=
endif
ifeq ($(PROFILING),1)
	CFLAGS+=-g -pg -DPROFILING
	LDFLAGS+=-pg
endif

.PHONY: clean all

all: $(OBJECTS) $(TARGET) $(TARGET).stripped
	ls -l mtsp mtsp.stripped

%.o: %.c %.h config.h
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.stripped: %
	$(STRIP) -s -o $@ $^

clean:
	-$(RM) -- $(OBJECTS) $(TARGET) $(TARGET).stripped
