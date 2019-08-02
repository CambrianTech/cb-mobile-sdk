#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(BGFX REQUIRED)
#    include_directories(${BGFX_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
#    target_link_libraries(MY_TARGET_NAME ${BGFX_LIBS})

# Ensure the directory exists
if( NOT IS_DIRECTORY ${ZLIB_DIR} )
	message( SEND_ERROR "Could not load zlib, directory does not exist. ${ZLIB_DIR}" )
	return()
endif()

set(ZLIB_LIB_DIRS ${ZLIB_DIR}/lib)
set(ZLIB_INCLUDE_DIRS ${ZLIB_DIR}/include)

include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIB_DIRS})

set(ZLIB_LIBS ${ZLIB_DIR}/lib/zlib.lib)

