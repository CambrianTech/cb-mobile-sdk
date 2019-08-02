# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(libraryName "CambrianAR")

set(CB_ROOT "" CACHE STRING "Must pass in the location of cb-base root (-DCB_ROOT)")
set(CB_CPP "" CACHE STRING "Must pass in the location of cb-cpp root (-DCB_CPP)")
set(CB_PREBUILTS "" CACHE STRING "Must pass in the location of prebuilts (-DCB_PREBUILTS)")

include(${CB_CPP}/cmake/functions.cmake)

project( ${libraryName} )

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

option(DO_LOGGING "Enable Console Logging" ON)

file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

set(HEADERS ${HEADERS} ${headerFiles} CACHE INTERNAL "")
set(SOURCE ${SOURCE} ${compileFiles} CACHE INTERNAL "")

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(dir ${SUBDIRS})
    add_subdirectory(${dir})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

add_library(${libraryName} SHARED
    ${SOURCE}
)

SET("BGFX_DIR" "${CB_PREBUILTS}/bgfx")
SET("Eigen_INCLUDE_DIRS" "${CB_ROOT}/third-party/eigen/src/eigen")
SET("BZIP2_DIR" "${CB_PREBUILTS}/bzip2")
SET("LIBLZMA_DIR" "${CB_PREBUILTS}/liblzma")
set("jsoncpp_DIR" "${CB_PREBUILTS}/jsoncpp")

SET("OpenCV_DIR" "${CB_PREBUILTS}/opencv/opencv-android/sdk/native/jni")
SET("FastCV_DIR" "${CB_PREBUILTS}/fastcv")
find_package(FastCV REQUIRED)

SET("CAMBRIAN_DIR" "${CB_PREBUILTS}/CambrianAR")
find_package( CAMBRIAN REQUIRED )
find_package( BGFX REQUIRED )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       ${libraryName}

                       # Links the target library to the log library
                       # included in the NDK.
                       ${CAMBRIAN_LIBS}
                       ${BGFX_LIBS}
                       ${log-lib} )

install (FILES ${headerFiles} DESTINATION include)