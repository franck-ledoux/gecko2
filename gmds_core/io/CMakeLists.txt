#==============================================================================
# LIBRARY DEFINTION (SOURCE FILES)
#==============================================================================
set(GECKO_LIB           gmds_io)
set(GECKO_LIB_PREFIX    gmds/io)

set(GECKO_INC
        inc/gmds/io/GMSHWriter.h
        inc/gmds/io/IGMeshIOService.h
        inc/gmds/io/IMeshIOService.h
        inc/gmds/io/IReader.h
        inc/gmds/io/IWriter.h
        inc/gmds/io/MeditReader.h
        inc/gmds/io/VTKReader.h
        inc/gmds/io/VTKWriter.h
)
set(GECKO_SRC
        src/GMSHWriter.cpp
        src/IGMeshIOService.cpp
        src/IMeshIOService.cpp
        src/IReader.cpp
        src/IWriter.cpp
        src/MeditReader.cpp
        src/VTKReader.cpp
        src/VTKWriter.cpp
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
        gmds_ig)

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
