TARGET = fnv
CC = gcc
CFLAGS = -Wall -Wextra -O2

SRC = fnv.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	@rm -f $(OBJ)

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
