# specify path to FLTK dir (mingw compiled)
FLTK_DIR=../../fltk-1.4-mingw32/

export ROOT?=$(PWD)/..

HOST=i686-w64-mingw32
CXX=$(shell sh -c 'which $(HOST)-g++')

ifeq ($(CXX),)
HOST=i586-pc-mingw32
CXX=$(shell sh -c 'which $(HOST)-g++')
endif

ifeq ($(CXX),)
HOST=i586-mingw32msvc
CXX=$(shell sh -c 'which $(HOST)-g++')
endif

ifeq ($(CXX),)
all clean:
	@tput setaf 1
	@echo "cross compiler for $(HOST) not installed."
	@tput sgr0
else

APPLICATION=fltk-gomoku

FLTK_CONFIG=$(FLTK_DIR)fltk-config

SRC=$(ROOT)

OBJ=\
	$(APPLICATION).o

INCLUDE=-I$(ROOT)/include -I.

LDFLAGS=
LDLIBS=`$(FLTK_CONFIG) --use-images --ldstaticflags` -static -lgcc -static -lstdc++ -static -lwinmm
LDLIBSCPP=-static -lgcc -static -lstdc++ -dynamic -static -lwinmm

FLTKCXXFLAGS = `$(FLTK_CONFIG) --cxxflags`
CXXDEFS+=-DUSE_FLTK_RUN=$(USE_FLTK_RUN)
CXXDEFS+=-DHAVE_SLOW_CPU=$(HAVE_SLOW_CPU)
CXXFLAGS+=$(CXXDEFS) -g -Wall -pedantic $(INCLUDE) $(FLTKCXXFLAGS)
#OPT=
OPT=-O3 -DNDEBUG
OPT+=-DFLTK_USE_NANOSVG


CP=cp
MV=mv
RM=rm
MKDIR=mkdir
PATCH=patch
TAR=tar

TARGET=$(APPLICATION).exe

.PHONY: clean all target

all:: $(TARGET)

$(TARGET): depend $(OBJ)
	@echo Linking $@...
	$(CXX) -o $@ $(LDFLAGS) $(OBJ) $(LDLIBS)

%.o: $(SRC)/%.cxx
	@echo Compiling $@...
	$(CXX) -c -o $@ $< $(OPT) $(CXXFLAGS)

clean::
	$(RM) -f $(ALL) *.o core makedepend depend *~
	$(RM) -f $(TARGET)

depend: $(SRC)/*.cxx
	@echo Make dependencies..
	$(CXX) $(CXXFLAGS) -M $(SRC)/*.cxx >makedepend
	# only a dummy
	touch depend

-include makedepend

endif
