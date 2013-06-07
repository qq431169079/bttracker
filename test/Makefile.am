SRC_DIR = ../src

AM_CFLAGS  = -g -Wall -O3 $(PTHREAD_CFLAGS) $(GLIB_CFLAGS) -I$(SRC_DIR) -include allheads.h
LIBS = $(SRC_DIR)/libbttracker.a $(PTHREAD_LIBS) $(GLIB_LIBS)

TESTS = ttl_tests

# Library used by the unit tests.
check_PROGRAMS = $(TESTS)

ttl_tests_SOURCES = ttl_tests.c test_runner.c
