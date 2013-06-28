AM_CFLAGS = -g -Wall -O3 $(PTHREAD_CFLAGS) $(GLIB_CFLAGS) -include allheads.h
LIBS = $(PTHREAD_LIBS) $(GLIB_LIBS)

MAIN = bttracker.c
SRC = random.c ttl.c net.c data.c connect.c

bin_PROGRAMS = bttracker
bttracker_SOURCES = $(SRC) $(MAIN)

# Library used by the unit tests.
noinst_LIBRARIES = libbttracker.a
libbttracker_adir = $(includedir)/bttracker
libbttracker_a_SOURCES = $(SRC)
libbttracker_a_HEADERS = byteorder.h random.h ttl.h net.h data.h connect.h
