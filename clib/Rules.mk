CFLAGS += -m32 -D_GNU_SOURCE -std=gnu99 -fshort-enums -D_FILE_OFFSET_BITS=64
CFLAGS += -iquote$(DEPTH)

LDFLAGS=-L. -m32

# array.o array_iter.o dispatch.o watch.o signal.o heap.o

OBJS =	err.o crc32.o misc.o ecc.o	\
	exception.o slab.o		\
	list.o list_iter.o		\
	tree.o tree_iter.o		\
	vector.o vector_iter.o		\
	table.o table_iter.o 		\
	value.o mq.o	 	\
	memory_leak_detection.o		\
	trace_indent.o checksum.o


TARGETS=libclib.so libclib.a crc32

vpath	%.c ../src
vpath	%.h ..

all: $(TARGETS)

libclib.so: $(OBJS)
	$(CC) $(LDFLAGS) -fPIC -shared -Wl,-soname,$@ -o $@ $^ -lpthread \
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
