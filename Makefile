CC=gcc
SRC=./src
INC=$(SRC)/include
TARG=./target
CFLAGS=-Werror -Wall -g -pedantic

.PHONY: clean run build


# these should be collapsed into more abstract rules
# something with patsubst would cut down on the INC + object boilerplate
$(TARG)/store.o: $(SRC)/store.c
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

$(TARG)/sock.o: $(SRC)/sock.c
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

$(TARG)/handler.o: $(SRC)/handler.c 
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

$(TARG)/app.o: $(SRC)/app.c 
	$(CC) $(CFLAGS) -c -o $@ $^ -I $(INC)

build: $(TARG)/app.o $(TARG)/sock.o $(TARG)/handler.o $(TARG)/store.o
	$(CC) $(CFLAGS) -o app $^ -I $(INC)

clean:
	rm -rf $(TARG)/*

run: build 
	./app
