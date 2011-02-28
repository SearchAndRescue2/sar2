#!/bin/bash
BIN_FILE="bin/sar2"
PATH_TO_GAME=$(dirname $0) #make it possible to execute this from any location

 if [[ ! -f "$PATH_TO_GAME/$BIN_FILE" ]]
 then
     echo "Search and Rescue II will be now compiled"
     "$PATH_TO_GAME/scons"
     if [[ $? -ne 0 ]]
     then 
         echo "Search and Rescue II cannot be compiled. Aborting..."
         exit 1
    else
        echo "Search and Rescue II has been compiled successfully."
    fi
fi
# export full path of the data location
export SEARCHANDRESCUE2_DATA="$(pwd)/$(dirname $0)/data"
"$PATH_TO_GAME/bin/sar2"