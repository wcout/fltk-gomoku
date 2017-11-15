# If using a not installed FLTK version, specify it's path here (with '/' at end)
FLTK := ../fltk-1.4/
FLTK_CONFIG := $(FLTK)fltk-config

SRC := fltk-gomoku.cxx
OBJ := $(SRC:.cxx=.o)
TGT := $(SRC:.cxx=)

all:
	$(CXX) -g -O2 -Wall -o $(TGT) `$(FLTK_CONFIG) --use-images --cxxflags` $(SRC) `$(FLTK_CONFIG) --use-images --ldflags`
