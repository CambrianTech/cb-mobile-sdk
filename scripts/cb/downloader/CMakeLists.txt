cmake_minimum_required(VERSION 3.0)
project(dependencies)

include(ExternalProject) # Should not need this after we are done.

set(third_party_dir "$ENV{CB}/third-party")


if (download STREQUAL "cb-android")
  ExternalProject_Add(cb-android
    PREFIX $ENV{CB}/cb-android
    GIT_REPOSITORY https://github.com/CambrianTech/cb-android.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-cpp")
  ExternalProject_Add(cb-cpp
    PREFIX $ENV{CB}/cb-cpp
    GIT_REPOSITORY https://github.com/CambrianTech/cb-cpp.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-ios")
  ExternalProject_Add(cb-ios
    PREFIX $ENV{CB}/cb-ios
    GIT_REPOSITORY https://github.com/CambrianTech/cb-ios.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-web")
  ExternalProject_Add(cb-web
    PREFIX $ENV{CB}/cb-web
    GIT_REPOSITORY https://github.com/CambrianTech/cb-web.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-ml")
  ExternalProject_Add(cb-ml
    PREFIX $ENV{CB}/cb-ml
    GIT_REPOSITORY https://github.com/CambrianTech/cb-ml.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-demoweb")
  ExternalProject_Add(cb-demoweb
    PREFIX $ENV{CB}/cb-demoweb
    GIT_REPOSITORY https://github.com/CambrianTech/cb-demoweb.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-deepweb")
  ExternalProject_Add(cb-deepweb
    PREFIX $ENV{CB}/cb-deepweb
    GIT_REPOSITORY https://github.com/CambrianTech/cb-deepweb.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "cb-unity")
  ExternalProject_Add(cb-unity
    PREFIX $ENV{CB}/cb-unity
    GIT_REPOSITORY https://github.com/CambrianTech/cb-unity.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )

  ExternalProject_Add(arcore-unity
    PREFIX ${third_party_dir}/unity/arcore-unity-sdk
    GIT_REPOSITORY  https://github.com/google-ar/arcore-unity-sdk.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

  ExternalProject_Add(arkit-unity
    PREFIX ${third_party_dir}/unity/unity-arkit-plugin
    HG_REPOSITORY https://bitbucket.org/Unity-Technologies/unity-arkit-plugin
    HG_TAG spring2018_update
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    BUILD_ALWAYS 0
    DOWNLOAD_NO_PROGRESS 0
    )

  add_dependencies(cb-unity arcore-unity arkit-unity)
endif()

if (download STREQUAL "cb-assets")
  ExternalProject_Add(cb-assets
    PREFIX $ENV{CB}/cb-assets
    GIT_REPOSITORY https://github.com/CambrianTech/assets.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()


if (download STREQUAL "opencv")
  ExternalProject_Add(opencv
    PREFIX ${third_party_dir}/opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG "3.3.1"
    INSTALL_DIR ${opencv_INSTALL_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )

  ExternalProject_Add(opencv_contrib
    PREFIX ${third_party_dir}/opencv_contrib
    GIT_REPOSITORY https://github.com/opencv/opencv_contrib.git
    GIT_TAG "3.3.1"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )
  add_dependencies(opencv opencv_contrib)
endif()

if (download STREQUAL "bgfx")
  ExternalProject_Add(bgfx
    PREFIX ${third_party_dir}/bgfx
    GIT_REPOSITORY https://github.com/CambrianTech/bgfx.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )

  ExternalProject_Add(bx
    PREFIX ${third_party_dir}/bgfx
    GIT_REPOSITORY https://github.com/CambrianTech/bx.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )

  ExternalProject_add(bimg
    PREFIX ${third_party_dir}/bgfx
    GIT_REPOSITORY https://github.com/CambrianTech/bimg.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )

  add_dependencies(bgfx bx bimg)
endif()

if (download STREQUAL "libyuv")
  ExternalProject_Add(libyuv
    PREFIX ${third_party_dir}/libyuv
    GIT_REPOSITORY https://chromium.googlesource.com/libyuv/libyuv
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
endif()

if (download STREQUAL "tensorflow")
  ExternalProject_Add(tensorflow
    PREFIX ${third_party_dir}/tensorflow
    GIT_REPOSITORY https://github.com/tensorflow/tensorflow
    #GIT_TAG "cambrian.v1.3.0"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )
endif()

if (download STREQUAL "keras")
  ExternalProject_Add(keras
    PREFIX ${third_party_dir}/keras
    GIT_REPOSITORY https://github.com/fchollet/keras
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
 endif()
 
 if (download STREQUAL "eigen")
  ExternalProject_Add(eigen
    PREFIX ${third_party_dir}/eigen
    HG_REPOSITORY https://bitbucket.org/eigen/eigen
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    BUILD_ALWAYS 0
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )
endif()

if (download STREQUAL "jsoncpp")
  ExternalProject_Add(jsoncpp
    PREFIX ${third_party_dir}/jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )
endif()

if (download STREQUAL "pcl")
  ExternalProject_Add(pcl
    PREFIX ${third_party_dir}/pcl
    GIT_REPOSITORY https://github.com/PointCloudLibrary/pcl.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )
 endif()

if (download STREQUAL "flann")
  ExternalProject_Add(flann
    PREFIX ${third_party_dir}/flann
    GIT_REPOSITORY https://github.com/mariusmuja/flann
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
 endif()

if (download STREQUAL "boost")
  ExternalProject_Add(boost
    PREFIX ${third_party_dir}/boost
    GIT_REPOSITORY https://github.com/boostorg/boost
    GIT_TAG boost-1.64.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_PROGRESS 0
    GIT_PROGRESS 1
  )
 endif()

if (download STREQUAL "liblzma")
  ExternalProject_Add(liblzma
    PREFIX ${third_party_dir}/liblzma
    GIT_REPOSITORY https://github.com/hunter-packages/lzma
    GIT_TAG "hunter-5.2.3"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
 endif()

 if (download STREQUAL "libjpeg")
  ExternalProject_Add(libjpeg
    PREFIX ${third_party_dir}/libjpeg
    GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
 endif()
 
if (download STREQUAL "bzip2")
  ExternalProject_Add(bzip2
    PREFIX ${third_party_dir}/bzip2
    GIT_REPOSITORY https://github.com/osrf/bzip2_cmake
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
 endif()



#message(FATAL_ERROR " Cannot download ${download}. Be sure it is added to ${CMAKE_CURRENT_LIST_FILE}")



add_executable(dummy dummy.cpp)
