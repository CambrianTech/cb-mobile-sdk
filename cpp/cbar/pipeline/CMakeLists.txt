set(libraryName "pipeline")

project( ${libraryName} CXX)

file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

set(HEADERS ${HEADERS} ${headerFiles} CACHE INTERNAL "")
set(SOURCE ${SOURCE} ${compileFiles} CACHE INTERNAL "")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

foreach(dir ${SUBDIRS})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
    add_subdirectory(${dir})
endforeach()

install (FILES ${headerFiles} DESTINATION include/cbar/${libraryName})
