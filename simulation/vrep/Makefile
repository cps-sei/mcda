VREP=$(VREP_MCDA_HOME)
CXXFLAGS=-I$(VREP)/programming/remoteApi -I$(VREP)/programming/include -DNON_MATLAB_PARSING -DMAX_EXT_API_CONNECTIONS=255

all : extApi.o  extApiPlatform.o extApiCustom.o

extApi.o : $(VREP)/programming/remoteApi/extApi.c
	g++ $(CXXFLAGS) -c -o $@ $<

extApiPlatform.o : $(VREP)/programming/remoteApi/extApiPlatform.c
	g++ $(CXXFLAGS) -c -o $@ $<

extApiCustom.o : $(VREP)/programming/remoteApi/extApiCustom.c
	g++ $(CXXFLAGS) -c -o $@ $<

clean :
	rm -f *.o
