CC = gcc
CFLAGS = -Wall -g

CLIENT_SRC = dclient.c client.c 
SERVER_SRC = dserver.c server.c 
CLIENT_BIN = dclient
SERVER_BIN = dserver

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC)

clean:
	rm -f *.o

fclean: clean
	rm -f $(CLIENT_BIN) $(SERVER_BIN)

re: fclean all