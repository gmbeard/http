cmake_minimum_required(VERSION 3.7)

include(ExternalProject)

find_package(Result QUIET)
if(NOT Result_FOUND)
    ExternalProject_Add(
        Result
        GIT_REPOSITORY https://github.com/gmbeard/result.git
        INSTALL_DIR ${INSTALL_DEPS_TO}
        CMAKE_ARGS
            -DINSTALL_DEPS_TO=<INSTALL_DIR>
            -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
endif()

find_package(Catch2 QUIET)
if(NOT Catch2_FOUND)
    ExternalProject_Add(
        Catch2
        GIT_REPOSITORY https://github.com/CatchOrg/Catch2.git
        INSTALL_DIR ${INSTALL_DEPS_TO}
        CMAKE_ARGS
            -DCMAKE_PREFIX_PATH=<INSTALL_DIR>
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
endif()
