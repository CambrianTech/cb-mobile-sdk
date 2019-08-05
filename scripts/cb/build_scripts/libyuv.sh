#!/bin/bash

function build_libyuv_selector() {

    case $1 in
        android)
            android_defs
            build_libyuv_android
            ;;
        ios)
            build_libyuv_ios
            ;;
        osx | linux)
            build_libyuv_native
            ;;
        *)
            cb_logger "Don't know how to build SDK for $1"
            exit 1
        ;;
    esac

}

function build_libyuv_android() {

    if [ $# -eq 0 ]; then
        build_libyuv_android --arch 32
        build_libyuv_android --arch 64
        build_libyuv_android --arch x86
        build_libyuv_android --arch x86_64
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

    ### path setup
    BUILD_DIR="$CB/third-party/libyuv/src/libyuv/build/"
    INSTALL_DIR=$CB/prebuilts/libyuv/android/${ABI}

    echo $BUILD_DIR

    rm -Rdf $BUILD_DIR;
    mkdir -p $BUILD_DIR

    rm -Rdf $INSTALL_DIR;
    mkdir -p $INSTALL_DIR

    cd "${BUILD_DIR}"

    cmake -Wno-deprecated \
                -DCMAKE_TOOLCHAIN_FILE="${CMAKE_ANDROID_TOOLCHAIN}" \
                -DANDROID_NDK="$ANDROID_NDK_ROOT" \
                -DANDROID_ABI="${ANDROID_ABI}" \
                -DANDROID_NATIVE_API_LEVEL="${ANDROID_NATIVE_API_LEVEL}" \
                -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
                ..

    make -j8
    make install
}

function build_libyuv_ios() {

    while [[ $# -gt 1 ]]; do
        key="$1"

        case $key in
            --options)
            CMAKE_OPTIONS="$2"
            shift
            ;;
            --ignore)
            IGNORE_LIBS="$2"
            shift
            ;;
            *) ;;
        esac
        shift
    done

    SRC_DIR="${CB}/third-party/libyuv/src/libyuv"
    BUILD_DIR="${SRC_DIR}/build"

    rm -Rdf $BUILD_DIR/*
    mkdir -p $BUILD_DIR

    INSTALL_DIR=$CB/prebuilts/libyuv

    rm -Rdf $INSTALL_DIR/*

    IPHONE_SDKS=( iPhoneOS iPhoneSimulator )

    BINARY_PATHS=""

    for SDK in "${IPHONE_SDKS[@]}"; do

        if [[ $SDK == *"iPhoneSimulator"* ]]; then
            NEON=NO
            ARCHS=( x86_64 i386 )
        else
            NEON=YES
            ARCHS=( arm64 armv7 armv7s )
        fi

        for ARCH in "${ARCHS[@]}"; do

            THIS_BUILD_DIR="${BUILD_DIR}/${ARCH}"
            rm -Rdf ${THIS_BUILD_DIR}
            mkdir -p ${THIS_BUILD_DIR}

            THIS_INSTALL_DIR="${THIS_BUILD_DIR}/install"
            mkdir -p ${THIS_INSTALL_DIR}

            cd ${THIS_BUILD_DIR}

            cmake -GXcode \
                -DCMAKE_TOOLCHAIN_FILE="${OPENCV_SRC_DIR}/platforms/ios/cmake/Toolchains/Toolchain-${SDK}_Xcode.cmake" \
                -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
                -DCMAKE_INSTALL_PREFIX="${THIS_INSTALL_DIR}" \
                -DAPPLE_FRAMEWORK=ON \
                -DIOS_ARCH=$ARCH \
                -DCMAKE_C_FLAGS=-fembed-bitcode \
                -DCMAKE_CXX_FLAGS=-fembed-bitcode \
                ../..
            
            xcsdk="$(echo $SDK | tr [:upper:] [:lower:])"
            xcodebuild -sdk $xcsdk -project YUV.xcodeproj -target=ALL_BUILD -configuration ${BUILD_TYPE} ONLY_ACTIVE_ARCH=NO -ARCHS=$ARCH IPHONEOS_DEPLOYMENT_TARGET=${IPHONEOS_DEPLOYMENT_TARGET} 

            #cmake -P cmake_install.cmake

            THIS_LIB="${THIS_BUILD_DIR}/${BUILD_TYPE}-${SDK}/libyuv.a"
            BINARY_PATHS="${BINARY_PATHS} ${THIS_LIB}"
        done

    done

    LAST_INCLUDE_SRC="${THIS_INSTALL_DIR}/include"
    FINAL_BINARY_PATH="${INSTALL_DIR}/lib/ios"

    mkdir -p ${FINAL_BINARY_PATH}

    cp -R ${SRC_DIR}/include ${INSTALL_DIR}/include

    lipo -create ${BINARY_PATHS} -o ${FINAL_BINARY_PATH}/libyuv.a
}