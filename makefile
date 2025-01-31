CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared -ldl -lpam

TARGET = lib.so

SRCS = lib.c ./socket/client/client.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

rebuild: clean all

.PHONY: all clean rebuild
