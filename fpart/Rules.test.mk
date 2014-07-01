CFLAGS += -m32 -D_GNU_SOURCE -std=gnu99 -D_FILE_OFFSET_BITS=64 -I$(DEPTH)/apps -iquote..
LDFLAGS += -L. -L$(DEPTH)/apps/clib/$(ARCH_DEP_DIR)

OBJS=test_libffs.o

TARGETS=test_libffs

vpath	%.c ../test
vpath	%.h ..

all: $(TARGETS)

#libffs.a: $(OBJS)
#	$(AR) -r $@ $^

test_libffs: test_libffs.o ../ppc/libffs.a $(DEPTH)/apps/clib/$(ARCH_DEP_DIR)/libclib.a
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread

install: $(TARGETS)
	$(INSTALL) ../test/test_libffs $(TEST_LIBFFS_INSTALL)
	$(INSTALL) ../test/libffs_test.sh $(TEST_LIBFFS_INSTALL)
	$(INSTALL) ../test/ffs_tool_test.sh $(TEST_LIBFFS_INSTALL)

clean distclean:
	$(RM) -f $(TARGETS) $(OBJS)
