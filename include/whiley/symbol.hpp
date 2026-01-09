#ifndef _WHILEY_SYMBOL__
#define _WHILEY_SYMBOL__


#include <memory>

namespace Whiley {
  
  
  class Symbol {
  private:
    struct Internal;
    std::shared_ptr<Internal> internal;
  public:
    std::string getName() const;
    std::string getFullName() const;
    
  public:
    Symbol (const Symbol& s) : internal(s.internal) {}
    Symbol (Symbol parent,std::string);
    Symbol (std::string s);
    std::size_t hash () const;
  };
  
  class Frame {
  public:
    Frame (std::string s);
    
    Frame open(std::string s);
    Frame create(std::string s);
    
    Symbol resolve(const std::string& s) const;
    Symbol createSymbol (std::string s);
  private:
    struct Internal;
    std::shared_ptr<Internal> _internal;
    
    Frame (std::shared_ptr<Internal>&& s) : _internal(std::move(s)) {}
    Frame (const std::shared_ptr<Internal>& s) : _internal(s) {}
    
   };
  
}


#endif
