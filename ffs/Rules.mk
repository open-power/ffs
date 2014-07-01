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
