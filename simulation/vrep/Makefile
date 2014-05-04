VREP=$(VREP_MCDA_HOME)
CXXFLAGS=-I$(VREP)/programming/remoteApi -I$(VREP)/programming/include -DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255

all : testExtApi

testExtApi : extApi.o  extApiPlatform.o testExtApi.cpp
	g++ $(CXXFLAGS) -o $@ $^ -lpthread

extApi.o : $(VREP)/programming/remoteApi/extApi.c
	g++ $(CXXFLAGS) -c -o $@ $<

extApiPlatform.o : $(VREP)/programming/remoteApi/extApiPlatform.c
	g++ $(CXXFLAGS) -c -o $@ $<

clean :
	rm -f *.o testExtApi
