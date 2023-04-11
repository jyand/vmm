CC ?= gcc
all:
	$(CC) -o main main.c
debug:
	./main
clean:
	rm -v main
