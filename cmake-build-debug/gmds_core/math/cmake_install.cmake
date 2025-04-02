# Install script for directory: /Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/math/libgmds_math.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_math.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_math.a")
    execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_math.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gmds/math" TYPE FILE FILES
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/AxisAngleRotation.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Chart.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Constants.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Cross.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Cross2D.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Hexahedron.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Line.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Matrix.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Numerics.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Orientation.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Plane.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Point.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Prism3.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Pyramid.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Quadrilateral.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Quaternion.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Ray.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Segment.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Tetrahedron.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Triangle.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/Vector.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/math/inc/gmds/math/VectorDyn.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/math/tst/cmake_install.cmake")
endif()

