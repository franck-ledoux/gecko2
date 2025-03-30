#==============================================================================
set(GECKO_LIB_TEST test_${GECKO_LIB})

add_executable(${GECKO_LIB_TEST} boolean_mark_tests.cpp doctor_tests.cpp edge_tests.cpp mesh_tests.cpp)
#==============================================================================
target_link_libraries(${GECKO_LIB_TEST} PRIVATE  ${GECKO_LIB} Catch2::Catch2WithMain)
#==============================================================================
catch_discover_tests(${GECKO_LIB_TEST})