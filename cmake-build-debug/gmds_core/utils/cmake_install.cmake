# Install script for directory: /Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/libgmds_utils.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_utils.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_utils.a")
    execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libgmds_utils.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gmds/utils" TYPE FILE FILES
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/BitVector.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/CommonFlags.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/CommonTypes.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/Exception.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/IndexedVector.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/LocalCellTopology.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/Marks32.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/SmartVector.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/Variable.h"
    "/Users/paulbourmaud/Projects/gecko/gecko2/gmds_core/utils/inc/gmds/utils/VariableManager.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst/cmake_install.cmake")
endif()

