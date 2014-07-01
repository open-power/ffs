SUBDIRS = clib fcp ecc ffs fpart

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

fpart:: ffs clib
fcp:: ffs clib
ecc:: clib

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

all install: $(SUBDIRS)

clean distclean: $(SUBDIRS)
