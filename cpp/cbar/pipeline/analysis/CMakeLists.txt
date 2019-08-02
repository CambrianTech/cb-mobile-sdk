
set(libraryName "analysis")

project( ${libraryName} )

file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

list(REMOVE_ITEM headerFiles "${CMAKE_CURRENT_SOURCE_DIR}/CBP_DeepSegmenter.hpp")
list(REMOVE_ITEM compileFiles   "${CMAKE_CURRENT_SOURCE_DIR}/CBP_DeepSegmenter.cpp")

set(SOURCE ${SOURCE} ${compileFiles}  CACHE INTERNAL "")

install (FILES ${headerFiles} DESTINATION include/cbar/pipeline/${libraryName})
