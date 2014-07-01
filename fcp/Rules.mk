CFLAGS += -m32 -D_GNU_SOURCE -std=gnu99 -D_FILE_OFFSET_BITS=64
CFLAGS += -I$(DEPTH)/.. -iquote..

LDFLAGS += -L. -m32

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
