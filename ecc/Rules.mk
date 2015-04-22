# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: ecc/Rules.mk $
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
CFLAGS += -D_GNU_SOURCE -std=gnu99 -D_FILE_OFFSET_BITS=64 -I$(DEPTH)/.. -iquote..
LDFLAGS += -L. -L$(DEPTH)/../clib/x86

OBJS=main.o

TARGETS=ecc

vpath	%.c ../src
vpath	%.h ..

all: $(TARGETS)

ecc: main.o $(DEPTH)/../clib/x86/libclib.a
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread

install: $(TARGETS)
	$(INSTALL) ecc $(ECC_INSTALL)/bin

clean distclean:
	$(RM) -f $(TARGETS) $(OBJS)
