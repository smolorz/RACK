cmake_minimum_required(VERSION 2.8)

include(CMakeDependentOption)
include(UseJava)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)

set(PROJECT_NAME rack)
project(${PROJECT_NAME})


#======================================================================
#   Build options
#======================================================================

#--------------------------------------------------
# Xenomai
#--------------------------------------------------

option(Build_OS_Xenomai "Build RACK for Xenomai (otherwise Linux)" OFF)
#-----
option(Build_KMod "Build Xenomai kernel modules (e.g. TiMS)" OFF)

if(Build_KMod)
  set(Build_OS_Xenomai ON CACHE
      BOOL "Build RACK for Xenomai (otherwise Linux)" FORCE)
endif(Build_KMod)

#--------------------------------------------------
# Java
#--------------------------------------------------

option(Build_RACK_Java "RACK Java" ON)
#-----
option(Build_RACK_JavaGUI "RACK Java GUI" ON)

if(Build_RACK_JavaGUI)
  set(Build_RACK_Java ON CACHE BOOL "RACK Java" FORCE)
endif(Build_RACK_JavaGUI)

#--------------------------------------------------
# Doxygen
#--------------------------------------------------

option(Build_Doxygen "Generate RACK Doxygen API documentation" OFF)


#----------------------------------------------------------------------
# External Dependencies
#----------------------------------------------------------------------

option(Build/ExternalDependencies_RTnet "Enable RTnet support" OFF)

if(Build/ExternalDependencies_RTnet)
  set(Build_OS_Xenomai ON CACHE
      BOOL "Build RACK for Xenomai (otherwise Linux)" FORCE)
  set(Build_KMod ON CACHE
      BOOL  "Build Xenomai kernel modules (e.g. TiMS)" FORCE)
endif(Build/ExternalDependencies_RTnet)
#----------
option(Build/ExternalDependencies_libraw1394
       "Enable libraw1394 support" OFF)
#----------
option(Build/ExternalDependencies_libdc1394 "Enable libdc1394 support" OFF)

if(Build/ExternalDependencies_libdc1394)
  set(Build/ExternalDependencies_libraw1394 ON CACHE
      BOOL "Enable libraw1394 support" FORCE)
endif(Build/ExternalDependencies_libdc1394)
#----------
option(Build/ExternalDependencies_libjpeg "Enable libjpeg support" OFF)
#----------
option(Build/ExternalDependencies_zlib "Enable zlib support" OFF)
#----------
option(Build/ExternalDependencies_libpng "Enable libpng support" OFF)

if(Build/ExternalDependencies_libpng)
  set(Build/ExternalDependencies_zlib ON CACHE
      BOOL "Enable zlib support" FORCE)
endif(Build/ExternalDependencies_libpng)
#----------
option(Build/ExternalDependencies_OpenCV "Enable OpenCV support" OFF)

if(Build/ExternalDependencies_OpenCV)
  set(Build/ExternalDependencies_libjpeg ON CACHE
      BOOL "Enable libjpeg support" FORCE)
  set(Build/ExternalDependencies_libpng ON CACHE
      BOOL "Enable libpng support" FORCE)
  set(Build/ExternalDependencies_libraw1394 ON CACHE
      BOOL "Enable libraw1394 support" FORCE)
  set(Build/ExternalDependencies_libdc1394 ON CACHE
      BOOL "Enable libdc1394 support" FORCE)
endif(Build/ExternalDependencies_OpenCV)
#----------
option(Build/ExternalDependencies_gsl "Enable gsl support" OFF)
#----------
option(Build/ExternalDependencies_libusb "Enable libusb support" OFF)
#----------
option(Build/ExternalDependencies_boost "Enable boost support" OFF)
#----------
option(Build/ExternalDependencies_pcl "Enable pcl support" OFF)

if(Build/ExternalDependencies_pcl)
  set(Build/ExternalDependencies_boost ON CACHE
      BOOL "Enable boost support" FORCE)
endif(Build/ExternalDependencies_pcl)
#----------
option(Build/ExternalDependencies_OpenNI "Enable OpenNI support" OFF)

if(Build/ExternalDependencies_OpenNI)
  set(Build/ExternalDependencies_libusb ON CACHE
      BOOL "Enable libusb support" FORCE)
endif(Build/ExternalDependencies_OpenNI)


#======================================================================
#   Linux Kernel with Xenomai
#======================================================================

if(Build_OS_Xenomai AND Build_KMod)

  message("-- Linux kernel source directory: ${Linux_Kernel_SRC}")

  #--------------------------------------------------
  # check config file
  #--------------------------------------------------

  if (NOT EXISTS "${Linux_Kernel_SRC}/.config")
    message(FATAL_ERROR
            "FATAL: No Linux kernel .config found in ${Linux_Kernel_SRC}.")
  endif()

  message(STATUS "Linux kernel .config found.")

  #--------------------------------------------------
  # check kernel version
  #--------------------------------------------------

  if(CMAKE_SYSTEM_VERSION VERSION_GREATER 2.6 AND
     CMAKE_SYSTEM_VERSION VERSION_LESS 4)
    message("-- Linux kernel version string: ${CMAKE_SYSTEM_VERSION}")
  else()
    message(FATAL_ERROR "*** Unsupported kernel version "
                         ${CMAKE_SYSTEM_VERSION})
  endif()

  #--------------------------------------------------
  # check needed Xenomai and skins in linux config
  #--------------------------------------------------

  file(STRINGS "${Linux_Kernel_SRC}/.config" KERNEL_CONFIG_XENO
       REGEX CONFIG_XENO)

  if(NOT KERNEL_CONFIG_XENO MATCHES "CONFIG_XENOMAI=y")
    message(FATAL_ERROR "*** Xenomai not enabled in kernel config")
  endif()

  if(NOT KERNEL_CONFIG_XENO MATCHES "CONFIG_XENO_SKIN_NATIVE=y")
    message(FATAL_ERROR "*** Xenomai native skin not enabled in kernel config")
  endif()

  if(NOT KERNEL_CONFIG_XENO MATCHES "CONFIG_XENO_SKIN_RTDM=y")
    message(FATAL_ERROR "*** Xenomai RTDM skin not enabled in kernel config")
  endif()

  message(STATUS "Checking kernel config for Xenomai: [OK]")

endif(Build_OS_Xenomai AND Build_KMod)


#======================================================================
#   Xenomai userspace
#======================================================================

if(Build_OS_Xenomai)

  #--------------------------------------------------
  # try to find xeno-config
  #--------------------------------------------------

  find_program(XENO_USER_CONFIG xeno-config CMAKE_FIND_ROOT_PATH_BOTH)

  if(NOT XENO_USER_CONFIG)
    message(FATAL_ERROR "*** xeno-config not found!")
  endif()

  message(STATUS "Found xeno-config: ${XENO_USER_CONFIG}")

  #--------------------------------------------------
  # check Xenomai version
  #--------------------------------------------------

  execute_process(COMMAND ${XENO_USER_CONFIG} --version
                  OUTPUT_VARIABLE XENO_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(XENO_VERSION VERSION_LESS 2.6 OR XENO_VERSION VERSION_GREATER 2.6.9)
    message(FATAL_ERROR "*** Unsupported Xenomai version ${XENO_VERSION}")
  else()
    message("-- Xenomai version: ${XENO_VERSION}")
  endif()

endif(Build_OS_Xenomai)


#======================================================================
#   header / library checks
#======================================================================

unset(RACK_SUPPORTS)

#----- Xenomai ------------------------

if(Build_OS_Xenomai)

  execute_process(COMMAND ${XENO_USER_CONFIG} --skin=native --cflags
                  OUTPUT_VARIABLE XENOMAI_CFLAGS
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(XENOMAI_LIBS "native rtdm xenomai pthread rt")

endif(Build_OS_Xenomai)

#----- Linux --------------------------

set(LINUX_LIBS "pthread")

#----- RTnet --------------------------

if(Build/ExternalDependencies_RTnet)

  find_path(RTNET_INCLUDE_PATH rtnet.h)
  if(NOT RTNET_INCLUDE_PATH)
    message(FATAL_ERROR "*** Cannot find rtnet.h")
  endif()

  set(CMAKE_REQUIRED_INCLUDES "${RTNET_INCLUDE_PATH}")
  CHECK_INCLUDE_FILE(rtnet.h RTNET_H_FOUND ${XENOMAI_CFLAGS})
  if(NOT RTNET_H_FOUND)
    message(FATAL_ERROR "*** Cannot include rtnet.h")
  endif()

  set(RACK_SUPPORTS "rtnet ${RACK_SUPPORTS}")

endif(Build/ExternalDependencies_RTnet)

#----- libraw1394 ---------------------

if(Build/ExternalDependencies_libraw1394)

  find_path(LIBRAW1394_INCLUDE_PATH "libraw1394/raw1394.h")
  if(NOT LIBRAW1394_INCLUDE_PATH)
    message(FATAL_ERROR "*** Cannot find libraw1394 header files.")
  endif()

  set(CMAKE_REQUIRED_INCLUDES "${LIBRAW1394_INCLUDE_PATH}")
  CHECK_INCLUDE_FILE("libraw1394/raw1394.h" RAW1394_H_FOUND)
  if(NOT RAW1394_H_FOUND)
    message(FATAL_ERROR "*** Cannot include raw1394.h")
  endif()

  find_library(LIBRAW1394_LIB "raw1394")
  if(NOT LIBRAW1394_LIB)
    message(FATAL_ERROR "*** Cannot find libraw1394.")
  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${LIBRAW1394_LIB})
  CHECK_FUNCTION_EXISTS(raw1394_new_handle LIBRAW1394_FOUND)
  if(NOT LIBRAW1394_FOUND)
    message(FATAL_ERROR "*** Cannot link to raw1394_new_handle() of libraw1394.")
  endif()

  set(LIBRAW1394_LIBS "raw1394")

  set(RACK_SUPPORTS "libraw1394 ${RACK_SUPPORTS}")

endif(Build/ExternalDependencies_libraw1394)

#----- libdc1394_control --------------

if(Build/ExternalDependencies_libdc1394)

  find_path(LIBDC1394_INCLUDE_PATH "dc1394/dc1394.h")
  if(NOT LIBDC1394_INCLUDE_PATH)
    message(FATAL_ERROR "*** Cannot find libdc1394 header files.")
  endif()

  set(CMAKE_REQUIRED_INCLUDES ${LIBDC1394_INCLUDE_PATH})
  CHECK_INCLUDE_FILE("dc1394/dc1394.h" DC1394_H_FOUND)
  if(NOT DC1394_H_FOUND)
    message(FATAL_ERROR "*** Cannot include dc1394.h")
  endif()

  find_library(LIBDC1394_LIB "dc1394")
  if(NOT LIBDC1394_LIB)
    message(FATAL_ERROR "*** Cannot find libdc1394.")
  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${LIBDC1394_LIB} ${LIBRAW1394_LIB})
  CHECK_FUNCTION_EXISTS(dc1394_camera_enumerate LIBDC1394_FOUND)
  if(NOT LIBDC1394_FOUND)
    message(FATAL_ERROR "*** Cannot link to dc1394_camera_enumerate() of libdc1394.")
  endif()

  set(LIBDC1394_LIBS "dc1394")

  set(RACK_SUPPORTS "libdc1394 ${RACK_SUPPORTS}")

endif(Build/ExternalDependencies_libdc1394)

#----- libjpeg ------------------------

if(Build/ExternalDependencies_libjpeg)

  find_package(JPEG)
  if(NOT JPEG_FOUND)
    message(FATAL_ERROR "libjpeg not found!")
  endif(NOT JPEG_FOUND)

  set(CMAKE_REQUIRED_INCLUDES "${JPEG_INCLUDE_DIR}")

#  CHECK_INCLUDE_FILE("jpeglib.h" JPEGLIB_H_FOUND)
#  if(NOT JPEGLIB_H_FOUND)
#    message(FATAL_ERROR "*** Cannot include jpeglib.h")
#  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARIES})
  CHECK_FUNCTION_EXISTS(jpeg_start_compress LIBJPEG_FOUND)
  if(NOT LIBJPEG_FOUND)
    message(FATAL_ERROR "*** Cannot link to jpeg_start_compress() of libjpeg.")
  endif()

  set(LIBJPEG_LIBS "jpeg")

  set(RACK_SUPPORTS "libjpeg ${RACK_SUPPORTS}") 

endif(Build/ExternalDependencies_libjpeg)

#----- zlib ---------------------------

if(Build/ExternalDependencies_zlib)

  find_package(ZLIB)
  if(NOT TARGET ZLIB::ZLIB)
    message(FATAL_ERROR "zlib not found!")
  endif()

endif(Build/ExternalDependencies_zlib)

#----- libpng -------------------------

if(Build/ExternalDependencies_libpng)

  find_package(PNG)
  if(NOT TARGET PNG::PNG)
    message(FATAL_ERROR "libpng not found!")
  endif()

  set(CMAKE_REQUIRED_INCLUDES "${PNG_INCLUDE_DIRS}")
  CHECK_INCLUDE_FILE("png.h" PNG_H_FOUND)
  if(NOT PNG_H_FOUND)
    message(FATAL_ERROR "*** Cannot include png.h")
  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
  CHECK_FUNCTION_EXISTS(png_write_end LIBPNG_FOUND)
  if(NOT LIBPNG_FOUND)
    message(FATAL_ERROR "*** Cannot link to png_write_end() of libpng.")
  endif()

  set(RACK_SUPPORTS "libpng ${RACK_SUPPORTS}")

endif(Build/ExternalDependencies_libpng)

#----- OpenCV -------------------------

if(Build/ExternalDependencies_OpenCV)

  find_package(OpenCV REQUIRED core highgui objdetect legacy)
  message(STATUS "Found OpenCV ${OpenCV_VERSION} in ${OpenCV_INSTALL_PATH}")

  set(CMAKE_REQUIRED_INCLUDES ${OpenCV_INCLUDE_DIRS})
  CHECK_INCLUDE_FILE_CXX("cv.h" CV_H_FOUND)
  if(NOT CV_H_FOUND)
    message(FATAL_ERROR "*** Cannot include cv.h")
  endif()
  CHECK_INCLUDE_FILE_CXX("cxcore.h" CXCORE_H_FOUND)
  if(NOT CXCORE_H_FOUND)
    message(FATAL_ERROR "*** Cannot include cxcore.h")
  endif()
  CHECK_INCLUDE_FILE_CXX("highgui.h" HIGHGUI_H_FOUND)
  if(NOT HIGHGUI_H_FOUND)
    message(FATAL_ERROR "*** Cannot include highgui.h")
  endif()

  set(OPENCV_LIBS "opencv_legacy opencv_objdetect")

  set(RACK_SUPPORTS "opencv ${RACK_SUPPORTS}")

endif(Build/ExternalDependencies_OpenCV)

#----- gsl ----------------------------



#----- libusb -------------------------


#----- boost --------------------------



#----- pcl ----------------------------



#----- OpenNI -------------------------






#--------------------------------------------------
# Unset temporary cache variables used for checks
#--------------------------------------------------

unset(XENO_USER_CONFIG CACHE)
unset(RTNET_INCLUDE_PATH CACHE)
unset(LIBRAW1394_INCLUDE_PATH CACHE)
unset(LIBRAW1394_LIB CACHE)
unset(LIBDC1394_LIB CACHE)
unset(LIBDC1394_INCLUDE_PATH CACHE)
unset(OpenCV_DIR CACHE)



#======================================================================
#   Set hard-coded compiler flags (no CMAKE_BUILD_TYPE)
#======================================================================

set(CMAKE_CXX_FLAGS "-Wall -O2 -pipe -fstrict-aliasing" CACHE
    STRING "CXX default compiler flags for RACK." FORCE)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}" CACHE
    STRING "C default compiler flags for RACK." FORCE)


#======================================================================
#   Set old autotools variables (still used by scripts)
#======================================================================

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix \${prefix})
set(bindir \${exec_prefix}/bin)
set(libdir \${exec_prefix}/lib)
set(datarootdir \${prefix}/share)
set(includedir \${prefix}/include)


#======================================================================
#   Find JAVA
#======================================================================

unset(RACK_JAVA_SRC_FILES)

if(Build_RACK_Java)

  find_package(Java)

  if(NOT Java_JAVAC_EXECUTABLE)
    message(FATAL_ERROR "Java compiler not found! Did you install a JDK?")
  endif()

  if(NOT Java_JAR_EXECUTABLE)
    message(FATAL_ERROR "Java archiver (jar) not found!")
  endif()

  message(STATUS "Found javac: ${Java_JAVAC_EXECUTABLE}")
  message(STATUS "Found jar: ${Java_JAR_EXECUTABLE}")

endif(Build_RACK_Java)


#======================================================================
#   Include Directory and RACK Libs
#======================================================================

include_directories("${PROJECT_SOURCE_DIR}")
set(RACK_LIBS "${PROJECT_NAME}")

#======================================================================
#   Subdirectories
#======================================================================

set(PKG_INCLUDE_DIR "include/rack")

add_subdirectory(config)
include_directories("${PROJECT_BINARY_DIR}/config")

add_subdirectory(main)
add_subdirectory(drivers)
add_subdirectory(navigation)
#add_subdirectory(doc)
add_subdirectory(control)
add_subdirectory(perception)
add_subdirectory(tools)
#add_subdirectory(skel)
add_subdirectory(gui)
add_subdirectory(examples)
add_subdirectory(scripts)


#======================================================================
#   JAVA: Add and install rack.jar
#======================================================================

if(Build_RACK_Java)

  if(Build_RACK_JavaGUI)
    add_jar(${PROJECT_NAME}_jar ${RACK_JAVA_SRC_FILES}
            MANIFEST gui/gui.manifest OUTPUT_NAME ${PROJECT_NAME})
  else(Build_RACK_JavaGUI)
    add_jar(${PROJECT_NAME}_jar ${RACK_JAVA_SRC_FILES}
            OUTPUT_NAME ${PROJECT_NAME})
  endif(Build_RACK_JavaGUI)

  install_jar(${PROJECT_NAME}_jar DESTINATION "share/rack")

endif(Build_RACK_Java)

