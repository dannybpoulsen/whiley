#include "whiley/parser.hpp"
#include "scanner.h"
#include "parser.hh"
#include "whiley/ast.hpp"
#include "whiley/messaging.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

namespace Whiley {
  ParseResult  WParser::parse( std::istream& iss ) {
    ASTBuilder builder;
    Scanner scanner {&iss};
    
    Parser parser{scanner,builder, STDMessageSystem::get()};
    
    bool res = parser.parse ();
    return ParseResult{builder.get (),!res};
    
  }
  
  ParseResult WParser::parse(const std::string& s ) {
    std::ifstream ifs;
    
    ifs.open (s, std::ifstream::in);
    return parse (ifs);
  }
  
  
}
