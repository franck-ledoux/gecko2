#==============================================================================
# LIBRARY DEFINTION (SOURCE FILES)
#==============================================================================
set(GECKO_LIB           gmds_ig)
set(GECKO_LIB_PREFIX    gmds/ig)

set(GECKO_INC
        inc/gmds/ig/Cell.h
        inc/gmds/ig/CellGroup.h
        inc/gmds/ig/Edge.h
        inc/gmds/ig/EdgeContainer.h
        inc/gmds/ig/Face.h
        inc/gmds/ig/FaceContainer.h
        inc/gmds/ig/Mesh.h
        inc/gmds/ig/MeshDoctor.h
        inc/gmds/ig/Node.h
        inc/gmds/ig/NodeContainer.h
        inc/gmds/ig/Region.h
        inc/gmds/ig/RegionContainer.h
)
set(GECKO_SRC
        src/Cell.cpp
        src/Edge.cpp
        src/EdgeContainer.cpp
        src/Face.cpp
        src/FaceContainer.cpp
        src/Mesh.cpp
        src/MeshDoctor.cpp
        src/Node.cpp
        src/NodeContainer.cpp
        src/Region.cpp
        src/RegionContainer.cpp
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
        gmds_math)

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
