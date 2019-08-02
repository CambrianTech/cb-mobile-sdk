cmake_minimum_required(VERSION 3.0)

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(dir ${SUBDIRS})
    add_subdirectory(${dir})
endforeach()
