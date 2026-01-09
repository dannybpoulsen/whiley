add_executable (whiley_tparse parser.cpp)
target_link_libraries (whiley_tparse PUBLIC whiley)


add_executable (whiley_symbol symbols.cpp)
target_link_libraries (whiley_symbol PUBLIC whiley)
