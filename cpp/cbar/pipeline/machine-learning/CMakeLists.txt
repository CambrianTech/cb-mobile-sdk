
set(libraryName "machine-learning")

project( ${libraryName} )


file(GLOB headerFiles "*.h" "*.hpp")
file(GLOB compileFiles "*.cpp")

set(SOURCE ${SOURCE} ${compileFiles}  CACHE INTERNAL "")

install (FILES ${headerFiles} DESTINATION include/cbar/pipeline/${libraryName})

install (FILES ${headerFiles} DESTINATION include/cbar/pipeline/${libraryName})
