
cmake_minimum_required(VERSION 3.2)

set(libraryName "unity-native")

project( ${libraryName})

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Native)

include_directories(${PROJECT_SOURCE_DIR})

set(LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Libraries/libil2cpp/include)

include_directories(${PROJECT_SOURCE_DIR} ${LIB_SOURCE_DIR})

file(GLOB compileFiles ${PROJECT_SOURCE_DIR}/*.cpp)

set(CMAKE_OSX_SYSROOT iphoneos) 
#set(CMAKE_OSX_ARCHITECTURES "arm64;armv7s;x86_64") 
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator") 

set(CMAKE_MACOSX_BUNDLE YES)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")


add_library(${libraryName} ${compileFiles})

set_target_properties(${libraryName} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "10.0" CACHE STRING "Minimum Deployment Target")



