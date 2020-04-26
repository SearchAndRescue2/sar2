#!/bin/bash

# Prepares Sar2 for distribution
set -x

VERSION_STRING=$1
NAME="sar2-${VERSION_STRING}"

if [ -z $VERSION_STRING ]
then
    echo "Usage ./makedist.sh VERSION-STRING"
    exit 1
fi

echo "Cleaning up build files..."
scons -c &>/dev/null
rm -f "$NAME"
echo "Packaging the game..."
tar -jcf "${NAME}.tar.bz2" --exclude-backups --exclude-vcs --exclude-vcs-ignores --{owner,group}=root --transform "s/\./$NAME/" .
RES=$?
if [[ $RES -eq 0 ]]
then
    echo "Created $NAME.tar.bz2 successfully."
else
    echo "Failed to create sar2 archive."
fi
