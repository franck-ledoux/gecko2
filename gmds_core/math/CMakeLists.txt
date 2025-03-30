#==============================================================================
# LIBRARY DEFINTION (SOURCE FILES)
#==============================================================================
set(GECKO_LIB           gmds_math)
set(GECKO_LIB_PREFIX    gmds/math)

set(GECKO_INC
        inc/gmds/math/AxisAngleRotation.h
        inc/gmds/math/Chart.h
        inc/gmds/math/Constants.h
        inc/gmds/math/Cross.h
        inc/gmds/math/Cross2D.h
        inc/gmds/math/Hexahedron.h
        inc/gmds/math/Line.h
        inc/gmds/math/Matrix.h
        inc/gmds/math/Numerics.h
        inc/gmds/math/Orientation.h
        inc/gmds/math/Plane.h
        inc/gmds/math/Point.h
        inc/gmds/math/Prism3.h
        inc/gmds/math/Pyramid.h
        inc/gmds/math/Quadrilateral.h
        inc/gmds/math/Quaternion.h
        inc/gmds/math/Ray.h
        inc/gmds/math/Segment.h
        inc/gmds/math/Tetrahedron.h
        inc/gmds/math/Triangle.h
        inc/gmds/math/Vector.h
        inc/gmds/math/VectorDyn.h
)
set(GECKO_SRC
        src/AxisAngleRotation.cpp
        src/Chart.cpp
        src/Cross.cpp
        src/Cross2D.cpp
        src/Hexahedron.cpp
        src/Line.cpp
        src/Numerics.cpp
        src/Orientation.cpp
        src/Plane.cpp
        src/Point.cpp
        src/Prism3.cpp
        src/Pyramid.cpp
        src/Quadrilateral.cpp
        src/Quaternion.cpp
        src/Ray.cpp
        src/Segment.cpp
        src/Tetrahedron.cpp
        src/Triangle.cpp
        src/Vector.cpp
        src/VectorDyn.cpp
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
        gmds_utils
        Predicates_psm
        )

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
