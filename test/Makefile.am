SRC_DIR = ../src

AM_CFLAGS  = -g -Wall -O3 $(PTHREAD_CFLAGS) $(GLIB_CFLAGS) -I$(SRC_DIR) -include allheads.h
LIBS = $(SRC_DIR)/libbttracker.a $(PTHREAD_LIBS) $(GLIB_LIBS)

TESTS = byteorder_tests

check_PROGRAMS = $(TESTS)

byteorder_tests_SOURCES = byteorder_tests.c test_runner.c
