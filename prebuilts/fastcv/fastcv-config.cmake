#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(FastCV REQUIRED)
#    include_directories(${FastCV_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
#    target_link_libraries(MY_TARGET_NAME ${FastCV_LIBS})

# Ensure the directory exists
if( NOT IS_DIRECTORY ${FastCV_DIR} )
	message( SEND_ERROR "Could not load FastCV, directory does not exist. ${FastCV_DIR}" )
	return()
endif()

set(FastCV_LIB_DIRS ${FastCV_DIR}/inc/lib/${ANDROID_NDK_ABI_NAME})
set(FastCV_INCLUDE_DIRS ${FastCV_DIR}/inc)

include_directories(${FastCV_INCLUDE_DIRS})
link_directories(${FastCV_LIB_DIRS})

set(FastCV_LIBS ${FastCV_LIB_DIRS}/libfastcv.a)

