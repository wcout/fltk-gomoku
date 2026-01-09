#!/bin/sh

APPLICATION=fltk-gomoku

cat $APPLICATION.desktop | envsubst >~/.local/share/applications/$APPLICATION.desktop
chmod 0777 ~/.local/share/applications/$APPLICATION.desktop
cp -v $APPLICATION.png ~/.$APPLICATION/rsc/.
