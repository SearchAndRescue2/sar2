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
tar -zcf $NAME.tar.gz --exclude-backups $NAME
RES=$?
rm -rf $NAME
if [[ $RES -eq 0 ]]
then
    echo "Created $NAME.tar.gz successfully."
else
    echo "Failed to create sar2 archive."
fi