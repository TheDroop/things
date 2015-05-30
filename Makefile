CC=clang
FLAGS=-std=c11 -Wall

all:
	$(CC) ovm.c $(FLAGS) -o ovm

clean:
	rm -f ovm
