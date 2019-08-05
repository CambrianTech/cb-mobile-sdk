#!/usr/bin/env bash

function build_core_selector() {
    cb_logger "Building core for $1 (${BUILD_TYPE})"
    download_dependency eigen

    case $1 in
        android)
            build_core_android --arch 32
            ;;
        ios)
            build_core_ios
            ;;
        osx | linux)
            build_core_native
            ;;
        *)
            cb_logger "Don't know how to build core for $1"
            exit 1
    esac
}

function build_core_ios() {
    echo "Nothing to do here"
}

function build_core_native() {
    
    BUILD_NAME=${PLATFORM}
    BUILD_DIR="${CB}/build/CambrianAR/${PLATFORM}"
    INSTALL_DIR="${CB}/prebuilts/CambrianAR/${PLATFORM}"

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    if [ "${BUILD_TYPE}" == "Debug" ]; then
        CMAKE_FLAGS="-DFLOORING_ENABLED=1 -DDO_WRITING=1 -DDO_LOGGING=1"
    else
        CMAKE_FLAGS="-DFLOORING_ENABLED=1 -DDO_WRITING=1 -DDO_LOGGING=0"
    fi

    cd ${BUILD_DIR}
    cmake   -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            ${CMAKE_FLAGS} \
            -DCB_ROOT="${CB}" \
            "${CB_CPP}"
    
    make clean
    make -j8
    make install
}

function build_core_android() {

    android_defs
    
    if [ $# -eq 0 ]; then
        build_core_android --arch 32
        build_core_android --arch 64
        build_core_android --arch x86
        build_core_android --arch x86_64
        exit
    fi

    while [[ $# -gt 1 ]]; do
        key="$1"

        case $key in
            --arch)
            ARCH="$2"
            shift
            ;;
            *) ;;
        esac
        shift
    done

    ANDROID_FORCE_ARM_BUILD="OFF"

    case $ARCH in
        32)
            ANDROID_ABI="armeabi-v7a with NEON"
            ANDROID_FORCE_ARM_BUILD="ON"
            ;;
        64)
            ANDROID_ABI="arm64-v8a"
            ;;
        x86)
            ANDROID_ABI="x86"
            ;;
        x86_64)
            ANDROID_ABI="x86_64"
            ;;
        *)
            cb_logger "Dont know how to build core for android $1"
            exit 1
            ;;
    esac

    AR_TOOL="${ANDROID_NDK_CLANG}/bin/llvm-ar"

    set -- $ANDROID_ABI
    ABI=$1

    echo "Building for arch ${ABI}"

    PLATFORM=android
    BUILD_DIR="$CB/build/${PLATFORM}"
    INSTALL_DIR="${CB}/prebuilts/CambrianAR/${PLATFORM}/${ABI}"

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    if [ "${BUILD_TYPE}" == "Debug" ]; then
        CMAKE_FLAGS="-DFLOORING_ENABLED=1 -DDO_WRITING=1 -DDO_LOGGING=1"
    else
        CMAKE_FLAGS="-DFLOORING_ENABLED=1 -DDO_WRITING=1 -DDO_LOGGING=0"
    fi

    # option(FLOORING_ENABLED "Enable Flooring" OFF)
    # option(DEBUG "Enable debug mode" OFF)
    # option(DO_LOGGING "Enable debug mode" OFF)
    # option(DO_WRITING "Enable debug mode" ON)

    cd ${BUILD_DIR}
    cmake   -DCMAKE_TOOLCHAIN_FILE="${CMAKE_ANDROID_TOOLCHAIN}" \
            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -DUSE_BGFX=YES \
            -DANDROID_NDK="${ANDROID_NDK_ROOT}" \
            -DANDROID_ABI="${ANDROID_ABI}" \
            -DANDROID_NATIVE_API_LEVEL="${ANDROID_NATIVE_API_LEVEL}" \
            -DANDROID_FORCE_ARM_BUILD="${ANDROID_FORCE_ARM_BUILD}" \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            -DANDROID_STL="gnustl_static" \
            -DANDROID_GOLD_LINKER=ON \
            -DANDROID_STL_FORCE_FEATURES=ON \
            -DANDROID_NO_UNDEFINED=ON \
            -DANDROID_SO_UNDEFINED=ON \
            -DANDROID_FUNCTION_LEVEL_LINKING=ON \
            -DANDROID_GOLD_LINKER=ON \
            -DANDROID_NOEXECSTACK=ON \
            -DANDROID_RELRO=ON \
            ${CMAKE_FLAGS} \
            -DCB_ROOT="${CB}" \
            "${CB_CPP}"

    
    make clean
    make -j8
    make install

    #combine all libraries
    # ${AR_TOOL} xv ${INSTALL_DIR}/lib/libcambrian.a
    # ${AR_TOOL} xv ${INSTALL_DIR}/lib/libminizip.a

    # ${AR_TOOL} xv ${CB}/prebuilts/tensorflow/lib/android/${ABI}/libtensorflow-core.a
    # ${AR_TOOL} xv ${CB}/prebuilts/tensorflow/lib/android/${ABI}/libprotobuf.a

    # #combine
    # ${AR_TOOL} rc libmegalib.a *.o

    # rm ${INSTALL_DIR}/lib/*

    # mv libmegalib.a ${INSTALL_DIR}/lib/libcambrian.a
    
}
