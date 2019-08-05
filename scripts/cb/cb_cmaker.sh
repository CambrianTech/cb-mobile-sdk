#!/usr/bin/env bash

function cb_cmaker() {
  cb_logger "Generating cmake files for $@"
  cd $CBCORE
  cmake .

  # OCVD=$CB/prebuilts/android/opencv/sdk/native/jni
  # CMAKE_ANDROID_TOOLCHAIN="$CB/third-party/opencv/src/opencv/platforms/android/android.toolchain.cmake"

  # BUILD_TYPES=('debug' 'release' 'relwithdebinfo' 'minsizerel')
  # SHARED_LIBS=ON

  # for CPU_ARCH in 'arm' 'aarch64'; do
  #     for BUILD_TYPE in "${BUILD_TYPES[@]}"; do
  #             if [[ "${CPU_ARCH}" == "arm" ]]; then
  #                 ANDROID_FORCE_ARM_BUILD="ON"
  #                 ANDROID_STANDALONE_TOOLCHAIN="$CB/android-toolchains/osx/arm-linux-androideabi-4.9"
  #                 ANDROID_TOOLCHAIN_NAME="standalone"
  #                 ANDROID_ABI="armeabi-v7a with NEON"
  #                 ANDROID_NATIVE_API_LEVEL="android-23"
  #             elif [[ "${CPU_ARCH}" == "aarch64" ]]; then
  #                 ANDROID_FORCE_ARM_BUILD="OFF"
  #                 ANDROID_TOOLCHAIN_NAME="standalone"
  #                 export ANDROID_STANDALONE_TOOLCHAIN="$CB/android-toolchains/osx/aarch64-linux-android-4.9"
  #                 ANDROID_ABI="arm64-v8a"
  #                 ANDROID_NATIVE_API_LEVEL="android-23"
  #             fi

  #             BUILD_DIRECTORY="$CB/build/android/$CPU_ARCH-shared-${BUILD_TYPE}"
  #             [[ -d ${BUILD_DIRECTORY} ]] && rm -rf "${BUILD_DIRECTORY}"
  #             mkdir -p "${BUILD_DIRECTORY}" && cd "${BUILD_DIRECTORY}" && \
  #             cmake -Gmake -Wno-dev \
  #                 -DANDROID_STL="gnustl_static" \
  #                 -DANDROID_STL_FORCE_FEATURES="ON" \
  #                 -DANDROID_TOOLCHAIN_NAME="${ANDROID_TOOLCHAIN_NAME}" \
  #                 -DANDROID_STANDALONE_TOOLCHAIN="${ANDROID_STANDALONE_TOOLCHAIN}" \
  #                 -DANDROID_ABI="${ANDROID_ABI}" \
  #                 -DANDROID_NATIVE_API_LEVEL="${ANDROID_NATIVE_API_LEVEL}" \
  #                 -DANDROID_FORCE_ARM_BUILD="${ANDROID_FORCE_ARM_BUILD}" \
  #                 -DOpenCV_DIR="${OCVD}" \
  #                 -DBUILD_SHARED_LIBS="${SHARED_LIBS}" \
  #                 -DCMAKE_TOOLCHAIN_FILE="${CMAKE_ANDROID_TOOLCHAIN}" \
  #                 -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  #                 -DLIBRARY_OUTPUT_PATH_ROOT="$CB/build/${NAME}/output" \
  #                 -DCMAKE_INSTALL_PREFIX="$CB/build/${NAME}/install" \
  #                 -DREMOTE_BUILD=1 \
  #                 "$CB"
  #         done
  #     done
  # done
  return 0
}
