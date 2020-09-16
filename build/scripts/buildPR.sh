#!/bin/bash
SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PACKAGE_NAME="clips"
ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"
FINAL_ARTIFACT_FOLDER="${ROOT_CLIPS_FOLDER}/artifactory"
PACKAGE_TEMP_FOLDER="${ROOT_CLIPS_FOLDER}/dist_temp"
DEBIAN_FOLDER="${PACKAGE_TEMP_FOLDER}/DEBIAN"

ORIGIN_COMPILED_EXECUTABLE_FILE_PATH="${SOURCE_CODE_FOLDER}/clips"
DEBIAN_CONTROL_FILE_PATH="${DEBIAN_FOLDER}/control" # Mandatory file to build Debian package
INSTALLATION_EXECUTABLE_FILE_PATH="${PACKAGE_TEMP_FOLDER}/usr/lib/clips"

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

mkdir "${PACKAGE_TEMP_FOLDER}"

# https://linuxconfig.org/easy-way-to-create-a-debian-package-and-local-package-repository
mkdir "${DEBIAN_FOLDER}"
echo -e "Package: aura-clips
Version: ${CURRENT_LOCAL_VERSION}
Section: libs
Priority: optional
Architecture: ${ARCHITECTURE}
Installed-Size: 1353520
Maintainer: Victor Cordero
Description: Linux compiled fully functional 6.31 version of CLIPS (in November 2019)" > "${DEBIAN_CONTROL_FILE_PATH}"

mkdir -p "${INSTALLATION_EXECUTABLE_FILE_PATH}"
mv "${ORIGIN_COMPILED_EXECUTABLE_FILE_PATH}" "${INSTALLATION_EXECUTABLE_FILE_PATH}/${PACKAGE_NAME}"

version_artifactory_dir="${FINAL_ARTIFACT_FOLDER}/${CURRENT_LOCAL_VERSION}"
mkdir -p "${version_artifactory_dir}"

dpkg-deb --build "${PACKAGE_TEMP_FOLDER}" "${version_artifactory_dir}"
# TODO Change to use CPACK
