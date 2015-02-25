INC             =  ./inc
RET             =  ./retdir
AIS             =  ./ais
INCFLAGS        =  -I $(INC) -I $(RET) -I /usr/include -I /usr/local/include -I $(AIS)
LIB             =  ./lib
CFLAGS          =  -Wall -Wextra -ggdb -O2 -lpthread
CC              =  gcc $(CFLAGS) $(INCFLAGS)
CP              =  g++ $(CFLAGS) $(INCFLAGS)
DHSFILES        =  $(LIB)/itoa.o $(LIB)/ipv6defrag.o $(LIB)/ipv4defrag.o $(LIB)/common.o $(LIB)/sfhash.o $(LIB)/tcpreassembly.o $(LIB)/libntoh.o $(LIB)/share.o
RETFILES        =  $(LIB)/pull.o $(LIB)/antibody.o $(LIB)/random.o $(LIB)/import.o $(LIB)/itoa.o

all: retrieve dump ais test_module

retrieve: rand ab itoa imp pull consumer

dump: tcpreassembly libntoh sfhash common 4d 6d itoa share prod

test_module: testify

ais: do_ais

do_ais:
	cd ais/ && make

testify:
	g++ $(CFLAGS) tester.cpp -o tester -lcurl

imp:
	$(CP) -c -o $(LIB)/import.o $(RET)/import.cpp -Wno-unused-parameter

pull:
	$(CP) -c -o $(LIB)/pull.o $(RET)/pull.cpp -Wno-unused-parameter

ab:
	$(CP) -c -o $(LIB)/antibody.o $(AIS)/antibody.cpp

rand:
	$(CP) -c -o $(LIB)/random.o $(AIS)/random.cpp

consumer:
	$(CP) $(RET)/retrieve.cpp -o $(RET)/retrieve $(RETFILES)

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
	$(CC) -o $(LIB)/share.o -c $(INC)/share.c

prod: /usr/local/lib/libpcap.a
	$(CC) -o dhs dhs.c /usr/local/lib/libpcap.a $(DHSFILES)

veryclean: clean
	cd ais/ && make clean

clean:
	rm -f *.*.*.*:*-*.*.*.*:*
	rm -f *~ *.bak
	rm -f *.log
	rm -f *.dat
	rm -f *.o
	rm -f *.gch
	rm -f err
	rm -f dhs_err
	rm -f ret_err
	rm -f core
	rm -f ret
	rm -f dhs
	rm -f tester
	rm -f $(RET)/retrieve
	rm -f $(RET)/*.gch
	rm -f $(RET)/*.o
	rm -f $(LIB)/*.o
	rm -f $(LIB)/*.gch
	rm -f $(INC)/*.o
	rm -f $(INC)/*.gch
	rm -f ./a.out

