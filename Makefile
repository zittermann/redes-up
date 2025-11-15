CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -O2

SERVER_SRC = main.c
CLIENT_SRC = client.c
SERVER_EXEC = main
CLIENT_EXEC = client

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_EXEC) $(SERVER_SRC)

$(CLIENT_EXEC): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_EXEC) $(CLIENT_SRC)

clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)

rebuild: clean all


