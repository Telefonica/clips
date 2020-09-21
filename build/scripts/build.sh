#!/bin/bash

set -e
set -x

SCRIPT_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CLIPS_EXECUTABLE_NAME="clips"
SHARED_LIBRARY_NAME="libclips.so"
STATIC_LIBRARY_NAME="libclips.a"
PACKAGE_CLIPS_NAME="clips"
PACKAGE_LIBCLIPS_NAME="libclips"
PACKAGE_LIBCLIPS_NAME="libclips"
PACKAGE_LIBCLIPSDEV_NAME="libclips-dev"
PACKAGE_CLIPS_DESCRIPTION="Linux compiled fully functional 6.31 version of CLIPS (in November 2019)"
PACKAGE_LIBCLIPS_DESCRIPTION="Shared library with clips object files linked dynamically"
PACKAGE_LIBCLIPSDEV_DESCRIPTION="Develop library with header files and object files included statically"
DEBIAN_FOLDER="DEBIAN"
DEBIAN_CONTROL_FILE_PATH="${DEBIAN_FOLDER}/control" # Mandatory file to build Debian package
INSTALLATION_CLIPS_FILE_PATH="usr/lib/clips"
INSTALLATION_LIBCLIPS_SHARED_LIBRARY_FILE_PATH="usr/lib"
INSTALLATION_LIBCLIPSDEV_STATIC_LIBRARY_FILE_PATH="usr/lib"
INSTALLATION_LIBCLIPSDEV_HEADERS_FILE_PATH="usr/include/clips"

ROOT_CLIPS_FOLDER="${SCRIPT_FOLDER}/../.."
SOURCE_CODE_FOLDER="${ROOT_CLIPS_FOLDER}/branches/63x/core"
FINAL_ARTIFACT_FOLDER="${ROOT_CLIPS_FOLDER}/artifacts"
UNPACKED_TEMP_FOLDER="${ROOT_CLIPS_FOLDER}/unpacked"

CURRENT_LOCAL_VERSION="$(cat "${ROOT_CLIPS_FOLDER}/version.txt")"
ARCHITECTURE=$(dpkg --print-architecture) # Expected amd64

CONTROL_FILE_STRUCTURE="Package: PACKAGE_NAME
Version: ${CURRENT_LOCAL_VERSION}
Section: libs
Priority: optional
Architecture: ${ARCHITECTURE}
Maintainer: Victor Cordero
Description: PACKAGE_DESCRIPTION"

build_deb_package(){

  # package_name: name of package to create
  # package_description: description of package to create
  # data: associative array with installation path as key and data to be installed in that installation path as value

  local package_name=$1
  local package_description=$2
  eval "declare -A data=""${3#*=}"

  mkdir "${UNPACKED_TEMP_FOLDER}/${package_name}"
  mkdir "${UNPACKED_TEMP_FOLDER}/${package_name}/${DEBIAN_FOLDER}"

  # Replace control variables
  control_file="${CONTROL_FILE_STRUCTURE/PACKAGE_NAME/${package_name}}"
  control_file="${control_file/PACKAGE_DESCRIPTION/${package_description}}"

  # Create control file
  echo -e "${control_file}" > "${UNPACKED_TEMP_FOLDER}/${package_name}/${DEBIAN_CONTROL_FILE_PATH}"

  for i in "${!data[@]}"; do
    local installation_path="${UNPACKED_TEMP_FOLDER}/${package_name}/$i"
    mkdir -p "${installation_path}"
    cp ${data[$i]} "${installation_path}" # Don't quote first variable to be able copy array of path in a one command
  done

  dpkg-deb --build "${UNPACKED_TEMP_FOLDER}/${package_name}" "${FINAL_ARTIFACT_FOLDER}"
}

echo "*******************************"
echo "* Compile Clips C Source Code *"
echo "*******************************"

# Generate clips, libclips.so and libclips.a
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

# CLIPS building package
declare -A clips_data=(
  ["${INSTALLATION_CLIPS_FILE_PATH}"]="${SOURCE_CODE_FOLDER}/${CLIPS_EXECUTABLE_NAME}" # Executable
)
build_deb_package "${PACKAGE_CLIPS_NAME}" "${PACKAGE_CLIPS_DESCRIPTION}" "$(declare -p clips_data)"

# libclips building package
declare -A libclips_data=(
  ["${INSTALLATION_LIBCLIPS_SHARED_LIBRARY_FILE_PATH}"]="${SOURCE_CODE_FOLDER}/${SHARED_LIBRARY_NAME}" # Shared
)
build_deb_package "${PACKAGE_LIBCLIPS_NAME}" "${PACKAGE_LIBCLIPS_DESCRIPTION}" "$(declare -p libclips_data)"

# libclips-dev building package
declare -A libclipsdev_data=(
  ["${INSTALLATION_LIBCLIPSDEV_STATIC_LIBRARY_FILE_PATH}"]="${SOURCE_CODE_FOLDER}/${STATIC_LIBRARY_NAME}" # Static
  ["${INSTALLATION_LIBCLIPSDEV_HEADERS_FILE_PATH}"]=$(find "${SOURCE_CODE_FOLDER}" -name "*.h") # Headers
)
build_deb_package "${PACKAGE_LIBCLIPSDEV_NAME}" "${PACKAGE_LIBCLIPSDEV_DESCRIPTION}" "$(declare -p libclipsdev_data)"
