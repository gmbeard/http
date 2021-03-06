﻿get_filename_component(
    PARSER_DIR 
    ${CMAKE_CURRENT_LIST_DIR} 
    PATH
)

set(PARSER_DIR
    ${PARSER_DIR}/submodules/http-parser
)

set(http-parser-sources
    ${PARSER_DIR}/http_parser.c
    ${PARSER_DIR}/http_parser.h
)

set_source_files_properties(
    ${http-parser-sources}
    PROPERTIES
        LANGUAGE C
)

add_library(
    httpParser
    STATIC
        ${http-parser-sources}
)

set_target_properties(
    httpParser
    PROPERTIES
        LINKER_LANGUAGE C
)

target_compile_options(
    httpParser
    PRIVATE
       # MSVC reports that http-parser has some signed/unsigned 
       # mismatch comparisons. From what I can tell, they look 
       # safe to ignore...
       $<$<C_COMPILER_ID:MSVC>:/wd4018 /wd4244 /wd4456>
)

target_include_directories(
    httpParser
    PUBLIC
        $<INSTALL_INTERFACE:include/http-parser>
        $<BUILD_INTERFACE:${PARSER_DIR}>
)

add_library(
    Http::httpParser 
    ALIAS 
    httpParser
)

#add_library(
#    http-parser-objects
#    OBJECT
#        ${http-parser-sources}
#)
#
#target_compile_options(
#    http-parser-objects
#    PRIVATE
#        # MSVC reports that http-parser has some signed/unsigned 
#        # mismatch comparisons. From what I can tell, they look 
#        # safe to ignore...
#        $<$<C_COMPILER_ID:MSVC>:/wd4018>
#)
#
#add_library(
#    http-objects
#    OBJECT
#        http.cpp
#)

#target_include_directories(
#    http-objects
#    PUBLIC
#        $<TARGET_PROPERTY:includes,INTERFACE_INCLUDE_DIRECTORIES>
#        $<TARGET_PROPERTY:Result::result,INTERFACE_INCLUDE_DIRECTORIES>
#        $<INSTALL_INTERFACE:include/http-parser>
#        $<BUILD_INTERFACE:${PARSER_DIR}>
#)

#target_compile_features(
#    http-objects
#    PUBLIC
#        cxx_decltype_auto
#)
#
#target_compile_options(
#    http-objects
#    PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror -Wextra>
#)

add_library(
    http 
    STATIC
#        ${http-parser-sources}
        http.cpp
        error.cpp
#        $<TARGET_OBJECTS:http-parser-objects>
#        $<TARGET_OBJECTS:http-objects>
)

target_compile_features(
    http
    PRIVATE
        cxx_decltype_auto
)

target_compile_options(
    http
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive- /W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror -Wextra>
)

#target_include_directories(
#    http
#    PUBLIC
#        $<TARGET_PROPERTY:includes,INTERFACE_INCLUDE_DIRECTORIES>
#        $<TARGET_PROPERTY:Result::result,INTERFACE_INCLUDE_DIRECTORIES>
##        $<INSTALL_INTERFACE:include/http-parser>
##        $<BUILD_INTERFACE:${PARSER_DIR}>
#)

target_link_libraries(
    http
    PUBLIC
        includes
        Result::result
        Http::httpParser
)

add_library(
    Http::http
    ALIAS
        http
)

install(
    FILES
        ${PARSER_DIR}/http_parser.h
    DESTINATION
        include/http-parser
)

install(
    TARGETS
        httpParser http 
    EXPORT
        httpTargets
    ARCHIVE DESTINATION 
        lib
    INCLUDES DESTINATION
        include
        include/http-parser
)
