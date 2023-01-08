CC=gcc
SRC=./src
INC=$(SRC)/include
TARG=./target
CFLAGS=-Werror -Wall -g -pedantic

.PHONY: clean run build

$(TARG)/%.o: $(SRC)/%.c $(INC)
	$(CC) $(CFLAGS) -c -o $@ $< -I $(INC)

# it would be good to extract the obj file names but unsure how to do that reasonably
build: $(TARG)/app.o $(TARG)/sock.o $(TARG)/handler.o $(TARG)/store.o
	$(CC) $(CFLAGS) -o app $^ -I $(INC)

clean:
	rm -rf $(TARG)/*

run: build 
	./app
