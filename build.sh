#!/bin/sh

# If using a not installed FLTK version, specify it's path here (with '/' at end)
#FLTK=../fltk/build/
FLTK_CONFIG="$FLTK"fltk-config

TARGET=fltk-gomoku
SRC=fltk-gomoku.cxx
if [ -f miniaudio.h ]; then
OPT=-DUSE_MINIAUDIO
fi

g++ -Wall $OPT -o $TARGET `$FLTK_CONFIG --use-images --cxxflags` $SRC `$FLTK_CONFIG --use-images --ldflags`
