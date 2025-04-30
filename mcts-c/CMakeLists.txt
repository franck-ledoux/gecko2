#==============================================================================
# LIBRARY DEFINTION (SOURCE FILES)
#==============================================================================
set(GECKO_LIB           mcts)
set(GECKO_LIB_PREFIX    mcts)

set(GECKO_INC
        inc/gecko/mcts/BlockingAction.h
        inc/gecko/mcts/BlockingRewardFunction.h
        inc/gecko/mcts/BlockingState.h
        inc/gecko/mcts/IAction.h
        inc/gecko/mcts/IRewardFunction.h
        inc/gecko/mcts/IState.h
        inc/gecko/mcts/MCTSAgent.h
        inc/gecko/mcts/MCTSLoop.h
        inc/gecko/mcts/MCTSSelectionFunction.h
        inc/gecko/mcts/MCTSTree.h
)
set(GECKO_SRC
        src/BlockingAction.cpp
        src/BlockingRewardFunction.cpp
        src/BlockingState.cpp
        src/MCTSAgent.cpp
        src/MCTSLoop.cpp
        src/MCTSSelectionFunction.cpp
        src/MCTSTree.cpp
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
        cblock
        nlohmann_json::nlohmann_json)

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

