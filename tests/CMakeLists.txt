find_package(Catch2 REQUIRED)

add_executable(
    tests
    main.cpp
    http_tests.cpp
)

target_compile_features(
    tests
    PRIVATE
        cxx_decltype_auto
)

target_compile_options(
    tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror -Wextra>
)

target_link_libraries(
    tests
    PRIVATE
        http
        Catch2::Catch
)
