# Makefile – קומפילציה בלבד של קבצי C

CC      = gcc
CFLAGS  = -Wall
LDFLAGS = -lmta_crypt -lmta_rand

ENCRYPTER_SRC = mta-encrypter.c
ENCRYPTER_BIN = encrypter

DECRYPTER_SRC = mta-decrypter.c
DECRYPTER_BIN = decrypter

.PHONY: all clean

all: $(ENCRYPTER_BIN) $(DECRYPTER_BIN)

$(ENCRYPTER_BIN): $(ENCRYPTER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(DECRYPTER_BIN): $(DECRYPTER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(ENCRYPTER_BIN) $(DECRYPTER_BIN)
