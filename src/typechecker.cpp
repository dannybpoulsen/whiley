
#include "whiley/ast.hpp"
#include "whiley/typechecker.hpp"

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sstream>

namespace Whiley {
  struct TypeChecker::Internal {
    std::unordered_map<std::string,Declaration> declarations;
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
    _internal = std::make_unique<Internal> ();
    for (auto& t : prgm.getVars ()) {
      _internal->declarations.emplace (t.getName (),t);
    }
    return CheckStatement(prgm.getStmt());
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
  
  
  void TypeChecker::visitIdentifier (const Identifier& id)  {
    auto it = _internal->declarations.find (id.getName());
    if (it != _internal->declarations.end ()) {
      _internal->type = it->second.getType ();
    }

    else
      _internal->type = Type::Untyped;
  }
  
  void TypeChecker::visitNumberExpression (const NumberExpression& )  {
    _internal->type = Type::SI8;
  }

  void TypeChecker::visitUndefExpression (const UndefExpression& )  {
    _internal->type = Type::SI8;
  }
  
  
  struct TypeMismatch : public TypeCheckerMessage<Node>{
    TypeMismatch (Type t1,
		  Type t2,
		  const Node& n) : TypeCheckerMessage(n),t1(t1),t2(t2) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << "Type mismatch between" << t1 << " and " << t2; 
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
      _internal->type = Type::UI8;
  }

  
  
  void TypeChecker::visitCastExpression (const CastExpression& expr)  {
    auto v = CheckExpression (expr.getExpression ());
    if (bytesize(v) == bytesize(expr.getType()))
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
	
    else if (leftType == Type::SI8 ||
	rightType == Type::SI8) {
      _internal->type = Type::SI8;
    }
    else {
      _internal->type = Type::UI8;
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
    auto decl = _internal->declarations.find (ass.getAssignName());
    if (decl != _internal->declarations.end()) {
      _internal->ok = decl->second.getType () == val;
      if (!_internal->ok)
	messaging << TypeMismatch (decl->second.getType (),val,ass);
    }
    else {
      messaging << VariableNotDeclared{ass.getAssignName(),ass};
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

