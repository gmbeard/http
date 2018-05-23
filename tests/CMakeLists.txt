find_package(Catch2 REQUIRED)

add_executable(
    tests
    main.cpp
    http_tests.cpp
)

target_link_libraries(
    tests
    PRIVATE
        http
        Catch2::Catch
)
