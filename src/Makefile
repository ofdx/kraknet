# Kraknet Site System
# Written by Mike Perron (2013)
i=i
c=c
o=o
bin=../bin

all: $(i) $(o) $(bin) $(bin)/ws $(bin)/mod_find $(bin)/kraknet

# krakws
$(bin)/ws: $(o)/serv.o $(o)/http11.o $(o)/generic.o $(o)/utc_date.o
	cc -o $(bin)/ws -s $(o)/serv.o $(o)/http11.o $(o)/generic.o $(o)/utc_date.o -lrt -I$(i)

$(o)/serv.o: $(c)/serv.c $(i)/http11.h $(i)/generic.h
	cc -c $(c)/serv.c -o $(o)/serv.o -I$(i)

$(o)/http11.o: $(c)/http11.c $(i)/http11.h $(i)/generic.h
	cc -c $(c)/http11.c -o $(o)/http11.o -I$(i)

$(o)/utc_date.o: $(c)/utc_date.c $(i)/utc_date.h
	cc -c $(c)/utc_date.c -o $(o)/utc_date.o -I$(i)

$(o)/generic.o: $(c)/generic.c $(i)/generic.h
	cc -c $(c)/generic.c -o $(o)/generic.o -I$(i)


# mod_find 
$(o)/mod_find.o: $(c)/mod_find.c $(i)/generic.h
	cc -c $(c)/mod_find.c -o $(o)/mod_find.o -I$(i)

$(bin)/mod_find: $(o)/mod_find.o $(o)/generic.o
	cc -o $(bin)/mod_find -s $(o)/mod_find.o $(o)/generic.o -I$(i)


# kraknet
$(o)/kraknet.o: $(c)/kraknet.c $(i)/generic.h
	cc -c $(c)/kraknet.c -o $(o)/kraknet.o -I$(i)

$(bin)/kraknet: $(o)/kraknet.o $(o)/generic.o
	cc -o $(bin)/kraknet -s $(o)/kraknet.o $(o)/generic.o -I$(i)


# misc
$(i):
	mkdir $(i)

$(o):
	mkdir $(o)

$(bin):
	mkdir $(bin)

clean:
	rm $(o)/*.o $(bin)/ws $(bin)/kraknet $(bin)/mod_find
