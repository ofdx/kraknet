# Kraknet Site System
# Written by Mike Perron (2013)

# To build:
# 	$ make

# To clean (remove compiled binaries):
# 	$ make clean

I=i
C=c
O=o
CC=gcc
BIN=../bin

all: $(I) $(O) $(BIN) $(BIN)/ws $(BIN)/mod_find $(BIN)/kraknet

# krakws
$(BIN)/ws: $(O)/serv.o $(O)/http11.o $(O)/generic.o $(O)/utc_date.o $(O)/conf.o
	$(CC) -o $(BIN)/ws -s $(O)/serv.o $(O)/http11.o $(O)/generic.o $(O)/utc_date.o $(O)/conf.o -lrt -I$(I)

$(O)/serv.o: $(C)/serv.c $(I)/http11.h $(I)/generic.h $(I)/conf.h
	$(CC) -c $(C)/serv.c -o $(O)/serv.o -I$(I)

$(O)/http11.o: $(C)/http11.c $(I)/http11.h $(I)/generic.h
	$(CC) -c $(C)/http11.c -o $(O)/http11.o -I$(I)

$(O)/utc_date.o: $(C)/utc_date.c $(I)/utc_date.h
	$(CC) -c $(C)/utc_date.c -o $(O)/utc_date.o -I$(I)

$(O)/generic.o: $(C)/generic.c $(I)/generic.h
	$(CC) -c $(C)/generic.c -o $(O)/generic.o -I$(I)

$(O)/conf.o: $(C)/conf.c $(I)/conf.h $(I)/generic.h
	$(CC) -c $(C)/conf.c -o $(O)/conf.o -I$(I)


# mod_find 
$(O)/mod_find.o: $(C)/mod_find.c $(I)/generic.h
	$(CC) -c $(C)/mod_find.c -o $(O)/mod_find.o -I$(I)

$(BIN)/mod_find: $(O)/mod_find.o $(O)/generic.o
	$(CC) -o $(BIN)/mod_find -s $(O)/mod_find.o $(O)/generic.o -I$(I)


# kraknet
$(O)/kraknet.o: $(C)/kraknet.c $(I)/generic.h
	$(CC) -c $(C)/kraknet.c -o $(O)/kraknet.o -I$(I)

$(BIN)/kraknet: $(O)/kraknet.o $(O)/generic.o
	$(CC) -o $(BIN)/kraknet -s $(O)/kraknet.o $(O)/generic.o -I$(I)


# misc
$(I):
	mkdir $(I)

$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

clean:
	rm $(O)/*.o $(BIN)/ws $(BIN)/kraknet $(BIN)/mod_find
