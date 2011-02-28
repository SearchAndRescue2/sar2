#!/bin/bash
BIN_FILE="bin/sar2"

 if [[ ! -f $BIN_FILE ]]
 then
     echo "Search and Rescue II will be now compiled"
     scons
     if [[ $? -ne 0 ]]
     then 
         echo "Search and Rescue II cannot be compiled. Aborting..."
         exit 1
    else
        echo "Search and Rescue II has been compiled successfully."
    fi
fi
export SEARCHANDRESCUE2_DATA="$(pwd)/data"
bin/sar2