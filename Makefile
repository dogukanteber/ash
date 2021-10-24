CC = gcc
CFLAGS =
RM = rm -rf
OUTPUT = ash

all: build

build: main.o
	$(CC) $(CFLAGS) -o $(OUTPUT) main.c -lreadline

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	$(RM) *.o $(OUTPUT)