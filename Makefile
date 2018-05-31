PGLIBDIR     = $(shell pg_config --libdir)
PGINCLUDEDIR = $(shell pg_config --includedir)
# TODO: Add dynamic lookup based on Distro, HW vendor
CLLIBDIR     = /usr/include/CL/
CLINCLUDEDIR = /usr/lib/x86_64-linux-gnu/

CC           = gcc
LIBS         = -lm -lpq -lOpenCL
CFLAGS       = -I./src/ -I./src/lib/ -I$(PGINCLUDEDIR) -g -I$(CLINCLUDEDIR)

pgopencl: src/pgopencl.o src/lib/util.o
	$(CC) -o pgopencl src/pgopencl.o src/lib/util.o -g -L$(PGLIBDIR) -L$(CLLIBDIR) -lm -lpq

EXTENSION   = pgopencl
EXTVERSION  = 0.1
PG_CONFIG   = pg_config
MODULES     = pgopencl
EXTRA_CLEAN = src/lib/*.o src/*.o *.o

PGXS := $(shell $(PG_CONFIG) --pgxs)

include $(PGXS)
