#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(jsoncpp REQUIRED)
#    include_directories(${jsoncpp_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
#    target_link_libraries(MY_TARGET_NAME ${jsoncpp_LIBS})

# Ensure the directory exists
if( NOT IS_DIRECTORY ${jsoncpp_DIR} )
	message( SEND_ERROR "Could not load jsoncpp, directory does not exist. ${jsoncpp_DIR}" )
	return()
endif()

if (ANDROID)
	set(jsoncpp_LIB_DIRS ${jsoncpp_DIR}/android/lib/${CHOSEN_ABI})
	set(jsoncpp_INCLUDE_DIRS ${jsoncpp_DIR}/android/include)
elseif( APPLE )
	set(jsoncpp_LIB_DIRS ${jsoncpp_DIR}/darwin-x86_64/lib)
	set(jsoncpp_INCLUDE_DIRS ${jsoncpp_DIR}/darwin-x86_64/include)
elseif (UNIX)
	set(jsoncpp_LIB_DIRS ${jsoncpp_DIR}/linux-x86_64/lib)
	set(jsoncpp_INCLUDE_DIRS ${jsoncpp_DIR}/linux-x86_64/include)
endif()

#message( FATAL_ERROR "BLAHBLAH ${jsoncpp_INCLUDE_DIRS}" )

include_directories(${jsoncpp_INCLUDE_DIRS})
link_directories(${jsoncpp_LIB_DIRS})

set(jsoncpp_LIBS 
	${jsoncpp_LIB_DIRS}/libjsoncpp.a
	)

