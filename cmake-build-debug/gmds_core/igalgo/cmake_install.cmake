# Install script for directory: /Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/igalgo/libgmds_igalgo.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_igalgo.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_igalgo.a")
    execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_igalgo.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gmds/igalgo" TYPE FILE FILES
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/BoundaryExtractor2D.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/BoundaryExtractor3D.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/BoundaryOperator.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/BoundaryOperator2D.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/GridBuilder.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/SurfaceReorient.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/igalgo/inc/gmds/igalgo/THexBuilder.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/igalgo/tst/cmake_install.cmake")
endif()

