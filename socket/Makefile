# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Directories
SERVER_DIR = server
CLIENT_DIR = client

# Source files
SERVER_SRC = $(SERVER_DIR)/main.c
CLIENT_SRC = $(CLIENT_DIR)/main.c

# Output binaries
SERVER_BIN = $(SERVER_DIR)/server
CLIENT_BIN = $(CLIENT_DIR)/client

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Build the server
$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Build the client
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Run the server and client
run: all
	@echo "Starting the server..."
	@$(SERVER_BIN) & # Run the server in the background
	@sleep 1 # Give the server time to start
	@echo "Starting the client..."
	@$(CLIENT_BIN)

# Clean the compiled files
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

