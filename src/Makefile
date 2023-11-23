PROGS = nopfrog.so

CC = gcc

CLEANFILES = $(PROGS) *.o *.d

SRCDIR ?= ./

NO_MAN=
CFLAGS = -O3 -pipe
CFLAGS += -Werror -Wall -Wunused-function
CFLAGS += -Wextra
CFLAGS += -shared -fPIC

C_SRCS = rk.c
OBJS = $(C_SRCS:.c=.o)

.PHONY: all
all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-@rm -rf $(CLEANFILES)
