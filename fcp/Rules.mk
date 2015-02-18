# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: fcp/Rules.mk $
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
CFLAGS += -D_GNU_SOURCE -std=gnu99 -D_FILE_OFFSET_BITS=64
CFLAGS += -I$(DEPTH)/.. -iquote..

LDFLAGS += -L.

NAME=fcp

CLIB=$(DEPTH)/../clib/x86/libclib.a
FFS=$(DEPTH)/../ffs/x86/libffs.a

OBJS=cmd_list.o cmd_read.o cmd_write.o cmd_erase.o	\
     cmd_copy.o cmd_trunc.o cmd_user.o misc.o main.o

TARGETS=$(NAME)

vpath	%.c ../src
vpath	%.h ..

all: $(TARGETS)

$(NAME): $(OBJS) $(FFS) $(CLIB)
	$(CC) $(LDFLAGS) -Wl,-no-whole-archive \
		-o $@ $^ -lpthread -ldl -lrt

install: $(TARGETS)
	$(INSTALL) fcp $(FCP_INSTALL)/bin
	$(INSTALL) ../fcp.sh $(FCP_INSTALL_TEST)

clean distclean:
	$(RM) -f $(TARGETS) $(OBJS)
