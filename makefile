CC = gcc

SRCDIR = src
CFLAGS = -std=c99
LIBS = -lpthread -lconfig
BINDIR = bin
 
all: Bindir Common.o md5.o Crypto.o Config.o Serializer.o Client.o Server.o
	$(CC) $(CFLAGS) $(SRCDIR)/main.c $(BINDIR)/Common.o $(BINDIR)/md5.o $(BINDIR)/Crypto.o $(BINDIR)/Config.o $(BINDIR)/Serializer.o $(BINDIR)/Client.o $(BINDIR)/Server.o -o $(BINDIR)/sftu $(LIBS)
 
Server.o: Bindir md5.o Common.o Crypto.o Serializer.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/Server.c -o $(BINDIR)/Server.o $(LIBS)
 
Client.o: Bindir md5.o Common.o Serializer.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/Client.c -o $(BINDIR)/Client.o $(LIBS)

Serializer.o: Bindir Common.o Crypto.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/Serializer.c -o $(BINDIR)/Serializer.o
	
Config.o: Bindir Common.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/Config.c -o $(BINDIR)/Config.o $(LIBS)

Crypto.o: Bindir md5.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/Crypto.c -o $(BINDIR)/Crypto.o
 
md5.o: Bindir
	$(CC) $(CFLAGS) -c $(SRCDIR)/md5.c -o $(BINDIR)/md5.o

Common.o: Bindir
	$(CC) $(CFLAGS) -c $(SRCDIR)/Common.c -o $(BINDIR)/Common.o
	
Bindir:
	mkdir $(BINDIR)

clean:
	rm -r $(BINDIR)