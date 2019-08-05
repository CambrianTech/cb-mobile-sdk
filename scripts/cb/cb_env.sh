#!/usr/bin/env bash

set -eu
# set -x

unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
  export CBPLATFORM='linux'
elif [[ "$unamestr" == 'Darwin' ]]; then
  export CBPLATFORM='darwin'
else
  export CBPLATFORM='unknown'
fi

export CXX="clang++"
export CXXFLAGS="-std=c++11"

OPENCV_SRC_DIR=$CB/third-party/opencv/src/opencv
OPENCV_CONTRIB_SRC_DIR=$CB/third-party/opencv_contrib/src/opencv_contrib

export CBCORE="${CB}/cambrian-cpp"
export PREBUILTSDIR=$CB/prebuilts
export CBIOS="${CB}/cambrian-ios"
# export CBANDROID="${CB}/cambrian-android"
export CBWEB="${CB}/cambrian-web"
export CBBUILD="${CBCORE}/build"
export PROTOBUF_INCLUDE_DIRS=$PREBUILTSDIR//tensorflow/include/google/protobuf

# Add opencv to shared library path
if [ "${CBPLATFORM}" == "linux" ]; then
  export CBOPENCVSTATICDIR="${CB}/prebuilts/opencv/opencv-linux"
  #    export LD_LIBRARY_PATH=${OPENCVDIR}/lib:$LD_LIBRARY_PATH
elif [ "${CBPLATFORM}" == "darwin" ]; then
  export CBOPENCVSTATICDIR="${CB}/prebuilts/opencv/opencv-osx"
  export FRAMEWORKS="-framework OpenGL -framework GLUT -framework Accelerate -framework Foundation -framework OpenCL"
  #    export DYLD_LIBRARY_PATH=${OPENCVDIR}/lib:$DYLD_LIBRARY_PATH
fi

