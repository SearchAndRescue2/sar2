#!/bin/bash

# Prepares Sar2 for distribution

VERSION_STRING=$1
NAME="sar2-$VERSION_STRING"

if [ -z $VERSION_STRING ]
then
    echo "Usage ./makedist.sh VERSION-STRING"
    exit 1
fi

DISTFILES=$(ls)

echo "Cleaning up build files..."
scons -c &>/dev/null
echo "Packaging the game..."
mkdir $NAME
cp -l -r $DISTFILES $NAME/
tar -jcf $NAME.tar.bz2 --exclude-backups $NAME --exclude ".svn"
RES=$?
rm -rf $NAME
if [[ $RES -eq 0 ]]
then
    echo "Created $NAME.tar.bz2 successfully."
else
    echo "Failed to create sar2 archive."
fi
