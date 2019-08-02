
set(libraryName "imaging")

project( ${libraryName} )

include_directories(${BGFX_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})

file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

set(HEADERS ${HEADERS} ${headerFiles} CACHE INTERNAL "")
set(SOURCE ${SOURCE} ${compileFiles} CACHE INTERNAL "")

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(dir ${SUBDIRS})
add_subdirectory(${dir})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

install (FILES ${headerFiles} DESTINATION include/${libraryName})
