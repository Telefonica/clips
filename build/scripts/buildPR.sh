#!/bin/bash
SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PACKAGE_NAME="clips"
ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"
UNPACKED_TEMP_FOLDER="${ROOT_CLIPS_FOLDER}/unpacked"

ORIGIN_COMPILED_EXECUTABLE_FILE_PATH="${SOURCE_CODE_FOLDER}/clips"

CURRENT_LOCAL_VERSION="$(cat "${ROOT_CLIPS_FOLDER}/version.txt")"
ARCHITECTURE=$(dpkg --print-architecture) # Expected amd64

echo "*******************************"
echo "* Compile Clips C Source Code *"
echo "*******************************"

cd "${SOURCE_CODE_FOLDER}"
make clean # remove previous compiled object files and executables as clips
make all # gcc -o clips main.o -L. -lclips -lm, a direct way without show logs would be "gcc -o clips -DLINUX *.c -lm"

# Versioning is performed by manually modification of version.txt file
mkdir "${UNPACKED_TEMP_FOLDER}"
mv "${ORIGIN_COMPILED_EXECUTABLE_FILE_PATH}" "${UNPACKED_TEMP_FOLDER}/${PACKAGE_NAME}_${CURRENT_LOCAL_VERSION}_${ARCHITECTURE}"