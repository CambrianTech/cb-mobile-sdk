
cmake_minimum_required(VERSION 3.2)

set(libraryName "cbclient")

project( ${libraryName} )

ADD_CXX_FLAGS()

SET("BGFX_DIR" "${CB_ROOT}/prebuilts/bgfx")
SET("ASSIMP_DIR" "${CB_ROOT}/prebuilts/assimp")
SET("Eigen_INCLUDE_DIRS" "${CB_ROOT}/third-party/eigen/src/eigen")
SET("BZIP2_DIR" "${CB_ROOT}/prebuilts/bzip2")
SET("LIBLZMA_DIR" "${CB_ROOT}/prebuilts/liblzma")
set("jsoncpp_DIR" "${CB_ROOT}/prebuilts/jsoncpp")

if (OSX)
    SET("OpenCV_DIR" "${CB_ROOT}/prebuilts/opencv/opencv-darwin-x86_64/share/OpenCV")
else()
    SET("OpenCV_DIR" "${CB_ROOT}/prebuilts/opencv/opencv-linux-x86_64/share/OpenCV")
endif()

find_package(OpenCV REQUIRED PATHS "${OpenCV_DIR}")

SET("CAMBRIAN_DIR" "${CB_ROOT}/prebuilts/CambrianAR")
find_package( CAMBRIAN REQUIRED )

find_package(BGFX REQUIRED PATHS "${BGFX_DIR}")
find_package(ASSIMP REQUIRED PATHS "${ASSIMP_DIR}")
find_package(OpenCV REQUIRED PATHS "${OpenCV_DIR}")
find_package( bzip2 REQUIRED PATHS "${BZIP2_DIR}")
find_package( liblzma REQUIRED PATHS "${LIBLZMA_DIR}")
find_package( jsoncpp REQUIRED PATHS "${jsoncpp_DIR}")


file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

include_directories(
    ${CB_ROOT}/cb-cpp/src/cb-cpp
    ${BGFX_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${FastCV_INCLUDE_DIRS}
    ${Eigen_INCLUDE_DIRS}
    ${BZIP2_INCLUDE_DIRS}
    ${jsoncpp_INCLUDE_DIRS}
)

set(HEADERS ${HEADERS} ${headerFiles} CACHE INTERNAL "")
set(SOURCE ${SOURCE} ${compileFiles} CACHE INTERNAL "")

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(dir ${SUBDIRS})
    add_subdirectory(${dir})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable (${libraryName} ${SOURCE})

ADD_CUSTOM_TARGET(Core SOURCES "${CB_ROOT}/cambrian-ios/Cambrian/Cambrian.xcodeproj")

if (APPLE)
    target_link_libraries(${libraryName} PUBLIC "-framework Metal")
    target_link_libraries(${libraryName} PUBLIC "-framework QuartzCore")
    target_link_libraries(${libraryName} PUBLIC "-framework Accelerate")
endif()

target_link_libraries( ${libraryName} INTERFACE ${OpenCV_LIBS} )
target_link_libraries( ${libraryName} PRIVATE ${CAMBRIAN_LIBS} )
target_link_libraries( ${libraryName} PRIVATE ${BZIP2_LIBS} ${LIBLZMA_LIBS} ${jsoncpp_LIBS})


install (TARGETS ${libraryName} DESTINATION bin)
