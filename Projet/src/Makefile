CC ?= gcc

COBJS ?= client.o bmp.o couleur.o
SOBJS ?= serveur.o
JOBJS ?= json.o 

.SUFFIXES: .c .o

SERVER = serveur
CLIENT = client
JSON = json

all: $(SERVER) $(CLIENT) $(JSON)

$(SERVER): $(SOBJS)
	$(CC) -o $(SERVER) $(SOBJS)

$(CLIENT): $(COBJS)
	$(CC) -o $(CLIENT) $(COBJS)

$(JSON): $(JOBJS)
	$(CC) -o $(JSON) $(JOBJS)

.c.o: 
	$(CC) -c $*.c -Wall

