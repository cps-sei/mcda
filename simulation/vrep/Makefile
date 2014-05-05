VREP=$(VREP_MCDA_HOME)
CXXFLAGS=-I$(VREP)/programming/remoteApi -I$(VREP)/programming/include -DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255 -D__linux

all : testExtApi

testExtApi : extApi.o  extApiPlatform.o testExtApi.o 
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

testExtApi.o : testExtApi.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

extApi.o : $(VREP)/programming/remoteApi/extApi.c
	cc $(CXXFLAGS) -c -o $@ $<

extApiPlatform.o : $(VREP)/programming/remoteApi/extApiPlatform.c
	cc $(CXXFLAGS) -c -o $@ $<

clean :
	rm -f *.o testExtApi
