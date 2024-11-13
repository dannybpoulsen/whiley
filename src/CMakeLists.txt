find_package (BISON REQUIRED)
find_package (FLEX REQUIRED)

flex_target(flexer lexer.l "${CMAKE_CURRENT_BINARY_DIR}/lexer.cc")
bison_target(bparser parser.y "${CMAKE_CURRENT_BINARY_DIR}/parser.cc")

configure_file (
 "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
 "${CMAKE_CURRENT_BINARY_DIR}/whiley/config.hpp"
)


add_library (whiley STATIC ast.cpp wparser.cpp "${CMAKE_CURRENT_BINARY_DIR}/lexer.cc" "${CMAKE_CURRENT_BINARY_DIR}/parser.cc")
target_include_directories (whiley PUBLIC ${PROJECT_SOURCE_DIR}/include PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")


target_sources(whiley
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/whiley/ast.hpp
    FILES ${PROJECT_SOURCE_DIR}/include/whiley/parser.hpp
)

target_sources(whiley
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}	
    FILES ${CMAKE_CURRENT_BINARY_DIR}/whiley/config.hpp
)

add_library (Whiley::whiley ALIAS whiley)
