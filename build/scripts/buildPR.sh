#!/bin/bash

set -e
set -x

SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CLIPS_EXECUTABLE_NAME="clips"
SHARED_LIBRARY_NAME="libclips.so"
STATIC_LIBRARY_NAME="libclips.a"
ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"
UNPACKED_TEMP_FOLDER="${ROOT_CLIPS_FOLDER}/unpacked"

CURRENT_LOCAL_VERSION="$(cat "${ROOT_CLIPS_FOLDER}/version.txt")"
ARCHITECTURE=$(dpkg --print-architecture) # Expected amd64

echo "*******************************"
echo "* Compile Clips C Source Code *"
echo "*******************************"

cd "${SOURCE_CODE_FOLDER}"
make clean # remove previous compiled object files and executables as clips
make all # gcc -o clips main.o -L. -lclips -lm, a direct way without show logs would be "gcc -o clips -DLINUX *.c -lm"

mkdir "${UNPACKED_TEMP_FOLDER}"

# CLIPS executable
mv "${SOURCE_CODE_FOLDER}/${CLIPS_EXECUTABLE_NAME}" "${UNPACKED_TEMP_FOLDER}/${CLIPS_EXECUTABLE_NAME}_${CURRENT_LOCAL_VERSION}_${ARCHITECTURE}"

# libclips.so (shared CLIPS dynamic library)
mv "${SOURCE_CODE_FOLDER}/${SHARED_LIBRARY_NAME}" "${UNPACKED_TEMP_FOLDER}/${SHARED_LIBRARY_NAME}_${CURRENT_LOCAL_VERSION}_${ARCHITECTURE}"

# libclisp.a (static CLIPS library)
mv "${SOURCE_CODE_FOLDER}/${STATIC_LIBRARY_NAME}" "${UNPACKED_TEMP_FOLDER}/${STATIC_LIBRARY_NAME}_${CURRENT_LOCAL_VERSION}_${ARCHITECTURE}"
