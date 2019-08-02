#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(LIBLZMA REQUIRED)
#    include_directories(${LIBLZMA_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
#    target_link_libraries(MY_TARGET_NAME ${LIBLZMA_LIBS})

# Ensure the directory exists
if( NOT IS_DIRECTORY ${LIBLZMA_DIR} )
	message( SEND_ERROR "Could not load LIBLZMA, directory does not exist. ${LIBLZMA_DIR}" )
	return()
endif()

set(LIBLZMA_INCLUDE_DIRS ${LIBLZMA_DIR}/include)

if (ANDROID)
	set(LIBLZMA_LIB_DIRS ${LIBLZMA_DIR}/lib/android/${CHOSEN_ABI})
elseif( APPLE )
	set(LIBLZMA_LIB_DIRS ${LIBLZMA_DIR}/lib/darwin-x86_64)
elseif (UNIX)
	set(LIBLZMA_LIB_DIRS ${LIBLZMA_DIR}/lib/linux-x86_64)
endif()

include_directories(${LIBLZMA_INCLUDE_DIRS})
link_directories(${LIBLZMA_LIB_DIRS})

set(LIBLZMA_LIBS 
	${LIBLZMA_LIB_DIRS}/liblzma.a
	)

