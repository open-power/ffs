# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: clib/Rules.cunit.mk $
#
# OpenPOWER FFS Project
#
# Contributors Listed Below - COPYRIGHT 2014,2015
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG
CFLAGS += -m32 -std=gnu99 -D__USE_GNU -I$(DEPTH)/apps
LDFLAGS = -rdynamic -L$(DEPTH)/apps/clib/$(ARCH_DEP_DIR)

TARGETS=clib

OBJS=slab.o splay.o map.o vector.o ecc.o tree.o clib.o

vpath %.h ../../src
vpath %.c ../../cunit

all: $(TARGETS)

clib: $(OBJS) $(DEPTH)/apps/clib/$(ARCH_DEP_DIR)/libclib.a
	$(CC) $(LDFLAGS) -o $@ $^ -lcunit -lrt

clean:
	$(RM) $(OBJS) $(TARGETS)

install:
	$(INSTALL) -d $(CLIB_INSTALL)
	$(INSTALL) $(TARGETS) $(CLIB_INSTALL)
