#==============================================================================
set(GECKO_LIB_TEST test_${GECKO_LIB})

add_executable(${GECKO_LIB_TEST}
        fac_manager_tests.cpp
        geom_linker_tests.cpp
        geom_topology_tests.cpp
)
target_compile_definitions(${GECKO_LIB_TEST} PRIVATE TEST_SAMPLES_DIR="${TEST_SAMPLES_DIR}")
#==============================================================================
target_link_libraries(${GECKO_LIB_TEST} PRIVATE
        ${GECKO_LIB}
        gmds_io
        Catch2::Catch2WithMain)
#==============================================================================
catch_discover_tests(${GECKO_LIB_TEST})