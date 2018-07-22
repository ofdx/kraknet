# Kraknet Site System
# Written by Mike Perron (2013)
#
# To build:
# 	$ make
#
# To clean (remove compiled binaries):
# 	$ make clean

I=.
C=.
O=o
BIN=../bin
LIB=../lib
CC=gcc -L$(LIB) -I$(I) -s -O3 -Wall

all: $(O) $(BIN) $(LIB) $(LIB)/libkraknet.so $(BIN)/ws $(BIN)/mod_find $(BIN)/mod_home $(BIN)/kraknet $(BIN)/decode tags

clean:
	rm -f $(LIB)/kraknet.so $(O)/*.o $(BIN)/ws $(BIN)/kraknet $(BIN)/mod_find $(LIB)/libkraknet.so

tags: *.c *.h
	@if which ctags; then echo "generating ctags..."; ctags -R .; else echo "ctags not found; skipping"; fi

# libkraknet (shared library of common functions)
$(LIB)/libkraknet.so: $(C)/generic.c $(C)/utc_date.c $(C)/http11.c
	$(CC) -c -fPIC -o $(O)/generic.o $(C)/generic.c
	$(CC) -c -fPIC -o $(O)/utc_date.o $(C)/utc_date.c
	$(CC) -c -fPIC -o $(O)/http11.o $(C)/http11.c
	$(CC) -shared -o $(LIB)/libkraknet.so $(O)/generic.o $(O)/utc_date.o $(O)/http11.o -lrt
	chmod a-x $(LIB)/libkraknet.so


# krakws (web server)
$(BIN)/ws: $(C)/krakws.c $(O)/conf.o $(O)/handler.o $(LIB)/libkraknet.so
	$(CC) -Wall -o $(BIN)/ws $(C)/krakws.c $(O)/conf.o $(O)/handler.o -lkraknet

$(O)/conf.o: $(C)/conf.c $(I)/conf.h $(I)/generic.h
	$(CC) -c $(C)/conf.c -o $(O)/conf.o

$(O)/handler.o: $(C)/handler.c $(I)/handler.h $(I)/generic.h $(I)/conf.h $(I)/http11.h
	$(CC) -Wall -c $(C)/handler.c -o $(O)/handler.o

# mod_find (module:script locator)
$(BIN)/mod_find: $(C)/mod_find.c $(I)/generic.h $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/mod_find $(C)/mod_find.c -lkraknet

# mod_home
$(BIN)/mod_home: $(C)/mod_home.c $(I)/generic.h $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/mod_home $(C)/mod_home.c -lkraknet


# kraknet (text parsing)
$(BIN)/kraknet: $(C)/kraknet.c $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/kraknet $(C)/kraknet.c -lkraknet

# decode (URI Unescape)
$(BIN)/decode: $(C)/decode.c
	$(CC) -o $(BIN)/decode $(C)/decode.c


# directories
$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

$(LIB):
	mkdir $(LIB)
