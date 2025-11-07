CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -Wextra -O2
LIBS = `pkg-config --libs gtk+-3.0` -lsecp256k1 -lcrypto

# GTK Bitcoin Wallet application
btc_wallet: main.c bitcoin.c bitcoin.h
	$(CC) $(CFLAGS) -o btc_wallet main.c bitcoin.c $(LIBS)

# Keep old target for compatibility
myapp: main.c
	$(CC) $(CFLAGS) -o myapp main.c `pkg-config --libs gtk+-3.0`

clean:
	rm -f btc_wallet myapp

.PHONY: clean
