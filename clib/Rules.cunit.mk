CFLAGS += -m32 -std=gnu99 -D__USE_GNU -I$(DEPTH)/apps
LDFLAGS = -rdynamic -L$(DEPTH)/apps/clib/$(ARCH_DEP_DIR)

TARGETS=clib

OBJS=slab.o splay.o map.o vector.o ecc.o tree.o clib.o

vpath %.h ../../src
vpath %.c ../../cunit

all: $(TARGETS)

clib: $(OBJS) $(DEPTH)/apps/clib/$(ARCH_DEP_DIR)/libclib.a
	$(CC) $(LDFLAGS) -o $@ $^ -lcunit -lrt

clean:
	$(RM) $(OBJS) $(TARGETS)

install:
	$(INSTALL) -d $(CLIB_INSTALL)
	$(INSTALL) $(TARGETS) $(CLIB_INSTALL)
