cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME rack)
project(${PROJECT_NAME})

include(CMakeDependentOption)
include(UseJava)

set(CMAKE_CXX_FLAGS "-Wall -O2 -pipe -fstrict-aliasing" CACHE STRING "CXX default compiler flags for RACK." FORCE)
set(CMAKE_C_FLAGS "-Wall -O2 -pipe -fstrict-aliasing" CACHE STRING "C default compiler flags for RACK." FORCE)

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix \${prefix})
set(bindir \${exec_prefix}/bin)
set(libdir \${exec_prefix}/lib)
set(datarootdir \${prefix}/share)
set(includedir \${prefix}/include)

#option(ExternalDependencies_libraw1394 "Enable libraw1394 support")
#option(ExternalDependencies_libdc1394 "Enable lidc1394 support")
#option(ExternalDependencies_libjpeg "Enable libjpeg support")

#======================================================================
# check support
#======================================================================



#======================================================================
#   JAVA Part
#======================================================================

#-----------------------------------------------------------------
# rack java
#-----------------------------------------------------------------

option(Build_RACK_Java "RACK Java" ON)

#-----------------------------------------------------------------
# rack java gui
#-----------------------------------------------------------------

CMAKE_DEPENDENT_OPTION(Build_RACK_JavaGUI "RACK Java GUI" ON "Build_RACK_Java" OFF)

#-----------------------------------------------------------------
# javac & jar
#-----------------------------------------------------------------

unset(RACK_JAVA_SRC_FILES)

if(Build_RACK_Java)
  find_package(Java)
  if(NOT Java_FOUND)
    message(FATAL_ERROR "Java (Development Kit) not found!")
  endif(NOT Java_FOUND)
  message("-- Found javac: ${Java_JAVAC_EXECUTABLE}")
  message("-- Found jar: ${Java_JAR_EXECUTABLE}")
endif(Build_RACK_Java)


set(PKG_INCLUDE_DIR "include/rack")

add_subdirectory(config)
include_directories("${PROJECT_BINARY_DIR}/config")

include_directories("${PROJECT_SOURCE_DIR}")

set(RACK_LIBS "${PROJECT_NAME}")

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



#-----------------------------------------------------------------
# rack java: add and install rack.jar
#-----------------------------------------------------------------

if(Build_RACK_Java)

  if(Build_RACK_JavaGUI)
    add_jar(${PROJECT_NAME}_jar ${RACK_JAVA_SRC_FILES} MANIFEST gui/gui.manifest OUTPUT_NAME ${PROJECT_NAME})
  else(Build_RACK_JavaGUI)
    add_jar(${PROJECT_NAME}_jar ${RACK_JAVA_SRC_FILES} OUTPUT_NAME ${PROJECT_NAME})
  endif(Build_RACK_JavaGUI)

  install_jar(${PROJECT_NAME}_jar DESTINATION "share/rack")

endif(Build_RACK_Java)

