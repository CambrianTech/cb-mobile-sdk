#!/usr/bin/env bash


IPHONEOS_DEPLOYMENT_TARGET=8.0 

BUILD_SCRIPT_DIR=$CB/scripts/cb/build_scripts

OPENCV_SRC_DIR="${CB}/third-party/opencv/src/opencv"
BUILD_TYPE=Debug

source cb_common.sh

function cb_builder() {

    case ${3:-debug} in
        release)
            BUILD_TYPE=Release
            cb_logger "Build type: Release"
            ;;
        *)
            BUILD_TYPE=Debug
            cb_logger "Build type: ${BUILD_TYPE}"
            ;;
    esac
    
    case $1 in 
        core)
            source $BUILD_SCRIPT_DIR/libcambrian.sh
            build_core_selector ${@:2}
            ;;

        sdk)
            source $BUILD_SCRIPT_DIR/sdk.sh
            build_sdk_selector ${@:2}
            ;;

        cbclient)
            source $BUILD_SCRIPT_DIR/cbclient.sh
            build_cbclient_selector ${@:2}
            ;;

        opencv)
            source $BUILD_SCRIPT_DIR/libopencv.sh
            build_opencv_selector ${@:2}
            ;;

        bgfx)
            source $BUILD_SCRIPT_DIR/libbgfx.sh
            build_bgfx_selector ${@:2}
            ;;

        tensorflow)
            source $BUILD_SCRIPT_DIR/libtensorflow.sh
            build_tensorflow_selector ${@:2}
            ;;

        keras)
            build_keras
            ;;

        assimp)
            source $BUILD_SCRIPT_DIR/libassimp.sh
            build_assimp_selector ${@:2}
            ;;

        libyuv)
            source $BUILD_SCRIPT_DIR/libyuv.sh
            build_libyuv_selector ${@:2}
            ;;

        libjpeg)
            source $BUILD_SCRIPT_DIR/libjpeg.sh
            build_libjpeg_selector ${@:2}
            ;;

        pcl)
            build_pcl_selector ${@:2}
            ;;

        liblzma)
            source $BUILD_SCRIPT_DIR/liblzma.sh
            build_liblzma_selector ${@:2}
            ;;
            
        bzip2)
            source $BUILD_SCRIPT_DIR/bzip2.sh
            build_bzip2_selector ${@:2}
            ;;

        jsoncpp)
            source $BUILD_SCRIPT_DIR/jsoncpp.sh
            build_jsoncpp_selector ${@:2}
            ;;

        tests)
            build_tests_selector ${@:2}
            ;;

        *)
            cb_logger "Don't know how to build $1"
            exit 1
            ;;
    esac

    return 0
}

function android_defs() {
    export NDK_ROOT=$ANDROID_NDK
    export ANDROID_NDK_ROOT=$ANDROID_NDK
    export ANDROID_NDK_ARM=$ANDROID_NDK_ROOT/toolchains/aarch64-linux-androideabi-4.9/prebuilt/${PLATFORM}
    export ANDROID_NDK_X86=$ANDROID_NDK_ROOT/toolchains/x86_64-4.9/prebuilt/${PLATFORM}
    export ANDROID_NDK_CLANG=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/${PLATFORM}
    export ANDROID_NDK_MIPS=$ANDROID_NDK_ROOT/toolchains/mips64el-linux-android-4.9/prebuilt/${PLATFORM}

    export ANDROID_NATIVE_API_LEVEL="android-21"

    export CC="${ANDROID_NDK_CLANG}/bin/clang"
    export CXX="${ANDROID_NDK_CLANG}/bin/clang++"

    export AR="${ANDROID_NDK_CLANG}/bin/llvm-ar"

    export CMAKE_ANDROID_TOOLCHAIN="${CB_PREBUILTS}/opencv/opencv-android/sdk/native/jni/android.toolchain.cmake"
}

function build_tests_selector() {

    source $BUILD_SCRIPT_DIR/tests.sh

    case $1 in
        android)
            android_defs
            build_tests_android --arch 32
            ;;
        ios)
            build_tests_ios
            ;;
        osx | linux)
            build_tests_native
            ;;
        *)
            cb_logger "Don't know how to build SDK for $1"
            exit 1
        ;;
    esac

}

function build_pcl_selector() {

    download_dependency "pcl"
    download_dependency "eigen"
    download_dependency "flann"
    download_dependency "boost"

    BUILD_DIR="$CB/third-party/pcl/src/pcl/build"
    INSTALL_DIR="${CB_PREBUILTS}/pcl/${PLATFORM}"

    # Under ${PCL_ROOT}/cmake/Modules there is a list of FindXXX.cmake files
    # used to locate dependencies and set their related variables. They have
    # a list of default searchable paths where to look for them. In addition,
    # if pkg-config is available then it is triggered to get hints on their
    # locations. If all of them fail, then we look for a CMake entry or
    # environment variable named **XXX_ROOT** to find headers and libraries.
    # We recommend setting an environment variable since it is independent
    # from CMake and lasts over the changes you can make to your
    # configuration.

    # The available ROOTs you can set are as follow:

    # * **BOOST_ROOT**: for boost libraries with value `C:/Program Files/boost-1.4.6` for instance
    # * **CMINPACK_ROOT**: for cminpack with value `C:/Program Files/CMINPACK 1.1.13` for instance
    # * **QHULL_ROOT**: for qhull with value `C:/Program Files/qhull 6.2.0.1373` for instance
    # * **FLANN_ROOT**: for flann with value `C:/Program Files/flann 1.6.8` for instance
    # * **EIGEN_ROOT**: for eigen with value `C:/Program Files/Eigen 3.0.0` for instance


    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    cd ${BUILD_DIR}
    cmake   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DBOOST_ROOT="${CB}/third-party/boost/src/boost" \
            -DFLANN_ROOT="${CB}/third-party/flann/src/flann" \
            -DEIGEN_ROOT="${CB}/third-party/eigen/src/eigen" \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            ..
    
    make -j8

    # case $1 in
    #     android)
    #         android_defs
    #         source $BUILD_SCRIPT_DIR/bgfx_android.sh
    #         build_bgfx_android
    #         ;;
    #     ios)
    #         source $BUILD_SCRIPT_DIR/bgfx_ios.sh
    #         build_bgfx_ios
    #         ;;
    #     osx)
    #         source $BUILD_SCRIPT_DIR/bgfx_osx.sh
    #         build_bgfx_osx
    #         ;;
    #     linux)
    #         source $BUILD_SCRIPT_DIR/bgfx_linux.sh
    #         build_bgfx_linux
    #         ;;
    #     *)
    #         cb_logger "Don't know how to build SDK for $1"
    #         exit 1
    #     ;;
    # esac

}

