#!/bin/bash

function build_tests_ios() {
    BUILD_DIR="$CB/build/tests/ios"
    INSTALL_DIR="${CB}/prebuilts/cambrian-tests/ios"

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    cd ${BUILD_DIR}

    echo "Not done, copy how libyuv is done."
    
    
}

function build_tests_android() {

    android_defs

    if [ $# -eq 0 ]; then
        build_tests_android --arch 32
        build_tests_android --arch 64
        build_tests_android --arch x86
        build_tests_android --arch x86_64
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
            cb_logger "Dont know how to build opencv for android $1"
            exit 1
            ;;
    esac

    set -- $ANDROID_ABI
    ABI=$1

    echo "Building for arch ${ABI}"

    PLATFORM=android
    BUILD_DIR="$CB/build/tests/${PLATFORM}"
    INSTALL_DIR="${CB}/prebuilts/cambrian-tests/${PLATFORM}/${ABI}"

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    cd ${BUILD_DIR}
    cmake   -DCMAKE_TOOLCHAIN_FILE="${CMAKE_ANDROID_TOOLCHAIN}" \
            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            -DANDROID_NDK="${ANDROID_NDK_ROOT}" \
            -DANDROID_ABI="${ANDROID_ABI}" \
            -DANDROID_NATIVE_API_LEVEL="${ANDROID_NATIVE_API_LEVEL}" \
            -DANDROID_FORCE_ARM_BUILD="${ANDROID_FORCE_ARM_BUILD}" \
            -DANDROID_STL="gnustl_static" \
            -DANDROID_STL_FORCE_FEATURES=ON \
            -DANDROID_NO_UNDEFINED=ON \
            -DANDROID_SO_UNDEFINED=ON \
            -DANDROID_FUNCTION_LEVEL_LINKING=ON \
            -DANDROID_GOLD_LINKER=ON \
            -DANDROID_NOEXECSTACK=ON \
            -DANDROID_RELRO=ON \
            -DCB_ROOT="${CB}" \
            "${CB}/cambrian-tests"
    
    make clean
    make -j8
    make install
}

function build_tests_native() {
    BUILD_DIR="$CB/build/tests/${PLATFORM}"
    INSTALL_DIR="${CB}/prebuilts/cambrian-tests/${PLATFORM}"

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    cd ${BUILD_DIR}
    cmake   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            -DCB_ROOT=${CB} \
            ${CB}/cambrian-tests
    
    make clean
    make -j8
    make install
}
