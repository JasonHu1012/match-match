CC = gcc
INCLUDE = include
SRC = src
LIB = lib
CFLAGS = -I$(INCLUDE)

all: server draw
	@echo "finish compiling"

server: $(SRC)/server.c $(LIB)/game.o $(LIB)/func.o
	$(CC) $(SRC)/server.c -o server $(LIB)/game.o $(LIB)/func.o $(CFLAGS)

draw: $(SRC)/draw.c $(LIB)/func.o
	$(CC) $(SRC)/draw.c -o draw $(LIB)/func.o $(CFLAGS)

$(LIB)/game.o: $(SRC)/game.c $(INCLUDE)/game.h
	@mkdir -p $(LIB)
	$(CC) -c $(SRC)/game.c -o $(LIB)/game.o $(CFLAGS)

$(LIB)/func.o: $(SRC)/func.c $(INCLUDE)/func.h
	@mkdir -p $(LIB)
	$(CC) -c $(SRC)/func.c -o $(LIB)/func.o $(CFLAGS)
clean:
	rm -rf lib
	rm -rf server
	rm -rf draw
