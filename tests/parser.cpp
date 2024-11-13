#include "whiley/parser.hpp"
#include <iostream>

int main () {
  Whiley::WParser parser;
  auto prgm = parser.parse (std::cin);
  std::cerr << prgm << std::endl;
}

