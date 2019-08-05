#!/bin/bash

function build_cbclient_selector() {

    BUILD_TYPE=Release
    
    cb_logger "Building cbclient for $1 (${BUILD_TYPE})"

    if [ "${BUILD_TYPE}" == "Debug" ]; then
        CMAKE_FLAGS="-DDO_LOGGING=1 -DDO_WRITING=1"
    else
        CMAKE_FLAGS="-DDO_LOGGING=0 -DDO_WRITING=0"
    fi

    case $1 in
        osx)
            build_cbclient_osx ${CMAKE_FLAGS}
            ;;
        *)
            build_cbclient_native ${CMAKE_FLAGS}
            exit 1
    esac
}

function build_cbclient_native() {

    CMAKE_FLAGS=$1

    BUILD_NAME=${PLATFORM}
    BUILD_DIR="$CB/build/cbclient/${PLATFORM}"
    INSTALL_DIR="${CB}/prebuilts/cbclient/${PLATFORM}/${BUILD_TYPE}"

    rm -Rf ${BUILD_DIR}/*
    mkdir -p ${BUILD_DIR}

    rm -Rf ${INSTALL_DIR}/*
    mkdir -p ${INSTALL_DIR}

    cd ${BUILD_DIR}
    cmake   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
            -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${INSTALL_DIR}" \
            ${CMAKE_FLAGS} \
            ${CB}/cb-cpp/src/cb-cpp/server
    
    make clean
    make -j8
    make install

}

function build_cbclient_osx() {

    CMAKE_FLAGS=$1

    BUILD_NAME=${PLATFORM}
    BUILD_DIR="${CB}/prebuilts/cbclient/${PLATFORM}"

    rm -Rf ${BUILD_DIR}/CMake*
    rm -Rf ${BUILD_DIR}/*.build
    mkdir -p ${BUILD_DIR}

    cd ${BUILD_DIR}

    cmake   -G Xcode \
            ${CMAKE_FLAGS} \
            -DCMAKE_INSTALL_PREFIX="${BUILD_TYPE}" \
            -DDO_LOGGING=1 \
            ${CB}/cb-cpp/src/cb-cpp/server
    
    #open editor if not already open
    [[ $( ps aux | grep cbclient) != "cbclient" ]] && open cbclient.xcodeproj
}

