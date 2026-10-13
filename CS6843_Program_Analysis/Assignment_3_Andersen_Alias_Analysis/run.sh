#!/usr/bin/env bash
if [ $# -ne 1 ]
then
    echo "Specify the C/LLVM IR filename to analyse"
    echo "Usage: ./run.sh <filename.c/filename.ll>"
    exit
fi
filename=$1

if [ `echo $filename | grep "\.c$"` ]
then
    echo "Generating LLVM bitcode for $filename"
    llfilename=`echo $filename | sed 's/\.c$/\.ll/'`
    clang -S -emit-llvm -g -O0 $filename -o $llfilename
    if [ $? -ne 0 ]
    then
        echo "Error in compiling $filename. Exiting"
        exit
    fi
else
    llfilename=$filename
fi

# Build the analysis
./build.sh || { echo "Building analysis failed. Exiting."; exit; }

echo "Running the analysis on $llfilename"
opt -load-pass-plugin ./build/analysis/a3Pass.so -passes='pointer-analysis' -disable-output $llfilename 

# If you're using Mac, instead of .so use .dylib
