#!/bin/sh

echo $(pwd)

export LD_LIBRARY_PATH=$(pwd)/libs/

# Added by V.D. pb with 64 bits lib.
# ----------------------------------------
cd $(pwd)/libs/
rm libfmodex.so libfmodexp.so
ln -s libfmodex-4.22.00.so libfmodex.so
ln -s libfmodexp-4.22.00.so libfmodexp.so 
cd ..

# Added by V.D. FOR V4L under Ubuntu
# ----------------------------------------
export LD_PRELOAD=/usr/lib/libv4l/v4l1compat.so


./Community_Core_Vision
