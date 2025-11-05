CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`

myapp: main.c
	$(CC) $(CFLAGS) -o myapp main.c $(LIBS)

clean:
	rm -f myapp
