# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: ffs/Rules.mk $
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
CFLAGS += -I$(DEPTH)/.. -I$(DEPTH)/boot/fsp2_ipl -iquote..

LDFLAGS += -L. -m32

NAME=libffs

CLIB=$(DEPTH)/../clib/x86/libclib.a

OBJS=$(NAME).o $(NAME)2.o

TARGETS=$(NAME).so $(NAME).a

vpath	%.c ../src
vpath	%.h ..

all: $(TARGETS)

$(NAME).so: $(OBJS)
	$(CC) $(LDFLAGS) -fPIC -shared -Wl,-soname,$@ -o $@ $^

$(NAME).a: $(OBJS)
	$(AR) -r $@ $^

install: $(TARGETS)
	$(INSTALL) libffs.so libffs.a $(FFS_INSTALL)/lib
	$(INSTALL) ../ffs.h ../libffs.h ../libffs2.h $(FFS_INSTALL)/include

clean distclean:
	$(RM) -f $(TARGETS) $(OBJS)
