PROG = cutsreader
SRCS = cutsreader.c
OBJS = $(SRCS:.c=.o)

CC = gcc
CFLAGS = -O2 -Wall -Wextra

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROG)
