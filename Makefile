CC ?= gcc
X = main
all:
	$(CC) -o $(X) $(X).c
	$(CC) -o $(X)_pr $(X)_pr.c
	#$(CC) -o $(X)_no_pr $(X)_no_pr.c
debug:
	./$(X) addresses.txt
	./$(X)_pr addresses.txt
clean:
	rm -v $(X)_pr $(X)
