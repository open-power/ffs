# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: clib/Rules.test.mk $
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

TARGETS=exception	\
	slab		\
	list		\
	ecc		\
	tree		\
	splay		\
	map		\
	table		\
	mq		\
	vector		\
	checksum	\
	err

vpath %.h ../../src
vpath %.c ../../test

all: $(TARGETS)

exception: exception.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

slab:  slab.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

list:  list.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

tree:  tree.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

splay:  splay.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

map:  map.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

mq:  mq.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

table:  table.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

watch:  watch.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

dispatch:  dispatch.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

vector:  vector.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

checksum:  checksum.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

err:  err.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

ecc:  ecc.o ../libclib.so
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(TARGETS:=.o) $(TARGETS)

install:
	$(INSTALL) -d $(CLIB_INSTALL)
	$(INSTALL) $(TARGETS) $(CLIB_INSTALL)
