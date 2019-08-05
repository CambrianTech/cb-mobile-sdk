#!/bin/bash

function build_opencv_selector() {
    download_dependency "opencv"

    rm -f ${OPENCV_SRC_DIR}/CMakeCache.txt

    cb_logger "Building opencv for $1"

    #http://amritamaz.net/blog/opencv-config

    GLOBAL_OPTIONS="-DWITH_MATLAB=OFF \
                -DWITH_V4L=OFF \
                -DWITH_IPP=OFF \
                -DBUILD_DOCS=OFF \
                -DBUILD_PERF_TESTS=OFF \
                -DBUILD_TESTS=OFF \
                -DBUILD_JASPER=OFF \
                -DBUILD_ZLIB=OFF \
                -DBUILD_PNG=ON \
                -DBUILD_JPEG=OFF \
                -DBUILD_TIFF=OFF \
                -DBUILD_opencv_cnn_3dobj=OFF"

    #protobuf problems: https://github.com/opencv/opencv/pull/9106
    #-Dopencv_dnn_USE_PROTOBUF=ON

    GLOBAL_OPTIONS="$(echo $GLOBAL_OPTIONS|tr -d '\n')"

    #place any libraries to leave out here:
    #IGNORE_LIBS="libzlib"
    IGNORE_LIBS=""

    cb_logger "Building opencv for $1"

    case $1 in
        android)
            android_defs
            build_opencv_android --options "${GLOBAL_OPTIONS}" --ignore "${IGNORE_LIBS}"
            ;;
        ios)
            build_opencv_ios --options "${GLOBAL_OPTIONS}" --ignore "${IGNORE_LIBS}"
            ;;
        osx | linux)
            build_opencv_native "${GLOBAL_OPTIONS}" "${IGNORE_LIBS}"
            ;;
    esac
}

function filter_libs() {
    DIR="$1"

    PATHS=""

    for file in ${DIR}/{.,}*.a; do
        [ -f "$file" ] || continue
        
        filename=${file##*/}

        VALID=true

        if [ -n "$3" ]; then
            IGNORE_LIBS=(${3})
            for LIB_NAME in "${IGNORE_LIBS[@]}"; do
                if [[ $filename == *"${LIB_NAME}."* ]]; then
                    VALID=false
                    echo "Ignoring $filename"
                fi
            done
        fi

        if [ "$VALID" = true ]; then
            PATHS="${PATHS} ${file}"
        fi
    done

    eval "$2='${PATHS}'"
}

function build_opencv_ios() {

    IGNORE_LIBS=""
    CMAKE_OPTIONS=""

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

    OPENCV_SRC_DIR=$CB/third-party/opencv/src/opencv
    OPENCV_CONTRIB_SRC_DIR=$CB/third-party/opencv_contrib/src/opencv_contrib
    BUILD_DIR="${OPENCV_SRC_DIR}/build"

    rm -Rdf $BUILD_DIR/*
    mkdir -p $BUILD_DIR

    INSTALL_DIR=$CB/prebuilts/opencv/opencv-ios

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
                -DCMAKE_INSTALL_CONFIG_NAME=${BUILD_TYPE} \
                -DOPENCV_EXTRA_MODULES_PATH="${OPENCV_CONTRIB_SRC_DIR}/modules" \
                -DCMAKE_INSTALL_PREFIX="${THIS_INSTALL_DIR}" \
                -DAPPLE_FRAMEWORK=ON \
                -DIOS_ARCH=$ARCH \
                -DCMAKE_C_FLAGS=-fembed-bitcode \
                -DCMAKE_CXX_FLAGS=-fembed-bitcode \
                -DENABLE_NEON=$NEON \
                ${CMAKE_OPTIONS} \
                ../..
            
            xcsdk="$(echo $SDK | tr [:upper:] [:lower:])"
            xcodebuild -sdk $xcsdk -project OpenCV.xcodeproj -target=ALL_BUILD -configuration ${BUILD_TYPE} ONLY_ACTIVE_ARCH=NO -ARCHS=$ARCH IPHONEOS_DEPLOYMENT_TARGET=${IPHONEOS_DEPLOYMENT_TARGET} 

            cmake -DBUILD_TYPE=${BUILD_TYPE} -P cmake_install.cmake

            MERGE_PATHS=""

            #merge opencv libraries
            RESULT=""
            filter_libs "${THIS_INSTALL_DIR}/lib" RESULT "${IGNORE_LIBS}"
            MERGE_PATHS="${MERGE_PATHS} ${RESULT}"

            #merge 3rd party libraries
            RESULT=""
            filter_libs "${THIS_BUILD_DIR}/3rdparty/lib/${BUILD_TYPE}" RESULT "${IGNORE_LIBS}"
            MERGE_PATHS="${MERGE_PATHS} ${RESULT}"

            MERGED_LIB="${THIS_INSTALL_DIR}/lib/libopencv_merged.a"

            libtool -static -o ${MERGED_LIB} ${MERGE_PATHS}

            BINARY_PATHS="${BINARY_PATHS} ${MERGED_LIB}"

            cd "${OPENCV_SRC_DIR}"
        done

    done

    LAST_INCLUDE_SRC="${THIS_INSTALL_DIR}/include"
    FINAL_BINARY_PATH="${INSTALL_DIR}/lib/libopencv2.a"

    rm -Rdf $INSTALL_DIR/*
    
    mkdir -p ${INSTALL_DIR}/lib

    mv ${LAST_INCLUDE_SRC} ${INSTALL_DIR}

    lipo -create ${BINARY_PATHS} -o ${FINAL_BINARY_PATH}

}


function build_opencv_android() {

    ARCH=0

    while [[ $# -gt 1 ]]; do
        key="$1"

        case $key in
        	--arch)
        	ARCH="$2"
        	shift
        	;;
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

    if [ $ARCH -eq 0 ]; then
        #echo "ALL ARCHS"
        build_opencv_android --arch 32 --options "${CMAKE_OPTIONS}" --ignore "${IGNORE_LIBS}"
        build_opencv_android --arch 32v5 --options "${CMAKE_OPTIONS}" --ignore "${IGNORE_LIBS}"
        build_opencv_android --arch 64 --options "${CMAKE_OPTIONS}" --ignore "${IGNORE_LIBS}"
        build_opencv_android --arch x86 --options "${CMAKE_OPTIONS}" --ignore "${IGNORE_LIBS}"
        build_opencv_android --arch x86_64 --options "${CMAKE_OPTIONS}" --ignore "${IGNORE_LIBS}"
        exit
    fi

    echo "Building opencv for ${ARCH}"

    NEON=NO
    ANDROID_FORCE_ARM_BUILD="OFF"

    case $ARCH in
        32)
            ANDROID_ABI="armeabi-v7a with NEON"
            NEON=YES
            ANDROID_FORCE_ARM_BUILD="ON"
            ;;
        32v5)
            ANDROID_ABI="armeabi"
            ANDROID_FORCE_ARM_BUILD="ON"
            ;;
        64)
            ANDROID_ABI="arm64-v8a"
            NEON=YES
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

    ### path setup
    OPENCV_SRC_DIR=$CB/third-party/opencv/src/opencv
    OPENCV_CONTRIB_SRC_DIR=$CB/third-party/opencv_contrib/src/opencv_contrib
    BUILD_DIR="${OPENCV_SRC_DIR}/build"
    INSTALL_DIR=$CB/prebuilts/opencv/opencv-android

    echo $BUILD_DIR

    rm -Rdf $BUILD_DIR;
    mkdir $BUILD_DIR

    mkdir -p $INSTALL_DIR

    cd "${BUILD_DIR}"

    #  FIXME: This needs to be exported for the opencv cmake. I should be
    # passable to cmake with the -D argument, but it fails for some
    # reason for now.
    CMAKE_ANDROID_TOOLCHAIN="$CB/third-party/opencv/src/opencv/platforms/android/android.toolchain.cmake"

    cmake -Wno-deprecated \
                -DCMAKE_TOOLCHAIN_FILE="${CMAKE_ANDROID_TOOLCHAIN}" \
                -DANDROID_NDK="$ANDROID_NDK_ROOT" \
                -DANDROID_ABI="${ANDROID_ABI}" \
                -DANDROID_NATIVE_API_LEVEL="${ANDROID_NATIVE_API_LEVEL}" \
                -DANDROID_FORCE_ARM_BUILD="${ANDROID_FORCE_ARM_BUILD}" \
                -DBUILD_ANDROID_EXAMPLES=OFF \
                -WITH_GTK=OFF \
                -WITH_GTK_2_X=OFF \
                -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
                -DOPENCV_EXTRA_MODULES_PATH="${OPENCV_CONTRIB_SRC_DIR}/modules" \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
                -DENABLE_NEON=$NEON \
                -DWITH_CUDA=OFF \
                ${CMAKE_OPTIONS} \
                ..

    make -j8
    make install
}


function build_opencv_native() {
    
    CMAKE_OPTIONS="$1"
    IGNORE_LIBS="$2"

    CMAKE_OPTIONS=$1

    BUILD_DIR="${OPENCV_SRC_DIR}/build"
    INSTALL_DIR=$CB/prebuilts/opencv/opencv-${PLATFORM}

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    cd ${BUILD_DIR}

    cmake -Wno-deprecated \
                -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
                -DOPENCV_EXTRA_MODULES_PATH="${OPENCV_CONTRIB_SRC_DIR}/modules" \
                -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
                -DBUILD_SHARED_LIBS=OFF \
                -DWITH_CUDA=OFF \
                ${CMAKE_OPTIONS} \
                ..
    
    make -j8
    make install
}
