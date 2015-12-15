# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: clib/Rules.mk $
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
CFLAGS += -D_GNU_SOURCE -std=gnu99 -fshort-enums -D_FILE_OFFSET_BITS=64
CFLAGS += -iquote$(DEPTH) -fPIC

LDFLAGS=-L.

# array.o array_iter.o signal.o heap.o

OBJS =	err.o crc32.o misc.o ecc.o	\
	exception.o slab.o		\
	list.o list_iter.o		\
	tree.o tree_iter.o		\
	value.o mq.o	 	\
	memory_leak_detection.o		\
	trace_indent.o checksum.o


TARGETS=libclib.so libclib.a crc32

vpath	%.c ../src
vpath	%.h ..

all: $(TARGETS)

libclib.so: $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$@ -o $@ $^ -lpthread \
		-lrt

libclib.a: $(OBJS)
	$(AR) -r $@ $^

db.o: db.c db.h
	$(CC) $(CFLAGS) -DSQLITE3 -c $^

crc32: crc32.c crc32_main.c
	$(CC) $(CFLAGS) -o $@ $^

clean distclean:
	$(RM) *.so *.o *.gch  *.CKP *.lst $(TARGETS)
	$(RM) html latex

install: $(TARGETS) ../*.h
	$(INSTALL) -d $(CLIB_INSTALL)/include/clib $(CLIB_INSTALL)/{bin,lib}
	$(INSTALL) ../crc32.h $(CLIB_INSTALL)/include
	$(INSTALL) ../*.h $(CLIB_INSTALL)/include/clib
	$(INSTALL) libclib.so libclib.a $(CLIB_INSTALL)/lib
	$(INSTALL) crc32 $(CLIB_INSTALL)/bin
	$(MAKE) -C test $@
	$(MAKE) -C cunit $@
