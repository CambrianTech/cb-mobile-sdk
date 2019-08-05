#!/bin/bash

function build_bgfx_selector() {

    cb_logger "Building BGFX for $1 (${BUILD_TYPE})"

    download_dependency "bgfx"

    case $1 in
        android)
            android_defs
            build_bgfx_android
            ;;
        ios)
            build_bgfx_ios
            ;;
        osx | linux)
            build_bgfx_native
            ;;
        *)
            cb_logger "Don't know how to build SDK for $1"
            exit 1
        ;;
    esac

}

function build_bgfx_android() {
	echo $ANDROID_NDK_CLANG

	cd $CB/third-party/bgfx/src/bgfx
	make clean

	make android-arm-release
	make android-arm64-release
	make android-x86-release

	rm -Rf $CB/prebuilts/bgfx/lib/android-arm
	rm -Rf $CB/prebuilts/bgfx/lib/android-arm64
	rm -Rf $CB/prebuilts/bgfx/lib/android-x86

	mkdir -p $CB/prebuilts/bgfx/lib/android-arm
	mkdir -p $CB/prebuilts/bgfx/lib/android-arm64
	mkdir -p $CB/prebuilts/bgfx/lib/android-x86

	cp .build/android-arm/bin/*.a $CB/prebuilts/bgfx/lib/android-arm
	cp .build/android-arm/bin/*.a $CB/prebuilts/bgfx/lib/android-arm64
	cp .build/android-x86/bin/*.a $CB/prebuilts/bgfx/lib/android-x86

	make projgen
	
	#includes
	cp -R $CB/third-party/bgfx/src/bgfx/include/bgfx $CB/prebuilts/bgfx/include
	cp -R $CB/third-party/bgfx/src/bx/include/bx $CB/prebuilts/bgfx/include

}

function build_bgfx_ios() {
     cd $CB/third-party/bgfx/src/bgfx

     make clean

     make ios-arm-release
     make ios-armv7s-release
     make ios-arm64-release
     make ios-simulator-release
     make ios-simulator-64-release

     rm -Rf $CB/prebuilts/bgfx/include
     rm -Rf $CB/prebuilts/bgfx/lib/ios

     mkdir -p $CB/prebuilts/bgfx/include
     mkdir -p $CB/prebuilts/bgfx/lib/ios

     #includes
     cp -R $CB/third-party/bgfx/src/bgfx/include/bgfx $CB/prebuilts/bgfx/include
     cp -R $CB/third-party/bgfx/src/bx/include/bx $CB/prebuilts/bgfx/include
     cp -R $CB/third-party/bgfx/src/bimg/include/bimg $CB/prebuilts/bgfx/include

     #libraries
     lipo -create \
          $CB/third-party/bgfx/src/bgfx/.build/ios-simulator/bin/libbgfxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-simulator-64/bin/libbgfxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-arm64/bin/libbgfxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-armv7s/bin/libbgfxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-arm/bin/libbgfxRelease.a \
          -output $CB/prebuilts/bgfx/lib/ios/libbgfxRelease.a

     lipo -create \
          $CB/third-party/bgfx/src/bgfx/.build/ios-simulator/bin/libbxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-simulator-64/bin/libbxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-arm64/bin/libbxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-armv7s/bin/libbxRelease.a \
          $CB/third-party/bgfx/src/bgfx/.build/ios-arm/bin/libbxRelease.a \
          -output $CB/prebuilts/bgfx/lib/ios/libbxRelease.a

     # lipo -create \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-simulator/bin/libbimgRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-simulator-64/bin/libbimgRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-arm64/bin/libbimgRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-armv7s/bin/libbimgRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-arm/bin/libbimgRelease.a \
     #      -output $CB/prebuilts/bgfx/lib/ios/libbimgRelease.a

     # lipo -create \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-simulator/bin/libbimg_decodeRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-simulator-64/bin/libbimg_decodeRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-arm64/bin/libbimg_decodeRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-armv7s/bin/libbimg_decodeRelease.a \
     #      $CB/third-party/bgfx/src/bgfx/.build/ios-arm/bin/libbimg_decodeRelease.a \
     #      -output $CB/prebuilts/bgfx/lib/ios/libbimg_decodeRelease.a

     #tools, project links
     make projgen

     rm $CB/third-party/bgfx/src/bgfx/.build/projects 2>/dev/null || :
     ln -sfn $CB/third-party/bgfx/src/bgfx/.build/projects $CB/prebuilts/bgfx/projects

     mkdir -p $CB/prebuilts/bgfx/bin
     cp -R $CB/third-party/bgfx/src/bgfx/tools/bin/* $CB/prebuilts/bgfx/bin

}

function build_bgfx_native() {
	cd $CB/third-party/bgfx/src/bgfx

	make clean

	make osx-release64

	rm -Rf $CB/prebuilts/bgfx/lib/${PLATFORM}

	mkdir -p $CB/prebuilts/bgfx/lib/${PLATFORM}

	#library
	cp .build/osx64_clang/bin/*.a $CB/prebuilts/bgfx/lib/${PLATFORM}
	
	#includes
	cp -R $CB/third-party/bgfx/src/bgfx/include/bgfx $CB/prebuilts/bgfx/include
	cp -R $CB/third-party/bgfx/src/bx/include/bx $CB/prebuilts/bgfx/include

	#others
	make projgen

	make tools
	cp -R $CB/third-party/bgfx/src/bgfx/tools/bin/* $CB/prebuilts/bgfx/bin

}
