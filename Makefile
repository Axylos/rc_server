CC=gcc
SRC=./src
INC=$(SRC)/include
TARG=./target
CFLAGS=-Werror -Wall -g

.PHONY: clean run build

$(TARG)/sock.o: $(SRC)/sock.c
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

$(TARG)/handler.o: $(SRC)/handler.c
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

$(TARG)/app.o: $(SRC)/app.c 
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

build: $(TARG)/app.o $(TARG)/sock.o $(TARG)/handler.o
	$(CC) $(CFLAGS) -o app $^ -I $(INC)

clean:
	rm -rf $(TARG)/*

run: build 
	./app
