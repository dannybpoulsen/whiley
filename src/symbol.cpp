#include "whiley/symbol.hpp"

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sstream>

namespace Whiley {

  struct Symbol::Internal {
    Internal (std::string name, std::shared_ptr<Internal> par) : name(name),parent(par) {}
    std::ostream& output (std::ostream& os) const {
      if (parent ) {
	return parent->output (os) << "#" << name;
	
      }
      else
	return os << name;
    }
    
    
    bool isRoot () const {
      return parent == nullptr;
    }
    
    std::string name;
    std::shared_ptr<Internal> parent;
  };

  Symbol::Symbol(Symbol parent,std::string name) : internal(std::make_shared<Internal>(name,parent.internal)){
  }
  
  Symbol::Symbol (std::string s) : internal(std::make_shared<Internal> (s,nullptr)) {}
  
  std::string Symbol::getName() const {return internal->name; }
  std::string Symbol::getFullName() const {
    std::stringstream str;
    internal->output(str);
    return str.str();
  }
  
  
  std::size_t Symbol::hash () const {
    return reinterpret_cast<std::size_t> (internal.get());
  }
  
  struct Frame::Internal : public std::enable_shared_from_this<Internal> {
    Internal (Symbol s, std::shared_ptr<Internal> inter) : symb(s),parent(inter) {}

    Symbol makeSymbol (std::string name) {
      if (!symbols.count(name)) {
	Symbol b (symb,name);
	symbols.emplace (std::make_pair(name,b));
	return b;
      }
      else
	throw std::runtime_error ("Symbol already exists");
    }
    
    Symbol symb;
    std::shared_ptr<Internal> parent;
    std::unordered_map<std::string,Symbol> symbols;
    std::unordered_map<std::string,std::shared_ptr<Internal>> frames;
    
  };

  Symbol Frame::createSymbol (std::string s) {
    return _internal->makeSymbol (s);
  }
  
  Frame Frame::create (std::string s) {
    if (!_internal->frames.count(s)) {
      auto frame_symb = _internal->makeSymbol (s);
      auto inter_frame = std::make_shared<Internal> (frame_symb,_internal->shared_from_this ());
      return Frame{inter_frame};
    }
    throw std::runtime_error ("Symbol exists");
  }

  Frame Frame::open (std::string s) {
    std::shared_ptr<Internal> _frame{nullptr};
      auto it  = _internal->frames.find (s);
      if (it== _internal->frames.end ())
	throw std::runtime_error ("Cannot find frame");

      _frame = it->second;
      
      return _frame;   
  }
  
  Symbol Frame::resolve(const std::string& s) const {
    auto it = _internal->symbols.find(s);
    if (it != _internal->symbols.end())
      return it->second;
    else
      throw std::runtime_error ("Cannot find symbol");
	   
  }

  Frame::Frame(std::string s) {
    _internal = std::make_shared<Internal> (Symbol (s),nullptr);
  }
  
  
}
