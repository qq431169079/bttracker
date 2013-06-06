AM_CFLAGS = -g -Wall -O3 $(PTHREAD_CFLAGS) $(GLIB_CFLAGS) -include allheads.h
LIBS = $(PTHREAD_LIBS) $(GLIB_LIBS)

bin_PROGRAMS = bttracker
bttracker_SOURCES = random.c ttl.c net.c connect.c bttracker.c

# Library used by the unit tests.
noinst_LIBRARIES = libbttracker.a
libbttracker_adir = $(includedir)/bttracker
libbttracker_a_SOURCES = $(bttracker_SOURCES)
libbttracker_a_HEADERS = random.h ttl.h net.h connect.h
