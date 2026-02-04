# If using a not installed FLTK version, specify it's path here (with '/' at end)
#FLTK := ../fltk/build/
FLTK_CONFIG := $(FLTK)fltk-config

SRC := fltk-gomoku.cxx
OBJ := $(SRC:.cxx=.o)
TGT := $(SRC:.cxx=)

ifeq ($(wildcard miniaudio.h),)
else
OPT += -DUSE_MINIAUDIO
endif

$(TGT): $(SRC)
	$(CXX) -std=c++17 -g -O2 -Wall $(OPT) -o $(TGT) `$(FLTK_CONFIG) --use-images --cxxflags` $(SRC) `$(FLTK_CONFIG) --use-images --ldflags`

clean:
	rm $(TGT)

fetch-miniaudio:
	wget  https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h

cppcheck:
	cppcheck -I src -I include --std=c++20 --max-configs=4 --enable=all --disable=missingInclude --disable=information --check-level=exhaustive $(SRC)
