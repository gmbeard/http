cmake_minimum_required(VERSION 3.7)

include(ExternalProject)

get_filename_component(
    DEPENDENCY_INSTALL_DIR
    ${CMAKE_CURRENT_LIST_DIR}
    PATH
)

set(DEPENDENCY_INSTALL_DIR
    ${DEPENDENCY_INSTALL_DIR}/deps
)

ExternalProject_Add(
    Result
    GIT_REPOSITORY https://github.com/gmbeard/result.git
    INSTALL_DIR ${DEPENDENCY_INSTALL_DIR}
    CMAKE_ARGS
        -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    Catch2
    GIT_REPOSITORY https://github.com/CatchOrg/Catch2.git
    INSTALL_DIR ${DEPENDENCY_INSTALL_DIR}
    CMAKE_ARGS
        -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
