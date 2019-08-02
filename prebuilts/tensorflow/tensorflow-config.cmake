#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(Tensorflow REQUIRED)
#    include_directories(${Tensorflow_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
#    target_link_libraries(MY_TARGET_NAME ${Tensorflow_LIBS})

# Ensure the directory exists
if( NOT IS_DIRECTORY ${Tensorflow_DIR} )
	message( SEND_ERROR "Could not load bgfx, directory does not exist. ${Tensorflow_DIR}" )
	return()
endif()

if( APPLE )
	set(Tensorflow_LIB_DIRS "${Tensorflow_DIR}/lib/darwin-x86_64")
elseif (ANDROID)
	set(Tensorflow_LIB_DIRS "${Tensorflow_DIR}/lib/android/${CHOSEN_ABI}")
elseif (UNIX)
	set(Tensorflow_LIB_DIRS "${Tensorflow_DIR}/lib/linux-x86_64")
endif()

set(Tensorflow_INCLUDE_DIRS "${Tensorflow_DIR}/include")

include_directories("${Tensorflow_INCLUDE_DIRS}")
link_directories("${Tensorflow_LIB_DIRS}")

set(Tensorflow_LIBS 
					"${Tensorflow_LIB_DIRS}/libtensorflow-core.a"
					#"${Tensorflow_LIB_DIRS}/libprotobuf-lite.a"
					"${Tensorflow_LIB_DIRS}/libprotobuf.a"
					)

