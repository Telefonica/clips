#!/bin/bash
SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PACKAGE_NAME="aura_clips"
ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"
ARTIFACT_FOLDER="${ROOT_CLIPS_FOLDER}/dist"
DEBIAN_FOLDER="${ARTIFACT_FOLDER}/DEBIAN"

ORIGIN_COMPILED_EXECUTABLE_FILE_PATH="${SOURCE_CODE_FOLDER}/clips"
DEBIAN_CONTROL_FILE_PATH="${DEBIAN_FOLDER}/control" # Mandatory file to build Debian package
INSTALLATION_EXECUTABLE_FILE_PATH="${ARTIFACT_FOLDER}/usr/bin/"

CURRENT_LOCAL_VERSION="$(cat "${ROOT_CLIPS_FOLDER}/version.txt")"
ARCHITECTURE=$(dpkg --print-architecture) # Expected amd64

echo "*******************************"
echo "* Compile Clips C Source Code *"
echo "*******************************"

cd "${SOURCE_CODE_FOLDER}"
make clean # remove previous compiled object files and executables as clips
make all # gcc -o clips main.o -L. -lclips -lm, a direct way without show logs would be "gcc -o clips -DLINUX *.c -lm"

# Versioning is performed by manually modification of version.txt file

echo "*********************"
echo "* Generate artefact *"
echo "*********************"

mkdir "${ARTIFACT_FOLDER}"

# https://linuxconfig.org/easy-way-to-create-a-debian-package-and-local-package-repository
mkdir "${DEBIAN_FOLDER}"
echo -e "Package: libclips
Version: ${CURRENT_LOCAL_VERSION}
Section: libs
Priority: optional
Architecture: ${ARCHITECTURE}
Installed-Size: 1353520
Maintainer: Victor Cordero
Description: Linux compiled fully functional 6.31 version of CLIPS (in November 2019)" > "${DEBIAN_CONTROL_FILE_PATH}"

mkdir -p "${INSTALLATION_EXECUTABLE_FILE_PATH}"
mv "${ORIGIN_COMPILED_EXECUTABLE_FILE_PATH}" "${INSTALLATION_EXECUTABLE_FILE_PATH}/${PACKAGE_NAME}-${CURRENT_LOCAL_VERSION}_${ARCHITECTURE}"
dpkg-deb --build "${ARTIFACT_FOLDER}"
# TODO Change to use CPACK
# TODO Where place final artifact

# To install, only download clips executable from artifactory and add it $PATH environment variable

