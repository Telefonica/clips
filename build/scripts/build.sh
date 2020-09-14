#!/bin/bash
SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"

echo "*******************************"
echo "* Compile Clips C Source Code *"
echo "*******************************"

cd "${SOURCE_CODE_FOLDER}"
make clean # remove previous compiled object files and executables as clips
make all # gcc -o clips main.o -L. -lclips -lm, a direct way without show logs would be "gcc -o clips -DLINUX *.c -lm"

echo "*********************************"
echo "* Upload executable as artifact *"
echo "*********************************"



# To install, only download clips executable from artifactory and add it $PATH environment variable

