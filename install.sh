#!/bin/sh

APPLICATION=fltk-gomoku

cp -v $APPLICATION ~/bin/.
mkdir -p ~/.$APPLICATION/rsc
cd rsc
sh install.sh
cd ..
