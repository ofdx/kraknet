# Kraknet Site System
# Written by Mike Perron (2013)
I=i
C=c
O=o
BIN=../bin

all: $(I) $(O) $(BIN) $(BIN)/ws $(BIN)/mod_find $(BIN)/kraknet

# krakws
$(BIN)/ws: $(O)/serv.o $(O)/http11.o $(O)/generic.o $(O)/utc_date.o
	cc -o $(BIN)/ws -s $(O)/serv.o $(O)/http11.o $(O)/generic.o $(O)/utc_date.o -lrt -I$(I)

$(O)/serv.o: $(C)/serv.c $(I)/http11.h $(I)/generic.h
	cc -c $(C)/serv.c -o $(O)/serv.o -I$(I)

$(O)/http11.o: $(C)/http11.c $(I)/http11.h $(I)/generic.h
	cc -c $(C)/http11.c -o $(O)/http11.o -I$(I)

$(O)/utc_date.o: $(C)/utc_date.c $(I)/utc_date.h
	cc -c $(C)/utc_date.c -o $(O)/utc_date.o -I$(I)

$(O)/generic.o: $(C)/generic.c $(I)/generic.h
	cc -c $(C)/generic.c -o $(O)/generic.o -I$(I)


# mod_find 
$(O)/mod_find.o: $(C)/mod_find.c $(I)/generic.h
	cc -c $(C)/mod_find.c -o $(O)/mod_find.o -I$(I)

$(BIN)/mod_find: $(O)/mod_find.o $(O)/generic.o
	cc -o $(BIN)/mod_find -s $(O)/mod_find.o $(O)/generic.o -I$(I)


# kraknet
$(O)/kraknet.o: $(C)/kraknet.c $(I)/generic.h
	cc -c $(C)/kraknet.c -o $(O)/kraknet.o -I$(I)

$(BIN)/kraknet: $(O)/kraknet.o $(O)/generic.o
	cc -o $(BIN)/kraknet -s $(O)/kraknet.o $(O)/generic.o -I$(I)


# misc
$(I):
	mkdir $(I)

$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

clean:
	rm $(O)/*.o $(BIN)/ws $(BIN)/kraknet $(BIN)/mod_find
