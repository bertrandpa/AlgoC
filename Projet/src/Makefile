CC ?= gcc

COBJS ?= client.o bmp.o couleur.o json.o
SOBJS ?= serveur.o json.o
TOBJS ?= tests.o json.o

.SUFFIXES: .c .o

SERVER = serveur
CLIENT = client
TESTS	 = tests

all: $(SERVER) $(CLIENT)

$(SERVER): $(SOBJS)
	$(CC) -o $(SERVER) $(SOBJS) -lpthread

$(CLIENT): $(COBJS)
	$(CC) -o $(CLIENT) $(COBJS)

$(TESTS): $(TOBJS)
	$(CC) -o $(TESTS) $(TOBJS) -lpthread

.c.o: 
	$(CC) -c $*.c -Wall -Wextra -g

