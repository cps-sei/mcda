SRC=daig-scanner.cpp daig-parser.cpp DaigBuilder.cpp CopyVisitor.cpp \
SyncSeq.cpp SyncSem.cpp SyncSeqDbl.cpp ArrayElim.cpp \
../daig/Expression.cpp ../daig/Function.cpp \
../daig/Model_Of_Computation.cpp ../daig/Node.cpp ../daig/Program.cpp \
../daig/Variable.cpp ../daig/Statement.cpp ../daig/Type.cpp \
../daig/CProgram.cpp ../daig/Visitor.cpp \
../daig/madara/Sync_Builder.cpp ../daig/madara/Async_Builder.cpp \
../daig/madara/Madara_Builder.cpp \
../daig/madara/Function_Visitor.cpp
OBJ=$(patsubst %.cpp,%.o,$(SRC))
INC=-I$(ACE_ROOT) -I$(MADARA_ROOT)/include -I..
LIB=$(MADARA_ROOT)/libMADARA.so
CXX=g++

all : daslc

daslc : daslc.cpp daig-parser.hpp $(OBJ)
	$(CXX) -o $@ $(INC) daslc.cpp $(OBJ) $(LIB)

daig-scanner.cpp : daig-scanner.ll
	flex -o daig-scanner.cpp daig-scanner.ll

daig-parser.cpp daig-parser.hpp : daig-parser.yy
	bison -d -o daig-parser.cpp daig-parser.yy

%.o : %.cpp
	$(CXX) -c -o $@ $(INC) $<

%.d : %.cpp
	$(CXX) -MM -MT $(@:%.d=%.o) -o $@ $(INC) $<

clean :
	rm -f daslc daig-scanner.cpp daig-parser.cpp daig-parser.hpp \
stack.hh position.hh location.hh $(OBJ)

-include $(SRC:%.cpp=%.d)

