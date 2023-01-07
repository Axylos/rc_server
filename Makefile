CC=gcc
SRC=./src
INC=$(SRC)/include
TARG=./target
CFLAGS=-Werror -Wall -g

.PHONY: clean run build

$(TARG)/app.o: $(SRC)/app.c
	gcc -c -o $@ $^

build: $(TARG)/app.o
	gcc -o app $^

clean:
	rm -rf $(TARG)/*

run: build 
	./app
