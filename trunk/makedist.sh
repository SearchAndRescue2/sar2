#!/bin/bash

# Prepares Sar2 for distribution

VERSION_STRING=$1

if [ -z $VERSION_STRING ]
then
    echo "Usage ./makedist.sh VERSION-STRING"
    exit 1
fi

echo "Cleaning up build files..."
scons -c &>/dev/null
echo "Packaging the game..."
tar -zcf "sar2-$VERSION_STRING.tar.gz" --exclude ".*" --exclude-backups *
if [[ $? -eq 0 ]]
then
    echo "Created sar2-$VERSION_STRING.tar.gz successfully."
else
    echo "Failed to create sar2 archive."
fi