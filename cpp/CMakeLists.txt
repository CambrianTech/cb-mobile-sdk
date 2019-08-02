
cmake_minimum_required(VERSION 3.2)

set(CPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

include(${CPP_ROOT}/cmake/functions.cmake)

set(libraryName "cambrian")

project( ${libraryName})

ADD_CXX_FLAGS()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

SET("BGFX_DIR" "${CB_ROOT}/prebuilts/bgfx")
SET("Eigen_INCLUDE_DIRS" "${CB_ROOT}/third-party/eigen/src/eigen")
if (WIN32)
	SET("GLEW_INCLUDE_DIRS" "${CB_ROOT}/third-party/glew/include")
endif()
SET("BZIP2_DIR" "${CB_ROOT}/prebuilts/bzip2")
if (NOT WIN32)
    SET("LIBLZMA_DIR" "${CB_ROOT}/prebuilts/liblzma")
endif()
SET("jsoncpp_DIR" "${CB_ROOT}/prebuilts/jsoncpp")

if (ANDROID)
    SET("OpenCV_DIR" "${CB_ROOT}/prebuilts/opencv/opencv-android/sdk/native/jni")
    SET("FastCV_DIR" "${CB_ROOT}/prebuilts/fastcv")
    find_package(FastCV REQUIRED)
elseif (OSX)
    SET("OpenCV_DIR" "${CB_ROOT}/prebuilts/opencv/opencv-darwin-x86_64/share/OpenCV")
elseif (WIN32)
    SET("OpenCV_DIR" "${CB_ROOT}/prebuilts/opencv/opencv-win32-x86_64/share/OpenCV")
else()
    SET("OpenCV_DIR" "${CB_ROOT}/prebuilts/opencv/opencv-linux-x86_64/share/OpenCV")
endif()


if (USE_BGFX)
    find_package(BGFX REQUIRED PATHS "${BGFX_DIR}")
endif()

find_package(OpenCV REQUIRED PATHS "${OpenCV_DIR}")
find_package( bzip2 REQUIRED PATHS "${BZIP2_DIR}")
if (NOT WIN32)
    find_package( liblzma REQUIRED PATHS "${LIBLZMA_DIR}")
endif()
find_package( jsoncpp REQUIRED PATHS "${jsoncpp_DIR}")


include_directories(
    ${CPP_ROOT}
    ${Tensorflow_INCLUDE_DIRS}
    ${BGFX_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${FastCV_INCLUDE_DIRS}
    ${Eigen_INCLUDE_DIRS}
    ${BZIP2_INCLUDE_DIRS}
    ${jsoncpp_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
)

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

list(REMOVE_ITEM SUBDIRS ".git" "server" "Cambrian.xcodeproj" "build" "cmake" ".idea" ".vs" "cmake-build-debug")

foreach(dir ${SUBDIRS})
	add_subdirectory(${dir})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

add_library(${libraryName} ${HEADERS} ${SOURCE})

if (APPLE)
    target_link_libraries(${libraryName} PUBLIC "-framework Metal")
    target_link_libraries(${libraryName} PUBLIC "-framework QuartzCore")
    target_link_libraries(${libraryName} PUBLIC "-framework Accelerate")
endif()

target_link_libraries( ${libraryName} PRIVATE ${OpenCV_LIBS} ${BGFX_LIBS} ${Tensorflow_LIBS})
target_link_libraries( ${libraryName} PRIVATE ${BZIP2_LIBS} ${LIBLZMA_LIBS} ${jsoncpp_LIBS})

if (ANDROID)
    target_link_libraries( ${libraryName} PRIVATE ${FastCV_LIBS})
    target_link_libraries( ${libraryName} PRIVATE EGL GLESv3 jnigraphics android)
endif()

install (FILES ${headerFiles} cambrian.h DESTINATION include)
install (TARGETS ${libraryName} DESTINATION lib)
