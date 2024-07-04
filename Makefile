CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
SOURCES = src/main.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = chess_engine

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)