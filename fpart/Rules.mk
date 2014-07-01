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
