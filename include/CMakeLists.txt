add_library(includes INTERFACE)
target_include_directories(
    includes
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

install(
    DIRECTORY
        http
    DESTINATION
        include
)

add_library(Http:includes ALIAS includes)

install(
    TARGETS includes
    EXPORT httpTargets
)
