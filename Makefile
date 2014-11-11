INC             =  ./inc
RET             =  ./retdir
AIS             =  ./ais
INCFLAGS        =  -I $(INC) -I $(RET) -I /usr/include -I /usr/local/include -I $(AIS)
LIB             =  ./lib
CFLAGS          =  -Wall -Wextra -ggdb -O2 -lpthread -Wno-int-to-pointer-cast
CC              =  gcc $(CFLAGS) $(INCFLAGS)
DHSFILES        =  $(LIB)/itoa.o $(LIB)/ipv6defrag.o $(LIB)/ipv4defrag.o $(LIB)/common.o $(LIB)/sfhash.o $(LIB)/tcpreassembly.o $(LIB)/libntoh.o $(LIB)/share.o
RETFILES        =  $(LIB)/pull.o $(LIB)/antibody.o $(LIB)/random.o

all: retrieve dhs

retrieve: rand ab pull consumer

dhs: tcpreassembly libntoh sfhash common 4d 6d itoa share prod

pull:
	g++ $(INCFLAGS) $(CFLAGS) -c -o $(LIB)/pull.o $(RET)/pull.cpp -Wno-unused-variable

ab:
	g++ $(INCFLAGS) $(CFLAGS) -c -o $(LIB)/antibody.o $(AIS)/antibody.cpp

rand:
	g++ $(INCFLAGS) $(CLFAGS) -c -o $(LIB)/random.o $(AIS)/random.cpp

consumer:
	g++ $(INCFLAGS) $(CFLAGS) $(RET)/retrieve.cpp -o $(RET)/retrieve $(RETFILES) -Wno-unused-variable

itoa:
	$(CC) -o $(LIB)/itoa.o -c $(INC)/itoa.c

6d:
	$(CC) -o $(LIB)/ipv6defrag.o -c $(INC)/ipv6defrag.c -Wno-missing-field-initializers

4d:
	$(CC) -o $(LIB)/ipv4defrag.o -c $(INC)/ipv4defrag.c -Wno-missing-field-initializers

common:
	$(CC) -o $(LIB)/common.o -c $(INC)/common.c

sfhash:
	$(CC) -o $(LIB)/sfhash.o -c $(INC)/sfhash.c

tcpreassembly:
	$(CC) -o $(LIB)/tcpreassembly.o -c $(INC)/tcpreassembly.c -Wno-missing-field-initializers -Wno-unused-parameter

libntoh:
	$(CC) -o $(LIB)/libntoh.o -c $(INC)/libntoh.c

share:
	$(CC) -o $(LIB)/share.o -c $(INC)/share.c -Wno-unused-variable

prod: /usr/local/lib/libpcap.a
	$(CC) -o dhs dhs.c /usr/local/lib/libpcap.a $(DHSFILES)
#	$(CC) -o dhs dhs.c /usr/local/lib/libpcap.a $(LIB)/itoa.o $(LIB)/ipv6defrag.o $(LIB)/ipv4defrag.o $(LIB)/common.o $(LIB)/sfhash.o $(LIB)/tcpreassembly.o $(LIB)/libntoh.o $(LIB)/share.o

clean:
	rm -f *.*.*.*:*-*.*.*.*:*
	rm -f *~ *.bak
	rm -f *.o
	rm -f *.gch
	rm -f err
	rm -f dhs_err
	rm -f ret_err
	rm -f core
	rm -f dhs
	rm -f $(RET)/retrieve
	rm -f $(RET)/*.gch
	rm -f $(RET)/*.o
	rm -f dhs
	rm -f $(LIB)/*.o
	rm -f $(LIB)/*.gch
	rm -f $(INC)/*.o
	rm -f $(INC)/*.gch

