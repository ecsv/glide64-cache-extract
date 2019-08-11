#!/usr/bin/make -f
# SPDX-License-Identifier: GPL-3.0-or-later
# -*- makefile -*-
#
# SPDX-FileCopyrightText: 2013, Sven Eckelmann <sven@narfation.org>

BINARY_NAME = glide64_cache_extract
OBJ = glide64_cache_extract.o input_config.o input_file.o convert_file.o output_file.o

# flags and options
CFLAGS += -pedantic -Wall -W -std=gnu99 -MD
CPPFLAGS += -D_FILE_OFFSET_BITS=64

# disable verbose output
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC = @echo '   ' CC $@;
	Q_LD = @echo '   ' LD $@;
	export Q_CC
	export Q_LD
endif
endif

PKG_CONFIG = $(CROSS_COMPILE)pkg-config

ifeq ($(shell which $(PKG_CONFIG) 2>/dev/null),)
  $(error $(PKG_CONFIG) not found)
endif
ifeq ($(shell $(PKG_CONFIG) --modversion zlib 2>/dev/null),)
  $(error No zlib development libraries found!)
endif
CFLAGS += $(shell $(PKG_CONFIG) --cflags zlib)
LDLIBS +=  $(shell $(PKG_CONFIG) --libs zlib)


CC = $(CROSS_COMPILE)gcc
RM ?= rm -f
INSTALL ?= install
MKDIR ?= mkdir -p
COMPILE.c = $(Q_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.o = $(Q_LD)$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_ARCH)

# standard install paths
PREFIX = /usr/local
BINDIR = $(PREFIX)/sbin
MANDIR = $(PREFIX)/share/man

# default target
all: $(BINARY_NAME)

# standard build rules
.SUFFIXES: .o .c
.c.o:
	$(COMPILE.c) -o $@ $<

$(BINARY_NAME): $(OBJ)
	$(LINK.o) $^ $(LDLIBS) -o $@

clean:
	$(RM) $(BINARY_NAME) $(OBJ) $(DEP)

install: $(BINARY_NAME)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(BINARY_NAME) $(DESTDIR)$(BINDIR)

# load dependencies
DEP = $(OBJ:.o=.d)
-include $(DEP)

.PHONY: all clean install
