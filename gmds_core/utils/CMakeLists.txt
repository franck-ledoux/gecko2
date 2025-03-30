#==============================================================================
# LIBRARY DEFINTION (SOURCE FILES)
#==============================================================================
set(GECKO_LIB           gmds_utils)
set(GECKO_LIB_PREFIX    gmds/utils)

set(GECKO_INC
        inc/gmds/utils/BitVector.h
        inc/gmds/utils/CommonFlags.h
        inc/gmds/utils/CommonTypes.h
        inc/gmds/utils/Exception.h
        inc/gmds/utils/IndexedVector.h
        inc/gmds/utils/Marks32.h
        inc/gmds/utils/SmartVector.h
        inc/gmds/utils/Variable.h
        inc/gmds/utils/VariableManager.h
)
set(GECKO_SRC
        src/BitVector.cpp
        src/CommonTypes.cpp
        src/VariableManager.cpp
)
#==============================================================================
add_library(${GECKO_LIB} ${GECKO_INC} ${GECKO_SRC})
#==============================================================================
# TARGET DEFINITION
#==============================================================================
include(GNUInstallDirs)

target_compile_features(${GECKO_LIB} PUBLIC cxx_std_14)

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
