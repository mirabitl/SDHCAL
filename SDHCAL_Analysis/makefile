
INCLUDES=-I${CULA_INC_PATH} -I${HOME}/lapack-3.4.2/lapacke/include -I/usr/local/cuda/include \
		-I$(LCIODIR)/include -I include -I src -I/usr/include/libxml2 -I/usr/include/python2.7/ \
		-I/usr/local/include/eigen3/  -Dcimg_use_vt100 -I/usr/X11R6/include \
		-I${ROOTSYS}/include  
LIBPATH32=-L${CULA_LIB_PATH_32} -L${INTEL_LIB_PATH_32} -L${MKL_LIB_PATH_32}
LIBPATH64=-L${CULA_LIB_PATH_64} -L${INTEL_LIB_PATH_64} -L${MKL_LIB_PATH_64} -L${HOME}/lapack-3.4.2

LIBSCULA=-lcula_core -lcula_lapack -lcublas -lcudart -lpthread

ObjSuf        = o
SrcSuf        = cxx
ExeSuf        =
DllSuf        = so
EVENTLIB      = $(EVENTO)
OutPutOpt     = -o
ifeq ($(shell uname -s),Darwin)
 SOFLAGS       = -dynamiclib -fPIC
 ROOTLIBS      = `$(ROOTSYS)/bin/root-config --libs --glibs` -lNew -lXMLIO
else
 SOFLAGS       = -shared -fPIC
 ROOTLIBS      = `$(ROOTSYS)/bin/root-config --libs --glibs` -lXMLIO
endif

ROOTGLIBS     = -lGpad -lGui -lGX11 -lX3d
ROOTCINT      = ${ROOTSYS}/bin/rootcint
#LCIODIR       = /data/online/lcio/v01-12/

SRC= $(wildcard src/*.cxx)
OBJ= $(SRC:.cxx=.o)
EXEOBJ= $(SRC:.cc:.o)

# Linux with egcs
#
#
#
#
 CXX           = g++
 CXXFLAGS      =  -O3 -Wall -fPIC $(INCLUDES) 	`pkg-config --cflags opencv`
 LD            = g++ -m64
 LDFLAGS       = -O3 



LIBS          =  -lm  -L$(LCIODIR)/lib -llcio  -lxml2 -lsqlite3 -lm -L/usr/X11R6/lib -lpthread -lX11 $(LIBPATH64) $(LIBSCULA) `pkg-config --libs opencv`
EXTOBJ = 
GLIBS         = `root-config --glibs --libs`   
MYLIBS         = `mysql_config --libs`   
SHELL         = /bin/bash


%.o: %.cxx
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o: %.cc
	$(CXX) -o $@ -c $< $(CXXFLAGS)

objcxx:  $(OBJ)
	@cp src/*.o lib


#------------------------------------------------------------------------------



OBJC          = $(EVENTO) 
EVENTO        =  DCEvent.$(ObjSuf) \
		 DCRun.$(ObjSuf)  \
		DCEventHeader.$(ObjSuf)  \
		 DCAnalysis.$(ObjSuf)  \
		 DCFileAnalysis.$(ObjSuf)  \
		 DCFrame.$(ObjSuf)  \
		 DCBufferReader.$(ObjSuf)  \
                DCRU.o  DCEventCint.$(ObjSuf)\
		 DCScurveMaker.$(ObjSuf)  \




EVENTSO       = libDHCEvent.$(DllSuf)
$(EVENTSO): objcxx
	$(LD) $(SOFLAGS) $(LDFLAGS) $(OBJ) $(OutPutOpt) $(EVENTSO) `root-config --libs` $(EXTOBJ) $(LIBS) $(MYLIBS) -lXMLParser -lXMLIO
	@ mv $(EVENTSO) lib


EVENT = dumpevent 

dumpevent: src/dumpevent.o
	$(CXX) -o $@ src/dumpevent.o -Llib -lDHCEvent $(LIBS) $(ROOTLIBS) $(MYLIBS)
	@mv dumpevent bin

streamout: src/streamout.o
	$(CXX) -o $@ src/streamout.o -Llib -lDHCEvent $(LIBS) $(ROOTLIBS) $(MYLIBS)
	@mv streamout bin

chamber: src/chamber.o
	$(CXX) -o $@ src/chamber.o -Llib -lDHCEvent $(LIBS) $(ROOTLIBS) $(MYLIBS)
	@mv chamber bin

scurvemaker: src/scurveplot.o
	$(CXX) -o $@ src/scurveplot.o  -Llib -lDHCEvent $(LIBS) $(ROOTLIBS) $(MYLIBS)
	@mv scurvemaker bin

TestProcesseur: src/TestProcesseur.o
	$(CXX) -o $@ src/TestProcesseur.o -Llib -lDHCEvent $(LIBS) $(ROOTLIBS) $(MYLIBS)
	@mv TestProcesseur bin


PROGRAMS      = $(EVENT)


lib:	$(EVENTSO)
obj:  objcxx
doc: Doxyfile
	@doxygen;cd doc/latex;make;dvipdf refman;cd -
all:
	@make lib
	@make dumpevent
	@make streamout
	@make chamber




clean:
		@rm -f lib/*.o lib/*.so src/*.o src/DCEventCint.* core


DHCalEventReader.i:include/DHCalEventReader.h

DHCalEventReader_wrap.cxx: DHCalEventReader.i include/DHCalEventReader.h
	swig -c++ -python -classic -Iinclude  -I${LCIODIR}/include -I${ROOTSYS}/include DHCalEventReader.i 

_DHCalEventReader.so: DHCalEventReader_wrap.o
	$(LD) -shared $(LDFLAGS)  -L./lib -lDHCEvent  DHCalEventReader_wrap.o -o $@ $(LIBS) $(ROOTLIBS) $(MYLIBS)


showers: showers.h showers.C
	$(CXX) -o showers showers.C $(CXXFLAGS) -L./lib -lDHCEvent	$(LIBS) $(ROOTLIBS) -lMinuit 
tracks: tracks.h tracks.C
	$(CXX) -o tracks tracks.C $(CXXFLAGS) -L./lib -lDHCEvent	$(LIBS) $(ROOTLIBS) -lMinuit 





HEADERS =  DCRun.h  \
	DCEventHeader.h \
        DCEvent.h \
	DCAnalysis.h \
	DCFileAnalysis.h 

DCEventCint: 
	@touch DCEventCint.h 
	@rm DCEventCint.h 
	@echo "Generating dictionary ..."
	@touch DCEventCint.$(SrcSuf)
	@rm DCEventCint.$(SrcSuf)
	ROOTSYS=$(ROOTSYS) LD_LIBRARY_PATH=$(ROOTSYS)/lib cd include;\
	touch DCEventCint.h;\
	rm DCEventCint.h;\
	echo "Generating dictionary ...";\
	touch DCEventCint.cxx;\
	rm DCEventCint.cxx;\
	$(ROOTCINT) DCEventCint.$(SrcSuf)  -c $(HEADERS)  DCLinkDef.h;\
	mv  DCEventCint.cxx ../src







