# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: fpart/Rules.mk $
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
CFLAGS += -m32 -D_GNU_SOURCE -std=gnu99 -D_FILE_OFFSET_BITS=64
CFLAGS += -I$(DEPTH)/.. -iquote..

LDFLAGS += -L. -m32

NAME=fpart

CLIB=$(DEPTH)/../clib/x86/libclib.a
FFS=$(DEPTH)/../ffs/x86/libffs.a

OBJS=cmd_create.o cmd_add.o cmd_delete.o cmd_list.o	\
     cmd_erase.o cmd_trunc.o cmd_user.o command.o main.o

TARGETS=$(NAME)

vpath	%.c ../src
vpath	%.h ..

all: $(TARGETS)

$(NAME): $(OBJS) $(FFS) $(DBGX) $(SPINOR) $(CLIB)
	$(CC) $(LDFLAGS) -Wl,-whole-archive $(DBGX) -Wl,-no-whole-archive \
		-o $@ $^ -lpthread -ldl -lrt

install: $(TARGETS)
	$(INSTALL) fpart $(FPART_INSTALL)/bin
	$(INSTALL) ../fpart.sh $(FPART_INSTALL_TEST)

clean distclean:
	$(RM) -f $(TARGETS) $(OBJS)
