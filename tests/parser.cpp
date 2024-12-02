
#include "whiley/parser.hpp"
#include "whiley/typechecker.hpp"

#include <iostream>

int main () {
  Whiley::WParser parser;
  if (auto parseres = parser.parse (std::cin)) {
    auto prgm = parseres.get();
  
    if (Whiley::TypeChecker{}.CheckProgram (prgm))
      std::cerr << prgm << std::endl;
    else
      std::cerr << "Not Type correct" << std::endl;
  }
}

