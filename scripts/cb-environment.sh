case "$OSTYPE" in
  linux*)   export CB_PLATFORM="linux" ;;
  darwin*)  export CB_PLATFORM='darwin' ;;
  *)        export CB_PLATFORM='unknown' ;;
esac

export CB_CPP="${CB}/cb-cpp/src/cb-cpp"
export CB_IOS="${CB}/cb-ios/src/cb-ios"
export CB_WEB="${CB}/cb-web/src/cb-web"
export CB_PREBUILTS="${CB}/prebuilts"
export CB_BUILD="${CBCORE}/build"

# add completion to path
source $CB/scripts/cb/cb-completion.bash

export PATH="$CB/scripts/cb:$PATH"
export C="clang"
export CXX="clang++"
export CXXFLAGS="-std=c++11"

# Add opencv to shared library path
if [ "${CB_PLATFORM}" = "linux" ]; then
    export CBOPENCVSTATICDIR="${CB_PREBUILTS}/opencv/opencv-linux"
elif [ "${CB_PLATFORM}" = "darwin" ]; then
    export CBOPENCVSTATICDIR="${CB_PREBUILTS}/opencv/opencv-osx"
    export FRAMEWORKS="-framework OpenGL -framework GLUT -framework Accelerate -framework Foundation -framework OpenCL"
fi

