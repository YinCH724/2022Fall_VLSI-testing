
CXX      = g++ 
LINK     = g++ 
CXXFLAGS = -O2 -Wall -DDEBUG
#CXXFLAGS = -g -DDEBUG_ATPG -Wall #Turn on debugging
INCPATH  = 
LIBS     = -lreadline -lcurses
DEL_FILE = rm -f

####### Files

HEADERS = gate.h fault.h circuit.h GetLongOpt.h typeemu.h readcircuit.tab.h ReadPattern.h hash.h tfault.h ass4.h

SOURCES = readcircuit.tab.cc lex.yy.cc circuit.cc main.cc GetLongOpt.cc atpg.cc fsim.cc sim.cc psim.cc stfsim.cc tfatpg.cc ass0.cc ass2.cc ass3.cc ass5.cc ass6.cc atpg_bridge.cc

OBJECTS = readcircuit.tab.o lex.yy.o circuit.o main.o GetLongOpt.o atpg.o fsim.o sim.o psim.o stfsim.o tfatpg.o ass0.o ass2.o ass3.o ass5.o ass6.o atpg_bridge.o

TARGET  = atpg

####### Implicit rules

.SUFFIXES: .o .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

####### Build rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCPATH) -lm

$(OBJECTS): $(HEADERS) Makefile

readcircuit.tab.cc: readcircuit.y gate.h circuit.h
	bison -d -t readcircuit.y
	mv readcircuit.tab.c readcircuit.tab.cc

lex.yy.cc: readcircuit.l readcircuit.y
	flex readcircuit.l 
	mv lex.yy.c lex.yy.cc

clean:
	@$(DEL_FILE) $(OBJECTS) $(TARGET)
	@$(DEL_FILE) readcircuit.tab.cc readcircuit.tab.h lex.yy.cc

