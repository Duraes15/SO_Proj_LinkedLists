CC      := gcc
CFLAGS  := -Wall -g

# acrescenta LLFunctions.c a ambos os builds
COMMON_SRC     := LLFunctions.c
CLIENT_SRC     := dclient.c client.c $(COMMON_SRC)
SERVER_SRC     := dserver.c server.c $(COMMON_SRC)

CLIENT_BIN := dclient
SERVER_BIN := dserver

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC) dclient.h
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC)

$(SERVER_BIN): $(SERVER_SRC) dserver.h
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC)

clean:
	rm -f *.o

fclean: clean
	rm -f $(CLIENT_BIN) $(SERVER_BIN)

re: fclean all
