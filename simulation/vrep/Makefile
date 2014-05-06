VREP=$(VREP_MCDA_HOME)
CXXFLAGS=-I$(VREP)/programming/remoteApi -I$(VREP)/programming/include -DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255 -D__linux

all : testExtApi

testExtApi : testExtApi.o libDaslVrep.a 
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

testExtApi.o : testExtApi.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

libDaslVrep.a : extApi.o extApiPlatform.o DaslVrep.o
	ar rc $@ $^

extApi.o : $(VREP)/programming/remoteApi/extApi.c
	cc $(CXXFLAGS) -c -o $@ $<

extApiPlatform.o : $(VREP)/programming/remoteApi/extApiPlatform.c
	cc $(CXXFLAGS) -c -o $@ $<

DaslVrep.o : DaslVrep.cpp DaslVrep.hpp
	g++ $(CXXFLAGS) -c -o $@ $<

clean :
	rm -f *.o testExtApi
