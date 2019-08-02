set(libraryName "rendering")

project( ${libraryName} )

file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

set(HEADERS ${HEADERS} ${headerFiles}  CACHE INTERNAL "")
set(SOURCE ${SOURCE} ${compileFiles}  CACHE INTERNAL "")

install (FILES ${headerFiles} DESTINATION include/cbar/pipeline/${libraryName})
