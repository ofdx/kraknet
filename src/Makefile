# Kraknet Site System
# Written by Mike Perron (2013)
#
# To build:
# 	$ make
#
# To clean (remove compiled binaries):
# 	$ make clean

I=i
C=c
O=o
BIN=../bin
LIB=../lib
CC=gcc -L$(LIB) -I$(I)

all: $(O) $(BIN) $(LIB) $(LIB)/libkraknet.so $(BIN)/ws $(BIN)/mod_find $(BIN)/kraknet 

clean:
	rm -f $(LIB)/kraknet.so $(O)/*.o $(BIN)/ws $(BIN)/kraknet $(BIN)/mod_find $(LIB)/libkraknet.so


# libkraknet (shared library of common functions)
$(LIB)/libkraknet.so: $(C)/generic.c $(C)/utc_date.c
	$(CC) -c -fPIC -o $(O)/generic.o $(C)/generic.c
	$(CC) -c -fPIC -o $(O)/utc_date.o $(C)/utc_date.c
	$(CC) -shared -o $(LIB)/libkraknet.so $(O)/generic.o $(O)/utc_date.o -lrt
	chmod a-x $(LIB)/libkraknet.so


# krakws (web server)
$(BIN)/ws: $(O)/serv.o $(O)/http11.o $(O)/conf.o $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/ws -s $(O)/serv.o $(O)/http11.o $(O)/conf.o -lkraknet

$(O)/serv.o: $(C)/serv.c $(I)/http11.h $(I)/generic.h $(I)/conf.h
	$(CC) -c $(C)/serv.c -o $(O)/serv.o

$(O)/http11.o: $(C)/http11.c $(I)/http11.h $(I)/generic.h
	$(CC) -c $(C)/http11.c -o $(O)/http11.o

$(O)/conf.o: $(C)/conf.c $(I)/conf.h $(I)/generic.h
	$(CC) -c $(C)/conf.c -o $(O)/conf.o


# mod_find (module:script locator)
$(BIN)/mod_find: $(C)/mod_find.c $(I)/generic.h $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/mod_find -s $(C)/mod_find.c -lkraknet


# kraknet (text parsing)
$(BIN)/kraknet: $(C)/kraknet.c $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/kraknet -s $(C)/kraknet.c -lkraknet


# directories
$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

$(LIB):
	mkdir $(LIB)
