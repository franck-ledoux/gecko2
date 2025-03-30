#==============================================================================
# LIBRARY DEFINTION (SOURCE FILES)
#==============================================================================
set(GECKO_LIB           gmds_cadfac)
set(GECKO_LIB_PREFIX    gmds/cadfac)

set(GECKO_INC
        inc/gmds/cadfac/FACCurve.h
        inc/gmds/cadfac/FACManager.h
        inc/gmds/cadfac/FACPoint.h
        inc/gmds/cadfac/FACSurface.h
        inc/gmds/cadfac/FACVolume.h
)
set(GECKO_SRC
        src/FACCurve.cpp
        src/FACManager.cpp
        src/FACPoint.cpp
        src/FACSurface.cpp
        src/FACVolume.cpp
)
#==============================================================================
add_library(${GECKO_LIB} ${GECKO_INC} ${GECKO_SRC})
#==============================================================================
# TARGET DEFINITION
#==============================================================================
include(GNUInstallDirs)

target_compile_features(${GECKO_LIB} PUBLIC cxx_std_14)


#LIBRARY TO INSTALL
target_link_libraries(${GECKO_LIB}
        PUBLIC
        gmds_ig
        gmds_cad
        gmds_igalgo
        CGAL::CGAL)

# INCLUDE TO INSTALL
target_include_directories(${GECKO_LIB} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)
set_target_properties(${GECKO_LIB} PROPERTIES PUBLIC_HEADER "${GECKO_INC}")

install(TARGETS ${GECKO_LIB}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${GECKO_LIB_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


#==============================================================================
if(WITH_TEST)
    add_subdirectory(tst)
endif(WITH_TEST)
