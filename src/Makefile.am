AM_CFLAGS = -g -Wall -O3 $(GLIB_CFLAGS) -include allheads.h
LIBS = $(GLIB_LIBS) -lhiredis

MAIN = bttracker.c
SRC = random.c net.c data.c error.c connect.c handshake.c announce.c pool.c

bin_PROGRAMS = bttracker
bttracker_SOURCES = $(SRC) $(MAIN)

# Library used by the unit tests.
noinst_LIBRARIES = libbttracker.a
libbttracker_adir = $(includedir)/bttracker
libbttracker_a_SOURCES = $(SRC)
libbttracker_a_HEADERS = byteorder.h random.h net.h data.h error.h connect.h handshake.h announce.h pool.h
