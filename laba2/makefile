CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic

RELEASE = ./build/release/

PARENT = parent
CHILD = child

PARENT_SRC = parent.c
CHILD_SRC = child.c

# Цели
all: $(PARENT) $(CHILD)

$(PARENT): $(PARENT_SRC)
	$(CC) $(CFLAGS) -o $(RELEASE)$(PARENT) $(PARENT_SRC)

$(CHILD): $(CHILD_SRC)
	$(CC) $(CFLAGS) -o $(CHILD) $(CHILD_SRC)

clean:
	rm -f $(RELEASE)$(PARENT) $(RELEASE)$(CHILD)

.PHONY: all clean
