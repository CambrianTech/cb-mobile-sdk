
set(libraryName "cbar")

project( ${libraryName} )

find_package( OpenCV REQUIRED )

file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

list(REMOVE_ITEM headerFiles "${CMAKE_CURRENT_SOURCE_DIR}/CBAR_GLOffscreenRenderer.hpp")
list(REMOVE_ITEM compileFiles "${CMAKE_CURRENT_SOURCE_DIR}/CBAR_GLOffscreenRenderer.cpp")

set(HEADERS ${HEADERS} ${headerFiles} CACHE INTERNAL "")
set(SOURCE ${SOURCE} ${compileFiles} CACHE INTERNAL "")

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(dir ${SUBDIRS})
    add_subdirectory(${dir})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

install (FILES ${headerFiles} DESTINATION include/${libraryName})
