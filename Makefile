INC             =  ./inc
INCFLAGS        =  -I $(INC) -I /usr/include -I /usr/local/include
LIB             =  ./lib
CFLAGS          =  -Wall -Wextra -ggdb -O2 -pthread
CC              =  gcc $(CFLAGS) $(INCFLAGS)

all: tcpreassembly libntoh sfhash common 4d 6d itoa share dhs retrieve

retrieve:
	g++ $(INCFLAGS) $(CFLAGS) retrieve.cpp -o retrieve

itoa:
	$(CC) -o $(LIB)/itoa.o -c $(INC)/itoa.c

6d:
	$(CC) -o $(LIB)/ipv6defrag.o -c $(INC)/ipv6defrag.c

4d:
	$(CC) -o $(LIB)/ipv4defrag.o -c $(INC)/ipv4defrag.c

common:
	$(CC) -o $(LIB)/common.o -c $(INC)/common.c

sfhash:
	$(CC) -o $(LIB)/sfhash.o -c $(INC)/sfhash.c

tcpreassembly:
	$(CC) -o $(LIB)/tcpreassembly.o -c $(INC)/tcpreassembly.c

libntoh:
	$(CC) -o $(LIB)/libntoh.o -c $(INC)/libntoh.c

share:
	$(CC) -o $(LIB)/share.o -c $(INC)/share.c


dhs: /usr/local/lib/libpcap.a
	$(CC) -o dhs dhs.c $(LIB)/*.o /usr/local/lib/libpcap.a


clean:
	rm -f *.*.*.*:*-*.*.*.*:*
	rm -f *~ *.bak
	rm -f *.o
	rm -f *.gch
	rm -f err
	rm -f core
	rm -f dhs
	rm -f retrieve
	rm -f $(LIB)/*.o
	rm -f $(LIB)/*.gch
	rm -f $(INC)/*.o
	rm -f $(INC)/*.gch

