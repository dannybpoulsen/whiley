#include "whiley/symbol.hpp"

#include <iostream>

int main () {
  Whiley::Frame  frame("Main");

  auto symb = frame.createSymbol ("ss");

  std::cout << symb.getFullName() << std::endl;
  
  
}
