#!/usr/bin/env bash

function build_assimp_selector() {

    cb_logger "Building assimp for $1 (${BUILD_TYPE})"

    download_dependency "assimp"

    case $1 in
        android)
            build_assimp_android
            ;;
        ios)
            build_assimp_ios
            ;;
        osx | linux)
            build_assimp_native
            ;;
        *)
            cb_logger "Don't know how to build assimp for $1"
            exit 1
    esac

}

function join { local IFS="$1"; shift; echo "$*"; }


function build_assimp_native() {

    BUILD_DIR="./lib/${PLATFORM}"
    INSTALL_DIR=$CB/prebuilts/assimp/${PLATFORM}

    rm -Rf ${INSTALL_DIR}

    cd $CB/third-party/assimp/src/assimp

    rm -f CMakeCache.txt

    rm -Rf ${BUILD_DIR}
    mkdir ${BUILD_DIR}

    cd ${BUILD_DIR}

    cmake   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DBUILD_SHARED_LIBS=OFF \
            -DASSIMP_BUILD_TESTS=OFF \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            ../..

    make clean
    make -j8
    make install
    
}

function build_assimp_ios() {
    cd $CB/third-party/assimp/src/assimp/port/iOS

    BUILD_DIR="./lib/iOS"
    INSTALL_DIR=$CB/prebuilts/assimp/ios

    IOS_SDK_VERSION=
    IOS_SDK_TARGET=6.0
    #(iPhoneOS iPhoneSimulator) -- determined from arch
    IOS_SDK_DEVICE=

    XCODE_ROOT_DIR=/Applications/Xcode.app/Contents
    TOOLCHAIN=$XCODE_ROOT_DIR//Developer/Toolchains/XcodeDefault.xctoolchain

    BUILD_ARCHS_DEVICE="armv7 armv7s arm64"
    BUILD_ARCHS_SIMULATOR="i386 x86_64"
    BUILD_ARCHS_ALL=(armv7 armv7s arm64 i386 x86_64)

    CPP_DEV_TARGET_LIST=(miphoneos-version-min mios-simulator-version-min)
    CPP_DEV_TARGET=
    CPP_STD_LIB_LIST=(libc++ libstdc++)
    CPP_STD_LIB=
    CPP_STD_LIST=(c++11 c++14)
    CPP_STD=

    echo "[!] $0 - assimp iOS build script"

    CPP_STD_LIB=${CPP_STD_LIB_LIST[0]}
    CPP_STD=${CPP_STD_LIST[0]}
    DEPLOY_ARCHS=${BUILD_ARCHS_ALL[*]}
    DEPLOY_FAT=1

    for i in "$@"; do
        case $i in
        -s=*|--std=*)
            CPP_STD=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
            echo "[!] Selecting c++ standard: $CPP_STD"
        ;;
        -l=*|--stdlib=*)
            CPP_STD_LIB=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
            echo "[!] Selecting c++ std lib: $CPP_STD_LIB"
        ;;
        -a=*|--archs=*)
            DEPLOY_ARCHS=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
            echo "[!] Selecting architectures: $DEPLOY_ARCHS"
        ;;
        -n|--no-fat)
            DEPLOY_FAT=0
            echo "[!] Fat binary will not be created."
        ;;
        -h|--help)
            echo " - don't build fat library (--no-fat)."
            echo " - supported architectures (--archs):  $(echo $(join , ${BUILD_ARCHS_ALL[*]}) | sed 's/,/, /g')"
            echo " - supported C++ STD libs (--stdlib): $(echo $(join , ${CPP_STD_LIB_LIST[*]}) | sed 's/,/, /g')"
            echo " - supported C++ standards (--std): $(echo $(join , ${CPP_STD_LIST[*]}) | sed 's/,/, /g')"
            exit
        ;;
        *)
        ;;
        esac
    done

    cd ../../

    rm -rf $BUILD_DIR

    for ARCH_TARGET in $DEPLOY_ARCHS; do
        mkdir -p $BUILD_DIR/$ARCH_TARGET
        build_assimp_ios_arch $ARCH_TARGET
    done

    rm -Rf ${INSTALL_DIR}

    mkdir -p ${INSTALL_DIR}/include
    mkdir -p ${INSTALL_DIR}/lib

    cp -R $CB/third-party/assimp/src/assimp/include/* ${INSTALL_DIR}/include

    FINAL_BINARY_PATH=${INSTALL_DIR}/lib/libassimp.a

    BINARY_PATHS=

    if [[ "$DEPLOY_FAT" -eq 1 ]]; then
        echo '[+] Creating fat libassimp assimp binary ...'
        for ARCH_TARGET in $DEPLOY_ARCHS; do
            BINARY_PATHS="${BINARY_PATHS} $BUILD_DIR/$ARCH_TARGET/libassimp.a"
        done

        echo "[!] Done! The fat binary can be found at $FINAL_BINARY_PATH"
    fi

    lipo -create ${BINARY_PATHS} -o ${FINAL_BINARY_PATH}
}

build_assimp_ios_arch()
{
    IOS_SDK_DEVICE=iPhoneOS
    CPP_DEV_TARGET=${CPP_DEV_TARGET_LIST[0]}

    if [[ "$BUILD_ARCHS_SIMULATOR" =~ "$1" ]]
    then
        echo '[!] Target SDK set to SIMULATOR.'
        IOS_SDK_DEVICE=iPhoneSimulator
        CPP_DEV_TARGET=${CPP_DEV_TARGET_LIST[1]}
    else
        echo '[!] Target SDK set to DEVICE.'
    fi

    unset DEVROOT SDKROOT CFLAGS LDFLAGS CPPFLAGS CXXFLAGS

    export DEVROOT=$XCODE_ROOT_DIR/Developer/Platforms/$IOS_SDK_DEVICE.platform/Developer
    export SDKROOT=$DEVROOT/SDKs/$IOS_SDK_DEVICE$IOS_SDK_VERSION.sdk
    export CFLAGS="-arch $1 -pipe -no-cpp-precomp -stdlib=$CPP_STD_LIB -fembed-bitcode -isysroot $SDKROOT -$CPP_DEV_TARGET=$IOS_SDK_TARGET -I$SDKROOT/usr/include/"
    export LDFLAGS="-L$SDKROOT/usr/lib/"
    export CPPFLAGS=$CFLAGS
    export CXXFLAGS="$CFLAGS -std=$CPP_STD"

    rm -f CMakeCache.txt

    cmake  -G 'Unix Makefiles' -DCMAKE_TOOLCHAIN_FILE=./port/iOS/IPHONEOS_$(echo $1 | tr '[:lower:]' '[:upper:]')_TOOLCHAIN.cmake -DENABLE_BOOST_WORKAROUND=ON -DBUILD_SHARED_LIBS=OFF

    echo "[!] Building $1 library"

    $XCODE_ROOT_DIR/Developer/usr/bin/make clean
    $XCODE_ROOT_DIR/Developer/usr/bin/make assimp -j 8 -l

    echo "[!] Merging libassimp and libIrrXML"

    libtool -static -o $BUILD_DIR/$1/libassimp.a ./lib/libassimp.a ./lib/libIrrXML.a

    echo "[!] Moving built library into: $BUILD_DIR/$1/"
}


function build_assimp_android() {
    android_defs

    BUILD_DIR="./lib/android"
    INSTALL_DIR=$CB/prebuilts/assimp/android

    rm -Rf ${INSTALL_DIR}

    cd $CB/third-party/assimp/src/assimp

    rm -f CMakeCache.txt

    rm -Rf ${BUILD_DIR}
    mkdir ${BUILD_DIR}

    cd ${BUILD_DIR}

    cmake   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            -DBUILD_SHARED_LIBS=OFF \
            -DASSIMP_ANDROID_JNIIOSYSTEM=ON \
            -DASSIMP_BUILD_TESTS=OFF \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_ANDROID_TOOLCHAIN}" \
            ../..

    make clean
    make -j8
    make install
 
    cp contrib/irrXML/libIrrXML.a "${INSTALL_DIR}/lib"
}

