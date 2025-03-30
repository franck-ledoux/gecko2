#==============================================================================
cmake_minimum_required(VERSION 3.14)
project(gecko2)
#==============================================================================
# Compilation options mainly for testing purpose
#==============================================================================
option(WITH_TEST "Build test"   ON)
#==============================================================================
add_subdirectory(external/Catch2-3.8.0)
add_subdirectory(external/Predicates_psm)
include_directories(${CMAKE_SOURCE_DIR}/external/eigen-3.4.0)

find_package(Boost REQUIRED)
find_package(CGAL REQUIRED)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(external/nlohmann_json)
#==============================================================================
set(TEST_SAMPLES_DIR "${CMAKE_SOURCE_DIR}/test_data")
enable_testing()
include(CTest)
include(Catch)
#==============================================================================
add_subdirectory(gmds_core)
add_subdirectory(cblock)
add_subdirectory(mcts)

add_executable(gecko2 main_blocker.cpp)
target_link_libraries(gecko2 PRIVATE gmds_cad gmds_ig gmds_cadfac cblock mcts)

