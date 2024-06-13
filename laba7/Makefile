CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -pthread

SRCS = main.c
OBJS = $(SRCS:.c=.o)
EXEC = main

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJS)
