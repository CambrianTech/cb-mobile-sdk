#!/usr/bin/env bash

set -eu
#set -x

if [[ ! -d "${CB}"/install/lib ]]; then
  mkdir -p "${CB}"/install/lib
fi

for BUILD_DIR in "$@"; do
  if [[ ! -d "${BUILD_DIR}" ]]; then
    echo "Build directory $BUILD_DIR does not exist."
    echo "Building it..."
    ct cmake
  fi

  cmake --build "${BUILD_DIR}" -- -j8 VERBOSE=1 
done

