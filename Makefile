CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LIBS = -lreadline

SRCS = src/main.c src/token.c src/lexer.c src/parser.c src/expand.c src/history.c src/builtin.c src/executor.c
OBJS = $(SRCS:.c=.o)
TARGET = shellforge

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
