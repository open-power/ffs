CFLAGS += -m32 -D_GNU_SOURCE -std=gnu99 -D_FILE_OFFSET_BITS=64 -I$(DEPTH)/.. -iquote..
LDFLAGS += -L. -L$(DEPTH)/../clib/x86 -m32

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
