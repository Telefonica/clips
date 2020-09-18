#!/bin/bash

set -e
set -x

SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CLIPS_EXECUTABLE_NAME="clips"
SHARED_LIBRARY_NAME="libclips.so"
STATIC_LIBRARY_NAME="libclips.a"
PACKAGE_CLIPS_NAME="clips"
PACKAGE_LIBCLIPS_NAME="libclips"
PACKAGE_LIBCLIPSDEV_NAME="libclips-dev"
DEBIAN_FOLDER="DEBIAN"
DEBIAN_CONTROL_FILE_PATH="${DEBIAN_FOLDER}/control" # Mandatory file to build Debian package
INSTALLATION_CLIPS_FILE_PATH="usr/lib/clips"

ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"
FINAL_ARTIFACT_FOLDER="${ROOT_CLIPS_FOLDER}/artifacts"
UNPACKED_TEMP_FOLDER="${ROOT_CLIPS_FOLDER}/unpacked"

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
echo "* Generate artefacts *"
echo "*********************"

# https://linuxconfig.org/easy-way-to-create-a-debian-package-and-local-package-repository
mkdir "${UNPACKED_TEMP_FOLDER}"
mkdir "${FINAL_ARTIFACT_FOLDER}"

build_deb_package(){

  # package: name of package to create
  # data: associative array with installation path as key and data to be installed in that installation path as value

  local package_name=$1
  eval "declare -A data=""${2#*=}"
  echo ${package_name}
  mkdir "${UNPACKED_TEMP_FOLDER}/${package_name}"

  # Generate CONTROL info file
  mkdir "${UNPACKED_TEMP_FOLDER}/${package_name}/${DEBIAN_FOLDER}"
  # TODO Control is tabulated. printf?
  echo -e "Package: ${package_name}
  Version: ${CURRENT_LOCAL_VERSION}
  Section: libs
  Priority: optional
  Architecture: ${ARCHITECTURE}
  Maintainer: Victor Cordero
  Description: Linux compiled fully functional 6.31 version of CLIPS (in November 2019)" > "${UNPACKED_TEMP_FOLDER}/${package_name}/${DEBIAN_CONTROL_FILE_PATH}"

  for i in "${!data[@]}"; do
    local installation_path="${UNPACKED_TEMP_FOLDER}/${package_name}/$i"
    mkdir -p "${installation_path}"
    cp "${data[$i]}" "${installation_path}"
  done

  dpkg-deb --build "${UNPACKED_TEMP_FOLDER}/${package_name}" "${FINAL_ARTIFACT_FOLDER}"
}

# CLIPS building package
declare -A clips_data=(
  ["${INSTALLATION_CLIPS_FILE_PATH}"]="${SOURCE_CODE_FOLDER}/${CLIPS_EXECUTABLE_NAME}"
)
build_deb_package ${PACKAGE_CLIPS_NAME} "$(declare -p clips_data)"

# libclips building package TODO
# build_deb_package ${PACKAGE_LIBCLIPS_NAME}

# libclips-dev building package

# Generate executable CLIPS package

#mkdir "${UNPACKED_TEMP_FOLDER}/${PACKAGE_CLIPS_NAME}"
#mkdir "${UNPACKED_TEMP_FOLDER}/${PACKAGE_CLIPS_NAME}/${DEBIAN_FOLDER}"
#
#echo -e "Package: ${PACKAGE_CLIPS_NAME}
#Version: ${CURRENT_LOCAL_VERSION}
#Section: libs
#Priority: optional
#Architecture: ${ARCHITECTURE}
#Installed-Size: 1200928
#Maintainer: Victor Cordero
#Description: Linux compiled fully functional 6.31 version of CLIPS (in November 2019)" > "${UNPACKED_TEMP_FOLDER}/${PACKAGE_CLIPS_NAME}/${DEBIAN_CONTROL_FILE_PATH}"
#
#mkdir -p "${UNPACKED_TEMP_FOLDER}/${PACKAGE_CLIPS_NAME}/${INSTALLATION_CLIPS_FILE_PATH}"
#mv "${SOURCE_CODE_FOLDER}/${CLIPS_EXECUTABLE_NAME}" "${UNPACKED_TEMP_FOLDER}/${PACKAGE_CLIPS_NAME}/${INSTALLATION_CLIPS_FILE_PATH}"
#
#dpkg-deb --build "${UNPACKED_TEMP_FOLDER}/${PACKAGE_CLIPS_NAME}" "${FINAL_ARTIFACT_FOLDER}"
