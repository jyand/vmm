CC ?= gcc
X = main
all:
	$(CC) -o $(X) $(X).c
	$(CC) -o $(X)_pr $(X)_pr.c
debug:
	./$(X) addresses.txt
clean:
	rm -v $(X)_pr $(X)
