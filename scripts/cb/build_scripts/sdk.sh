#!/usr/bin/env bash

function build_sdk_selector() {
    cb_logger "Building SDK for $1 (${BUILD_TYPE})"
    case $1 in
        android)
            build_sdk_android
            ;;
        ios)
            build_sdk_ios
            ;;
        *)
            cb_logger "Don't know how to build SDK for $1"
            exit 1
    esac
}

function build_sdk_ios() {
    
    ######################
    # Options
    ######################
    BUILD_DIR="${CB}/build/framework"
    PROJECT_NAME="CambrianAR"
    PROJECT_FILE_PATH="${CB}/cambrian-ios/CambrianAR/CambrianAR.xcodeproj"

    TARGET_NAME="${PROJECT_NAME}"
    FRAMEWORK_NAME="${PROJECT_NAME}.framework"
    INSTALL_DIR="${CB}/prebuilts/${PROJECT_NAME}"

    CONFIG="Release"

    ######################
    # Build iPhone Frameworks
    ######################
    IPHONE_SDKS=( iphoneos )

    BINARY_PATHS=""

    for SDK in "${IPHONE_SDKS[@]}"; do

        OUTPUT_DIR="${BUILD_DIR}/${PROJECT_NAME}-${SDK}"

        say "Building ${SDK} for ${CONFIG} configuration"

        if [[ $SDK == *"simulator"* ]]; then
            ARCHS="x86_64 i386"
        else
            ARCHS="armv7 armv7s arm64"
        fi

        xcodebuild clean build -project "${PROJECT_FILE_PATH}" -target "${TARGET_NAME}" -sdk ${SDK} -configuration Release \
                   ONLY_ACTIVE_ARCH=NO ARCHS="${ARCHS}" \
                   CONFIGURATION_BUILD_DIR="${OUTPUT_DIR}" \
                   IPHONEOS_DEPLOYMENT_TARGET=${IPHONEOS_DEPLOYMENT_TARGET}


        cb_logger "${SDK} build completed"

        CUR_FRAMEWORK_PATH="${OUTPUT_DIR}/${FRAMEWORK_NAME}"
        CUR_BINARY_PATH="${CUR_FRAMEWORK_PATH}/${TARGET_NAME}"

        BINARY_PATHS="${BINARY_PATHS} ${CUR_BINARY_PATH}"
    done

    ######################
    # Install
    ######################

    FINAL_FRAMEWORK_PATH="${INSTALL_DIR}/${FRAMEWORK_NAME}"
    rm -rf "${FINAL_FRAMEWORK_PATH}"
    mkdir -p "${FINAL_FRAMEWORK_PATH}"

    cp -a ${CUR_FRAMEWORK_PATH} ${INSTALL_DIR}
    cp "${CB}/scripts/strip-frameworks.sh" "${FINAL_FRAMEWORK_PATH}/strip-frameworks.sh"

    FINAL_BINARY_PATH="${FINAL_FRAMEWORK_PATH}/${TARGET_NAME}"

    lipo -create ${BINARY_PATHS} -o ${FINAL_BINARY_PATH}
}


function build_sdk_android() {
    download_dependency "cambrian-android"

    android_defs

    echo ""
}