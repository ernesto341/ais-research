INC             =  ./inc
INCFLAGS        =  -I $(INC) -I /usr/include -I /usr/local/include
LIB             =  ./lib
CFLAGS          =  -Wall -Wextra -ggdb -O2 -pthread
CC              =  gcc $(CFLAGS) $(INCFLAGS)

all: tcpreassembly libntoh sfhash common 4d 6d example

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


example:
	$(CC) -o example example.c $(LIB)/*.o /usr/local/lib/libpcap.a


clean:
	rm -f *.*.*.*:*-*.*.*.*:*
	rm -f *~ *.bak
	rm -f *.o
	rm -f *.gch
	rm -f err
	rm -f core
	rm -f a.out
	rm -f /home/ernest/research/generic_dump
	rm -f /home/ernest/research/hdr_and_sig
	rm -f example
	rm -f $(INC)/*.o
	rm -f $(INC)/*.gch
	rm -f $(LIB)/*.o
	rm -f $(LIB)/*.gch


