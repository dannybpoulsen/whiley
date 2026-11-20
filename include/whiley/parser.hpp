#include "whiley/ast.hpp"
#include "whiley/options.hpp"

#include <utility>

namespace Whiley {
  struct ParseResult {
    ParseResult (Program&& prgm, bool res) : prgm(std::move(prgm)),success(res) {}
    operator bool () const {return success;}
    auto&& get() {success = false; return std::move(prgm);}
  private:
    Program prgm;
    bool success;
  };
  
  
  class WParser {
  public:
    WParser () : flags(TypeFlags::All()) {    }
    ParseResult parse( const std::string& filename );
    ParseResult parse( std::istream& iss );

    
    template<Type t>
    void disable() {flags.reset (t);}
    
	
  private:
    TypeFlags flags;
    
  };
}

