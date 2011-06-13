#!/bin/bash
BIN_FILE="bin/sar2"
PATH_TO_GAME=$(dirname $0) #make it possible to execute this from any location

if [[ ! -f "$PATH_TO_GAME/$BIN_FILE" ]]
 then
     echo "Search and Rescue II will now be compiled"
     cd "$PATH_TO_GAME"
     scons
     if [[ $? -ne 0 ]]
     then 
         echo "Search and Rescue II cannot be compiled. Aborting..."
         exit 1
     else
         echo "Search and Rescue II has been compiled successfully."
     fi
     cd -
fi
# export full path of the data location
export SEARCHANDRESCUE2_DATA="$(pwd)/$PATH_TO_GAME/data"
"$PATH_TO_GAME/bin/sar2" $@
