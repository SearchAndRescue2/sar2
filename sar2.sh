#!/bin/bash

BIN_FILE="bin/sar2"
# Different shells set $0 differently (bash uses relative and fish absolute)
# so we find what the absolute path to this script is.
# Also SAR2_DATA must be an absolute path
ABS_PATH=$(realpath $0)
PATH_TO_GAME=$(dirname $ABS_PATH) # absolute path to game folder
export SAR2_DATA="$PATH_TO_GAME/data"

if [[ ! -f "$PATH_TO_GAME/$BIN_FILE" ]]
 then
     echo "Search and Rescue II will now be compiled"
     pushd "$PATH_TO_GAME"
     scons
     if [[ $? -ne 0 ]]
     then 
         echo "Search and Rescue II cannot be compiled. Aborting..."
         exit 1
     else
         echo "Search and Rescue II has been compiled successfully."
     fi
     popd
fi

"$PATH_TO_GAME/bin/sar2" $@
