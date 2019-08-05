#!/usr/bin/env bash

function build_tensorflow_selector() {

    download_dependency "tensorflow"

    cd $CB/third-party/tensorflow/src/tensorflow

    make -f tensorflow/contrib/makefile/Makefile clean
    #rm -rf tensorflow/contrib/makefile/downloads

    bash tensorflow/contrib/makefile/download_dependencies.sh

    # Pull down the required versions of the frameworks we need.

    install_tool autoconf
    install_tool automake

    #we will strip them ourselves
    export ANDROID_TYPES="-D__ANDROID_TYPES_FULL__"

    cb_logger "Building tensorflow for $1 (${BUILD_TYPE})"

    case $1 in
        android)
            #android_defs
            build_tensorflow_android
            ;;
        ios)
            build_tensorflow_ios
            ;;
        osx)
            install_tool unzip
            install_tool libtool

            build_tensorflow_native
            ;;
        linux)
            install_tool unzip
            install_tool libtool
            install_tool libz-dev

            build_tensorflow_native
            ;;
        *)
            cb_logger "Don't know how to build tensorflow for $1"
            exit 1
    esac

    exit

    #copy includes
    INCLUDE_DIR=$CB/prebuilts/tensorflow/include
    mkdir -p ${INCLUDE_DIR}

    #not working:
    # cp -rf tensorflow/contrib/makefile/downloads/eigen/Eigen ${INCLUDE_DIR}/
    # cp -rf tensorflow/contrib/makefile/downloads/gemmlowp ${INCLUDE_DIR}/

    # cp -rf tensorflow/contrib/makefile/gen/proto/tensorflow ${INCLUDE_DIR}/
    # cp -rf tensorflow/contrib/makefile/gen/proto_text/tensorflow ${INCLUDE_DIR}/

    # cp -rf tensorflow/core/framework * ${INCLUDE_DIR}/

    #tensorflow docs say, copy into ${INCLUDE_DIR}, but this seems excessive
    # the root folder of tensorflow,
    # tensorflow/contrib/makefile/downloads/nsync/public
    # tensorflow/contrib/makefile/downloads/protobuf/src
    # tensorflow/contrib/makefile/downloads,
    # tensorflow/contrib/makefile/downloads/eigen, and
    # tensorflow/contrib/makefile/gen/proto.
}


function build_tensorflow_android() {

	if [ $# -eq 0 ]; then
        #build_tensorflow_android --arch 32 #BROKEN at nsync
        build_tensorflow_android --arch 64
        #build_tensorflow_android --arch x86
        #build_tensorflow_android --arch x86_64
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
            ANDROID_ABI="armeabi-v7a"
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

	INSTALL_DIR="$CB/prebuilts/tensorflow/lib/android/${ANDROID_ABI}"

	#rm -Rf ${INSTALL_DIR}
	mkdir -p ${INSTALL_DIR}

    #   echo "Usage: NDK_ROOT=<path to ndk root> $(basename "$0") [-Es:t:Tx:a]"
    #   echo "-E enable experimental hexnn ops"
    #   echo "-s [sub_makefiles] sub makefiles separated by white space"
    #   echo "-t [build_target] build target for Android makefile [default=all]"
    #   echo "-T only build tensorflow"
    #   echo "-x [hexagon library path] copy and hexagon libraries in the specified path"
    #   echo "-a [architecture] Architecture of target android [default=armeabi-v7a] \
    # (supported architecture list: \
    # arm64-v8a armeabi armeabi-v7a mips mips64 x86 x86_64 tegra)"

	#bash tensorflow/contrib/makefile/compile_android_protobuf.sh -a "${ANDROID_ABI}" -c

    #nsync is currently broken for android on osx armeabi-v7a

	bash tensorflow/contrib/makefile/build_all_android.sh -a "${ANDROID_ABI}"

	cp tensorflow/contrib/makefile/gen/lib/android_${ANDROID_ABI}/libtensorflow-core.a ${INSTALL_DIR}
	cp -rf tensorflow/contrib/makefile/gen/protobuf/lib/* ${INSTALL_DIR}/
    cp tensorflow/contrib/makefile/downloads/nsync/builds/${ANDROID_ABI}.android.c++11/nsync.a ${INSTALL_DIR}
}

function lipo_libs() {
    src_path=$1
    declare -a archs=($2)
    name=$3

    libs=""
    for arch in "${archs[@]}"; do
        libs="$libs ${src_path}/${arch}/${name}"
    done

    lipo -create ${libs} -output "${src_path}/${name}"
}

function build_tensorflow_ios() {
    #https://github.com/tensorflow/tensorflow/tree/master/tensorflow/examples/ios#building-the-tensorflow-ios-libraries-from-source

    INSTALL_DIR=$CB/prebuilts/tensorflow/lib/ios

    # usage:
    # echo "-a [build_arch] build only for specified arch x86_64 [default=all]"
    # echo "-g [graph] optimize and selectively register ops only for this graph"
    # echo "-T only build tensorflow (dont download other deps etc)"
    
    # bazel is crashing for me: 
    # tensorflow/contrib/makefile/build_all_ios.sh -g $CB/sdk-assets/nnets/icnet.pb

    ALL_ARCHS="x86_64 armv7 armv7s arm64"
    #ALL_ARCHS="x86_64"

    declare -a archs=("${ALL_ARCHS}")

    for arch in "${archs[@]}"; do

        archdir="${INSTALL_DIR}/${arch}"
        mkdir -p ${archdir}

        tensorflow/contrib/makefile/build_all_ios.sh -a ${arch}

        #move binaries to individual arch dirs
        mv tensorflow/contrib/makefile/gen/lib/libtensorflow-core.a ${archdir}
        mv tensorflow/contrib/makefile/gen/protobuf_ios/lib/libprotobuf-lite.a ${archdir}
        mv tensorflow/contrib/makefile/gen/protobuf_ios/lib/libprotobuf.a ${archdir}
        mv tensorflow/contrib/makefile/downloads/nsync/builds/lipo.ios.c++11/nsync.a ${archdir}
    done

    # lipo together into fat dir
    lipo_libs "${INSTALL_DIR}" "${ALL_ARCHS}" "libtensorflow-core.a"
    lipo_libs "${INSTALL_DIR}" "${ALL_ARCHS}" "libprotobuf-lite.a"
    lipo_libs "${INSTALL_DIR}" "${ALL_ARCHS}" "libprotobuf.a"
    lipo_libs "${INSTALL_DIR}" "${ALL_ARCHS}" "nsync.a"
    
    #WHICH BUILDS:

    #fat binary
    # /Users/joelteply/Development/cambrian/cb-base/third-party/tensorflow/src/tensorflow/tensorflow/contrib/makefile/gen/lib/libtensorflow-core.a

    
}


function build_tensorflow_native() {

	cd $CB/third-party/tensorflow/src/tensorflow


	INSTALL_DIR=$CB/prebuilts/tensorflow/lib/${PLATFORM}

	rm -Rf ${INSTALL_DIR}
	mkdir -p ${INSTALL_DIR}

	# Compile protobuf.
	tensorflow/contrib/makefile/compile_linux_protobuf.sh

	# Build TensorFlow.
	make -j8 -f tensorflow/contrib/makefile/Makefile \
	  OPTFLAGS="-O3 -march=native" \
	  HOST_CXXFLAGS="--std=c++11 -march=native"


	cp tensorflow/contrib/makefile/gen/lib/libtensorflow-core.a ${INSTALL_DIR}
	cp tensorflow/contrib/makefile/gen/protobuf/lib/libprotobuf-lite.a ${INSTALL_DIR}
	cp tensorflow/contrib/makefile/gen/protobuf/lib/libprotobuf.a ${INSTALL_DIR}
	cp tensorflow/contrib/makefile/gen/protobuf/lib/libprotoc.a ${INSTALL_DIR}

	INCLUDE_DIR=$CB/prebuilts/tensorflow/include
	cp -Rf tensorflow/contrib/makefile/gen/protobuf/include/* ${INCLUDE_DIR}

}
