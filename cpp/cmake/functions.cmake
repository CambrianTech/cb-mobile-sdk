
set(CMAKE_POLICY_DEFAULT_CMP0054 OLD)

option(FLOORING_ENABLED "Enable Flooring" OFF)
option(DEBUG "Enable debug mode" OFF)
option(DO_LOGGING "Enable debug mode" OFF)
option(DO_WRITING "Enable debug mode" ON)
option(USE_BGFX "Enable BGFX Rendering engine" OFF)

if (ANDROID_NDK) 
  set(ANDROID ON CACHE INTERNAL "")
  string(REGEX MATCH "[A-Za-z0-9\\-]*" CHOSEN_ABI ${ANDROID_ABI})
  MESSAGE(STATUS "Detected Android ABI ${CHOSEN_ABI}")
elseif (APPLE)
  if (CMAKE_TOOLCHAIN_FILE) 
    set(IOS ON CACHE INTERNAL "")
    MESSAGE(STATUS "Detected IOS")
  else()
    set(OSX ON CACHE INTERNAL "")
    MESSAGE(STATUS "Detected OSX")
  endif()
elseif (WIN32)
    set(WIN32 ON CACHE INTERNAL "")
    MESSAGE(STATUS "Detected WINDOWS")
else()
  MESSAGE(STATUS "Detected LINUX")
endif()

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

macro(ADD_OSX_FRAMEWORK target fwname)

  if (APPLE)

    # Cannot use ARGN directly with list() command.
    # Copy to a variable first.
    set (extra_macro_args ${ARGN})

    # Did we get any optional args?
    list(LENGTH extra_macro_args num_extra_args)
    if (${num_extra_args} GREATER 0)
        list(GET extra_macro_args 0 public_arg)
        SET (VISIBILITY ${public_arg})
    endif ()

    find_library(FRAMEWORK_${fwname}
    NAMES ${fwname}
    PATHS ${CMAKE_OSX_SYSROOT}/System/Library
    PATH_SUFFIXES Frameworks
    NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${target} ${VISIBILITY} "${FRAMEWORK_${fwname}}/${fwname}")
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()

  endif() #IF APPLE
endmacro(ADD_OSX_FRAMEWORK)

macro(ADD_CXX_FLAGS)

  if (ANDROID) 
    if("${ANDROID_ABI}" STREQUAL "arm64-v8a")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall \
        -fvisibility=default -Wno-unused-variable -Wno-reorder -Wno-sign-compare \
        -Wno-unknown-pragmas -Wl,--build-id -g")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall \
        -fvisibility=default -Wno-unused-variable -Wno-reorder -Wno-sign-compare \
        -Wno-unknown-pragmas -Wl,--build-id -mapcs-frame -g")
    endif()
    
  elseif (NOT WIN32)    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall \
        -fvisibility=default -Wno-unused-variable -Wno-reorder -Wno-unused-private-field \
        -Wno-unknown-pragmas -Wno-unused-const-variable")
  endif()

  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=std++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

    string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

    MESSAGE(STATUS "\n#######################################")

    if (DO_WRITING)
      MESSAGE(STATUS "WRITING OUTPUT is enabled.")
      add_definitions(-DDO_WRITING=1)
    endif()

    if (DO_LOGGING)
      MESSAGE(STATUS "DEBUG LOGGING is enabled.")
      add_definitions(-DDO_LOGGING=1)
    endif()

    if (DEBUG OR "${BUILD_TYPE}" STREQUAL "DEBUG")
      MESSAGE(STATUS "DEBUG macro is enabled.")
      add_definitions(-DDEBUG=1)
    endif()

    if (FLOORING_ENABLED)
      MESSAGE(STATUS "FLOORING is enabled.")
      add_definitions(-DFLOORING=1)
    endif()

    if (USE_BGFX)
      MESSAGE(STATUS "BGFX is enabled.")
      add_definitions(-DUSE_BGFX=1)
    endif()

    message("\nBUILD TYPE IS SET TO ${BUILD_TYPE}")

    set(message_cxx_flags ${CMAKE_CXX_FLAGS})
    set(message_c_flags ${CMAKE_C_FLAGS})

    if ("${BUILD_TYPE}" STREQUAL "DEBUG")
      set(message_cxx_flags "${message_cxx_flags} ${CMAKE_CXX_FLAGS_DEBUG}")
      set(message_c_flags "${message_c_flags} ${CMAKE_C_FLAGS_DEBUG}")
    else() 
      set(message_cxx_flags "${message_cxx_flags} ${CMAKE_CXX_FLAGS_RELEASE}")
      set(message_c_flags "${message_c_flags} ${CMAKE_C_FLAGS_RELEASE}")
    endif()

    MESSAGE(STATUS "\nBuilding with the following compiler flags: \nCMAKE_C_FLAGS: ${message_c_flags} \n\nCMAKE_CXX_FLAGS:\n ${message_cxx_flags}\n")

    MESSAGE(STATUS "\n\n#######################################\n\n")

endmacro(ADD_CXX_FLAGS)
