cmake_minimum_required(VERSION 3.7)

set(CMAKE_PREFIX_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
)

project(http CXX C)

find_package(Result REQUIRED)
add_subdirectory(include)
add_subdirectory(src)

set(HTTP_ENABLE_TESTS
    OFF
    CACHE
    BOOL
    "Enable the tests for ${PROJECT_NAME}"
)

if(HTTP_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(
    EXPORT
        httpTargets
    NAMESPACE 
        Http::
    DESTINATION 
        lib/cmake/http
)
