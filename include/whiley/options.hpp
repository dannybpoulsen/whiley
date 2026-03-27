
#ifndef _OPTIONS_WHILEY__
#define _OPTIONS_WHILEY__

#include "whiley/ast.hpp"

#include <bitset>


namespace Whiley {
  template<class E,E last>
  class Flags {
  public:
    static auto All () {
      Flags flags;
      flags.flags.set();
      return flags;
	
    }
    
    bool isSet (E e) const {
      return flags.test (std::to_underlying(e));
    }

    void set (E e)  {
      flags.set (std::to_underlying(e));
    }

    void reset (E e)  {
      flags.reset (std::to_underlying(e));
    }
    
    
  private:
    std::bitset<std::to_underlying(last)+1> flags;
  };
  
  using TypeFlags = Flags<Type,Type::Pointer>;
 
}

#endif
