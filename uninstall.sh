#!/bin/bash
DESTDIR=$1
DATA_PATH="$DESTDIR/usr/share/sar2"
GAME_PATH="$DESTDIR/usr/bin/sar2"
MAN_PATH="$DESTDIR/usr/share/man/man6/sar2.6.bz2"
ICON_NAME="sar2"
DESKTOP_MENU_NAME="sar2.desktop"

if [[ ! -w $DATA_PATH || ! -w $GAME_PATH ]]
then
    echo "The game is not installed or you do not"
    echo "have permission to uninstall it from"
    echo "from $GAME_PATH and $DATA_PATH."
    echo "Are you root?"
    exit 1
fi

if [[ -d $DATA_PATH ]]
then
    echo "Uninstalling data files..."
    rm -rf $DATA_PATH
fi

if [[ -f $GAME_PATH ]]
then
    echo "Uninstalling sar2 binary..."
    rm $GAME_PATH
fi

if [[ -f $MAN_PATH ]]
then
    echo "Uninstalling manual..."
    rm $MAN_PATH
fi

echo "Uninstalling icon..."
xdg-icon-resource uninstall --size 48 $ICON_NAME

echo "Uninstalling desktop menu entry..."
xdg-desktop-menu uninstall $DESKTOP_MENU_NAME
