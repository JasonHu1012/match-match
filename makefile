CC = gcc
INCLUDE = include
SRC = src
LIB = lib
CFLAGS = -I$(INCLUDE)

server: server.c $(LIB)/game.o $(LIB)/func.o
	$(CC) server.c -o server $(LIB)/game.o $(LIB)/func.o $(CFLAGS)

$(LIB)/game.o: $(SRC)/game.c $(INCLUDE)/game.h
	@mkdir -p $(LIB)
	$(CC) -c $(SRC)/game.c -o $(LIB)/game.o $(CFLAGS)

$(LIB)/func.o: $(SRC)/func.c $(INCLUDE)/func.h
	@mkdir -p $(LIB)
	$(CC) -c $(SRC)/func.c -o $(LIB)/func.o $(CFLAGS)
clean:
	rm -rf lib
	rm -rf server
