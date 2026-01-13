
#include "whiley/ast.hpp"
#include "whiley/symbol.hpp"
#include "whiley/typechecker.hpp"

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sstream>
#include <variant>

namespace Whiley {
  struct TypeChecker::Internal {
    Internal(Whiley::Frame f) : frame(f) {} 
    //std::unordered_map<std::string,Declaration> declarations;
    Whiley::Frame frame;
    Type type {Type::Untyped};
    bool ok;
  };

  template<class N>
  struct TypeCheckerMessage : Message {
    TypeCheckerMessage (const N& n) : node(n) {}
    virtual std::string loc_string () const {
      std::stringstream str;
      str << "@" << node.getFileLocation ();
      return str.str();
    }
    
    
    
  private:
    const N& node;
  };
  
  
  TypeChecker::TypeChecker (MessageSystem& m) : messaging(m) {
    
  }
  
  TypeChecker::~TypeChecker () {}

  bool TypeChecker::CheckProgram (Program& prgm) {
    _internal = std::make_unique<Internal> (prgm.getFrame());
    auto oldframe = _internal->frame;
    bool ok = true;
    for (auto s : oldframe.getLocalSymbols() ) {
      if (std::holds_alternative<Whiley::Function_ptr> (s.getUserData())) {
	auto func = std::get<Whiley::Function_ptr> (s.getUserData());
	_internal->frame = _internal->frame.open (s.getName());
	ok = ok && CheckStatement(*func->getStmt());
	_internal->frame = _internal->frame.close();
      }
    }
    /*for (auto t : prgm.getVars ()) {
      _internal->declarations.emplace (t.getName (),t);
      }*/
    //_internal->frame = prgm.getFrame();
    return ok && CheckStatement(prgm.getStmt());
  }

  
  
  Type TypeChecker::CheckExpression (Expression& e) {
    e.accept(*this);
    e.setType (_internal->type);
    return e.getType ();
  }

  bool TypeChecker::CheckStatement (Statement& e) {
    _internal->ok = true;
    e.accept(*this);
    return _internal->ok;
  }
  
  auto symbType(const Symbol& s) {
    return std::visit(Whiley::overloaded {
	[](const VarDecl& vd){return vd.type;},
	  [](const ParamDecl& vd){return vd.type;},
	  [](auto&) {return Whiley::Type::Untyped;}
	  },
      s.getUserData()
      );
  }
  
  void TypeChecker::visitIdentifier (const Identifier& id)  {
    /*auto it = _internal->declarations.find (id.getName());
    if (it != _internal->declarations.end ()) {
      _internal->type = it->second.getType ();
    }

    else
    _internal->type = Type::Untyped;*/
    auto symb = id.getSymbol();
    _internal->type = symbType(symb);
  }
  
  void TypeChecker::visitNumberExpression (const NumberExpression&)  {
    _internal->type = Type::SI64;
  }

  void TypeChecker::visitUndefExpression (const UndefExpression& e)  {
    _internal->type = e.getUndefType();
  }
  
  
  struct TypeMismatch : public TypeCheckerMessage<Node>{
    TypeMismatch (Type t1,
		  Type t2,
		  const Node& n) : TypeCheckerMessage(n),t1(t1),t2(t2) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << "Type mismatch between " << t1 << " and " << t2; 
      return str.str();
    }
    
  private:
    Type t1;
    Type t2;
  };
  
  
  void TypeChecker::visitDerefExpression (const DerefExpression& expr)  {
    auto leftType =  CheckExpression (expr.getMem ());
    if (leftType != Type::UI8) {
      messaging << TypeMismatch (leftType,Type::UI8,expr);
      _internal->type = Type::Untyped;
    }
    else
      _internal->type = expr.getLoadType();
  }

  
  
  void TypeChecker::visitCastExpression (const CastExpression& expr)  {
    auto v = CheckExpression (expr.getExpression ());
    if (v != Type::Untyped)
      _internal->type = expr.getType();
    else
      _internal->type = Type::Untyped;
  }
  
  
  void TypeChecker::visitBinaryExpression (const BinaryExpression& expr)  {
    auto leftType =  CheckExpression (expr.getLeft ());
    auto rightType = CheckExpression (expr.getRight ());
    
    if (leftType == Type::Untyped ||
	rightType == Type::Untyped)
      _internal->type = Type::Untyped;
	
    else if (leftType == rightType ) {
      _internal->type = leftType;
    }
    else {
      _internal->type = Type::Untyped;
    }
    
  }

  struct VariableNotDeclared : public TypeCheckerMessage<Node>{
    VariableNotDeclared (std::string name,
			 const Node& n) : TypeCheckerMessage(n),name(name) {}

    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << name <<"' not declared"; 
      return str.str();
    }
    
  private:
    std::string name; 
  };

  
  void TypeChecker::visitAssignStatement (const AssignStatement& ass)  {
    auto val = CheckExpression (ass.getExpression());
    Whiley::Symbol symb{"h"};
    if (_internal->frame.resolve(ass.getAssignName(),symb)) {
      auto symb_type = symbType(symb);
      _internal->ok = symb_type == val;
      if (!_internal->ok)
	messaging << TypeMismatch (symb_type,val,ass);
      
    }
    else {
      messaging << VariableNotDeclared (ass.getAssignName(),ass);
      _internal->ok = false;
    }
  }
  void TypeChecker::visitAssertStatement (const AssertStatement& ass)  {
    auto val = CheckExpression (ass.getExpression());
    _internal->ok = val!=Type::Untyped;
    if (!_internal->ok) {
      messaging << TypeMismatch (val,Type::SI8,ass);
    
    }
  }
  
  void TypeChecker::visitAssumeStatement (const AssumeStatement& ass)  {
    auto val = CheckExpression (ass.getExpression());
    _internal->ok = val!=Type::Untyped;
    if (!_internal->ok) {
      messaging << TypeMismatch (val,Type::SI8,ass);
    }
  }
  
  
  void TypeChecker::visitIfStatement (const IfStatement& iff)  {
    auto val = CheckExpression (iff.getCondition());
    bool ok = val != Type::Untyped;
    if (!ok) {
      messaging << TypeMismatch (val,Type::SI8,iff.getCondition());
    
    }
    _internal->ok =  ok && CheckStatement (iff.getIfBody ()) && CheckStatement (iff.getElseBody ());
  }
  
  void TypeChecker::visitSkipStatement (const SkipStatement& )  {} 
  void TypeChecker::visitWhileStatement (const WhileStatement& ww)  {
    auto val = CheckExpression (ww.getCondition());
    bool ok = val != Type::Untyped;
    if (!ok) {
      messaging << TypeMismatch (val,Type::SI8,ww.getCondition());
      
    }
    _internal->ok = ok && CheckStatement (ww.getBody ());
    
  }
  void TypeChecker::visitSequenceStatement (const SequenceStatement& seq)  {
    bool ok = CheckStatement (seq.getFirst ());
    _internal->ok = ok && CheckStatement (seq.getSecond ());
    
  }

  void TypeChecker::visitChooseStatement (const ChooseStatement& whiles)  {
    for (auto& s :  whiles.getStatements()) {
      CheckStatement (*s);
    }
  }
      
  
  void TypeChecker::visitMemAssignStatement (const MemAssignStatement& ma) {
    auto loc = CheckExpression(ma.getMemLoc ());
    auto expr = CheckExpression(ma.getExpression ());
    if (loc != Type::UI8) {
      messaging << TypeMismatch (loc,Type::UI8,ma.getMemLoc());
      _internal->ok = false; 
    }
    _internal->ok = _internal->ok && (expr!=Type::Untyped);
    
  }
  
}

