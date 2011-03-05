#!/bin/bash
BIN_FILE="bin/sar2"
DATA_FOLDER="data"
MAN_FOLDER="man"
ICON_FILE="extra/sar2.xpm"
DESKTOP_FILE="extra/sar2.desktop"

DATA_PATH="/usr/share/games/sar2"
GAME_PATH="/usr/bin/sar2"
MAN_PATH="/usr/share/man/man6"

if [[ ! -f $BIN_FILE ]]
then
    echo "You need to compile Search And Rescue II first"
    exit 1
fi

data_location=`dirname $DATA_PATH`
game_location=`dirname $GAME_PATH`

if [[ ! -w $game_location || ! -w $data_location ]]
then
    echo "You do not have permission to install the"
    echo "the game in $game_location and $data_location."
    echo "Are you root?"
    exit 1
fi


if [[ -d $DATA_PATH ]]
then
    echo "Cleaning older data files..."
    rm -rf $DATA_PATH
fi

echo "Creating data folder..."
mkdir -p $DATA_PATH
echo "Copying data..."
cp -r $DATA_FOLDER/* $DATA_PATH
echo "Installing manual..."
cp $MAN_FOLDER/* $MAN_PATH
echo "Installing sar2..."
cp $BIN_FILE $GAME_PATH
echo "Installing icon..."
xdg-icon-resource install --novendor --size 48 $ICON_FILE
echo "Installing menu entry..."
xdg-desktop-menu install --novendor $DESKTOP_FILE
echo "Installation successful"