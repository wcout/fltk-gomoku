#!/bin/sh

# If using a not installed FLTK version, specify it's path here (with '/' at end)
#FLTK=../fltk-1.4/
FLTK_CONFIG="$FLTK"fltk-config

TARGET=fltk-gomoku
SRC=fltk-gomoku.cxx
OPT=-DFLTK_USE_NANOSVG

g++ -Wall $OPT -o $TARGET `$FLTK_CONFIG --use-images --cxxflags` $SRC `$FLTK_CONFIG --use-images --ldflags`
