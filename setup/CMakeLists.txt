cmake_minimum_required(VERSION 3.7)

include(ExternalProject)

ExternalProject_Add(
    Result
    GIT_REPOSITORY https://github.com/gmbeard/result.git
    INSTALL_DIR ${INSTALL_DEPS_TO}
    CMAKE_ARGS
        -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    Catch2
    GIT_REPOSITORY https://github.com/CatchOrg/Catch2.git
    INSTALL_DIR ${INSTALL_DEPS_TO}
    CMAKE_ARGS
        -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
