CFLAGS = -g -Wno-deprecated
OFILES = antibody.o webdata.o heap.o mass.o random.o unknownweb.o

lifetime:	lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.1000 -DMAX_ANTIBODIES=1000 lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.500 -DMAX_ANTIBODIES=500 lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.250 -DMAX_ANTIBODIES=250 lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.125 -DMAX_ANTIBODIES=125 lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.75 -DMAX_ANTIBODIES=75 lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.50 -DMAX_ANTIBODIES=50 lifetime.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime.25 lifetime.cpp ${OFILES}

ulifetime:	lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.1000 -DMAX_ANTIBODIES=1000 lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.500 -DMAX_ANTIBODIES=500 lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.250 -DMAX_ANTIBODIES=250 lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.125 -DMAX_ANTIBODIES=125 lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.75 -DMAX_ANTIBODIES=75 lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.50 -DMAX_ANTIBODIES=50 lifetime_nounknown.cpp ${OFILES}
	g++ $(CFLAGS) -o lifetime_nounknown.25 lifetime_nounknown.cpp ${OFILES}

test:	antibody.o test.cpp
	g++ $(CFLAGS) test.cpp antibody.o heap.o

unknownweb.o:	webdata.o unknownweb.h unknownweb.cpp
	g++ $(CLFAGS) -c unknownweb.cpp

webdata.o:	antibody.h webdata.h webdata.cpp
	g++ $(CFLAGS) -c webdata.cpp

heap.o:	antibody.h heap.h heap.cpp
	g++ $(CFLAGS) -c heap.cpp

mass.o: antibody.h heap.h mass.h mass.cpp random.h
	g++ $(CFLAGS) -c mass.cpp

antibody.o:	antibody.h antibody.cpp random.h
	g++ $(CFLAGS) -c antibody.cpp

random.o:	random.h random.cpp
	g++ $(CFLAGS) -c random.cpp

clean:	
	rm -f *.o lifetime a.out core lifetime.25 lifetime.50 lifetime.75 lifetime.125 lifetime.250 lifetime.500 lifetime.1000 champions.abs
