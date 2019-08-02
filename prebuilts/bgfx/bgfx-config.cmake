#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(BGFX REQUIRED)
#    include_directories(${BGFX_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
#    target_link_libraries(MY_TARGET_NAME ${BGFX_LIBS})

# Ensure the directory exists
if( NOT IS_DIRECTORY ${BGFX_DIR} )
	message( SEND_ERROR "Could not load bgfx, directory does not exist. ${BGFX_DIR}" )
	return()
endif()

if (ANDROID)
	if("${ANDROID_ABI}" STREQUAL "arm64-v8a")
		set(BGFX_LIB_DIRS ${BGFX_DIR}/lib/android-arm64)
	else()
		set(BGFX_LIB_DIRS ${BGFX_DIR}/lib/android-arm)
	endif()
elseif( APPLE )
	set(BGFX_LIB_DIRS ${BGFX_DIR}/lib/darwin-x86_64)
elseif (UNIX)
	set(BGFX_LIB_DIRS ${BGFX_DIR}/lib/linux-x86_64)
endif()

set(BGFX_INCLUDE_DIRS ${BGFX_DIR}/include)

include_directories(${BGFX_INCLUDE_DIRS})
link_directories(${BGFX_LIB_DIRS})

set(BGFX_LIBS ${BGFX_LIB_DIRS}/libbgfxRelease.a ${BGFX_LIB_DIRS}/libbxRelease.a)

