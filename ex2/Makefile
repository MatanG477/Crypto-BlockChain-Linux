# Makefile for MTA Crypto Assignment

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2

# Libraries to link against (MTA crypto, pthreads, OpenSSL)
LDFLAGS = -lmta_rand -lmta_crypt -lpthread

# Main source file (change if your file name is different)
SRC = mta_crypto.c

# Output executable name
TARGET = mta_crypto.out

.PHONY: all clean

# Default target: build the main executable
all: $(TARGET)

# Link the executable from the source file
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean rule: remove the executable
clean:
	rm -f $(TARGET)
