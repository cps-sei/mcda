ACE_LIB=$(ACE_ROOT)/lib/libACE.so
MADARA_LIB=$(MADARA_ROOT)/lib/libMADARA.so

all : daslc
	make -C simulation/vrep

daslc : $(ACE_LIB) $(MADARA_LIB)
	make -C tools/daig/src/daslc

$(ACE_LIB) :
	@echo "building ACE"
	(mkdir -p madara/ace && cd madara/ace && \
	svn co -r97453 svn://svn.dre.vanderbilt.edu/DOC/Middleware/trunk/ACE ACE_wrappers && \
	svn co -r2171 svn://svn.dre.vanderbilt.edu/DOC/MPC/trunk ACE_wrappers/MPC && \
	echo  '#include "ace/config-linux.h"' > $(ACE_ROOT)/ace/config.h && \
	echo 'include $(ACE_ROOT)/include/makeinclude/platform_linux.GNU' > $(ACE_ROOT)/include/makeinclude/platform_macros.GNU && \
	cd $(ACE_ROOT)/ace && \
	perl $(ACE_ROOT)/bin/mwc.pl -type gnuace ace.mwc && \
	make)

$(MADARA_LIB) :
	@echo "building MADARA"
	(cd madara && \
	svn co -r1301 http://madara.googlecode.com/svn/trunk madara && \
	cd madara && \
	perl $(ACE_ROOT)/bin/mwc.pl -type gnuace MADARA.mwc && \
	make tests=1)

clean :
	make -C tools/daig/src/daslc clean
	make -C simulation/vrep clean
