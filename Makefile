# hsre Makefile
#
# I don't know how to make makefiles.
#

PWD = $(shell pwd)
PWDB = $(shell basename `pwd`)

GCC = gcc
CFLAGS = -g -Wall -pedantic
LDFLAGS = 

OBJS = hsre.o regcomp.o regexec.o

TEST_SRC = test.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_BIN = $(TEST_SRC:.c=)

all: test

archive:
	cd ..; tar -cvzf $(PWDB).tgz $(PWDB)

.c.o:
	$(GCC) $(CFLAGS) -c $<

clean:
	rm -f test $(OBJS) *~

test: $(OBJS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o test test.c

