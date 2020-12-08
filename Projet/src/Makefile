CC ?= gcc

COBJS ?= client.o bmp.o couleur.o json.o
SOBJS ?= serveur.o json.o

.SUFFIXES: .c .o

SERVER = serveur
CLIENT = client

all: $(SERVER) $(CLIENT)

$(SERVER): $(SOBJS)
	$(CC) -o $(SERVER) $(SOBJS) -lpthread

$(CLIENT): $(COBJS)
	$(CC) -o $(CLIENT) $(COBJS)


.c.o: 
	$(CC) -c $*.c -Wall -Wextra -g

