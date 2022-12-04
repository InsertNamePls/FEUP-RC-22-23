# Makefile to build the project

# Parameters
CC = gcc
CFLAGS = -Wall

SRC = src/
INCLUDE = include/
BIN = bin/

HOSTNAME = google.com

# Targets
.PHONY: all
all: $(BIN)/download $(BIN)/getip $(BIN)/clientTCP

$(BIN)/download: main.c #$(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE)

$(BIN)/getip: $(SRC)/getip.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE)

$(BIN)/clientTCP: $(SRC)/clientTCP.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE)

.PHONY: run_getIPexample
run_getIPexample: $(BIN)/getip
	./$(BIN)/getip $(HOSTNAME)

.PHONY: run_clientTCPexample
run_clientTCPexample: $(BIN)/clientTCP
	./$(BIN)/clientTCP

.PHONY: clean
clean:
	rm -f $(BIN)/download
	rm -f $(BIN)/getip
	rm -f $(BIN)/clientTCP
