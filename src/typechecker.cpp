
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
    Whiley::Frame frame;
    Type type {Type::Untyped};
    bool ok;
    Function_ptr func;
    bool hasReturn{false};
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

   struct MissingReturn : public TypeCheckerMessage<Node>{
    MissingReturn (const Node& n) : TypeCheckerMessage(n) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << "missing return statement"; 
      return str.str();
    }
  };
  
  TypeChecker::~TypeChecker () {}

  bool TypeChecker::CheckProgram (Program& prgm) {
    _internal = std::make_unique<Internal> (prgm.getFrame());
    auto oldframe = _internal->frame;
    bool ok = true;
    for (auto s : oldframe.getLocalSymbols() ) {
      if (std::holds_alternative<Whiley::Function_ptr> (s.getUserData())) {
	auto func = std::get<Whiley::Function_ptr> (s.getUserData());
	_internal->func = func;
	_internal->frame = _internal->frame.open (s.getName());
	_internal->hasReturn = false;
	ok = ok && CheckStatement(*func->getStmt());
	_internal->frame = _internal->frame.close();
	_internal->func = nullptr;
	if (!_internal->hasReturn) {
	  messaging << MissingReturn {*func->getStmt()};
	  ok = false;
	}
      }
    }
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

   
  struct InconsistentNumberofParameters : public TypeCheckerMessage<Node>{
    InconsistentNumberofParameters (const Node& n) : TypeCheckerMessage(n) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << "Inconsistent number of parameters (formal vs actual)"; 
      return str.str();
    }
    

  };

  struct NotAFunction : public TypeCheckerMessage<Node>{
    NotAFunction (const Node& n,Whiley::Symbol name) : TypeCheckerMessage(n),name(name) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << name.getFullName() << " is not a function"; 
      return str.str();
    }

      Whiley::Symbol name;
  };

    struct NotAVariable : public TypeCheckerMessage<Node>{
    NotAVariable (const Node& n,Whiley::Symbol name) : TypeCheckerMessage(n),name(name) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << name.getFullName() << " is not a variable"; 
      return str.str();
    }

      Whiley::Symbol name;
  };
  
  struct ReturnStatementNotInFunction : public TypeCheckerMessage<Node>{
    ReturnStatementNotInFunction (const Node& n) : TypeCheckerMessage(n) {}
    
    std::string to_string () const override {
      std::stringstream str;
      str << loc_string ()<< ": '" << "Return statement not inside a function"; 
      return str.str();
    }
   
  };
  
  
  void TypeChecker::visitDerefExpression (const DerefExpression& expr)  {
    auto leftType =  CheckExpression (expr.getMem ());
    if (leftType != Type::Pointer) {
      messaging << TypeMismatch (leftType,Type::Pointer,expr);
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
    else if (leftType  == Type::Pointer  && expr.getOp () == BinOps::Add) {
      if (rightType != Type::UI64) {
	messaging << TypeMismatch (rightType,Type::UI64,expr);
	_internal->type = Type::Untyped;
      }
      else
	_internal->type = Type::Pointer;
      
    }
    else if (leftType == rightType ) {
      _internal->type = leftType;
    }
    else {
      messaging << TypeMismatch (leftType,rightType,expr);
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
    _internal->hasReturn = false;
    if (auto symb = _internal->frame.resolve(ass.getAssignName())) {
      auto symb_type = symbType(symb.value());
      _internal->ok = symb_type == val;
      if (!_internal->ok)
	messaging << TypeMismatch (symb_type,val,ass);
      
    }
    else {
      messaging << VariableNotDeclared (ass.getAssignName(),ass);
      _internal->ok = false;
    }
  }

  void TypeChecker::visitIncrementDecrementStatement (const IncrementDecrementStatement& ass)  {
    _internal->hasReturn = false;
    if (auto symb = _internal->frame.resolve(ass.getIncrementee())) {
      auto symb_type = symbType(symb.value());
      _internal->ok = isInteger (symb_type);
      if (!_internal->ok)
	messaging << TypeMismatch (symb_type,Type::UI8,ass);
    }
    else {
      messaging << VariableNotDeclared (ass.getIncrementee(),ass);
      _internal->ok = false;
    }
  }

  void TypeChecker::visitAllocStatement (const AllocStatement& ass)  {
    _internal->hasReturn = false;
    auto val = CheckExpression (ass.getExpression());
    if (auto symb = _internal->frame.resolve(ass.getAssignName())) {
      auto symb_type = symbType(symb.value());
      if (symb_type != Type::Pointer)
	{
	  messaging << TypeMismatch (symb_type,Type::Pointer,ass);
	  _internal->ok = false;
	}
      if (val != Type::UI64) {
	  messaging << TypeMismatch (val,Type::UI64,ass);
	  _internal->ok = false;
	
      }
      
    }
    else {
      messaging << VariableNotDeclared (ass.getAssignName(),ass);
      _internal->ok = false;
    }
  }

  void TypeChecker::visitFreeStatement (const FreeStatement& ass)  {
    _internal->hasReturn = false;
    auto val = CheckExpression (ass.getExpression());
    if (val != Type::Pointer) {
      messaging << TypeMismatch (val,Type::UI64,ass);
      _internal->ok = false;
      
      }
  }

  
  void TypeChecker::visitAssertStatement (const AssertStatement& ass)  {
    _internal->hasReturn = false;
    auto val = CheckExpression (ass.getExpression());
    _internal->ok = val!=Type::Untyped;
    if (!_internal->ok) {
      messaging << TypeMismatch (val,Type::SI8,ass);
    
    }
  }
  
  void TypeChecker::visitAssumeStatement (const AssumeStatement& ass)  {
    _internal->hasReturn = false;
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
    bool left_ok =  CheckStatement (iff.getIfBody ());
    bool left_ret = _internal->hasReturn;
    bool right_ok =  CheckStatement (iff.getElseBody ());
    bool right_ret = _internal->hasReturn;
    
    _internal->hasReturn = left_ret && right_ret;
    _internal->ok =  ok && left_ok && right_ok;
  }
  
  void TypeChecker::visitSkipStatement (const SkipStatement& )  {
    _internal->ok = true;
    _internal->hasReturn = false;
  } 
  void TypeChecker::visitWhileStatement (const WhileStatement& ww)  {
    auto val = CheckExpression (ww.getCondition());
    bool ok = val != Type::Untyped;
    if (!ok) {
      messaging << TypeMismatch (val,Type::SI8,ww.getCondition());
      
    }
    _internal->ok = ok && CheckStatement (ww.getBody ());
    _internal->hasReturn = false;
  }
  void TypeChecker::visitSequenceStatement (const SequenceStatement& seq)  {
    bool ok = CheckStatement (seq.getFirst ());
    bool leftRet = _internal->hasReturn;
    bool sec_ok =   CheckStatement (seq.getSecond ());
    _internal->hasReturn = _internal->hasReturn || leftRet;
    _internal->ok = ok && sec_ok;
    
  }

  void TypeChecker::visitChooseStatement (const ChooseStatement& whiles)  {
    bool ok = true;
    bool hasRet = false;
    for (auto& s :  whiles.getStatements()) {
      CheckStatement (*s);
      ok = _internal->ok && ok;
      hasRet = hasRet && _internal->hasReturn;
    }
  }
      
  
  void TypeChecker::visitMemAssignStatement (const MemAssignStatement& ma) {
    _internal->hasReturn = false;
    auto loc = CheckExpression(ma.getMemLoc ());
    auto expr = CheckExpression(ma.getExpression ());
    if (loc != Type::Pointer) {
      messaging << TypeMismatch (loc,Type::Pointer,ma.getMemLoc());
      _internal->ok = false; 
    }
    _internal->ok = _internal->ok && (expr!=Type::Untyped);
    
  }

  void TypeChecker::visitReturnStatement (const ReturnStatement& r) {
    _internal->hasReturn = true;
    if (_internal->func == nullptr) {
      messaging << ReturnStatementNotInFunction (r);
      _internal->ok = false;
    }
    auto ll = CheckExpression (r.getExpr());
    if (ll != _internal->func->returns()) {
      messaging << TypeMismatch (ll,_internal->func->returns(),r);
      _internal->ok = false;
    }
  }

  void TypeChecker::visitCallStatement (const CallStatement& r) {
    _internal->hasReturn = false;
    auto func_symb = _internal->frame.resolve(r.funcname());
    
    if (!func_symb)   {
      _internal->ok = false;
      return;
    }
  
    if (!std::holds_alternative<Whiley::Function_ptr> (func_symb.value().getUserData ())) {
      messaging << NotAFunction (r,func_symb.value());
      _internal->ok = false;
      return;
    }
    
    
    
  auto func_ptrs = std::get<Whiley::Function_ptr> (func_symb.value().getUserData ());
  if (r.assignname ()!="") {
    auto assign_name = _internal->frame.resolve(r.assignname());
    if (!assign_name)   {
      _internal->ok = false;
      return;
    }
  
    if (!std::holds_alternative<Whiley::VarDecl> (assign_name.value().getUserData ()) && !std::holds_alternative<Whiley::ParamDecl> (assign_name.value().getUserData ())) {
      _internal->ok = false;
      return;
    }
    auto assignType = std::visit (Whiley::overloaded {
	[](const Whiley::VarDecl& dec) {return dec.type;},
	  [](const Whiley::ParamDecl& dec) {return dec.type;},
	  [this,&r,&assign_name](auto& )->Whiley::Type { _internal->ok = false; messaging << NotAVariable (r,assign_name.value()); return Type::Untyped;
	  }
	  },
      assign_name.value().getUserData()
      );
    
    if (assignType != func_ptrs->returns()) {
      messaging << TypeMismatch (assignType,func_ptrs->returns(),r);
      _internal->ok = false;
    }
  }
  
  if (func_ptrs->getParams().size() != r.parameters().size()) {
    messaging << InconsistentNumberofParameters(r);
    
    _internal->ok = false;
    return;
  }
  

  auto it = func_ptrs->getParams().begin();
  auto pit = r.parameters().begin();
  for (; pit != r.parameters().end(); ++it,++pit) {
    auto actual_param = CheckExpression (**pit);
    auto formal_param = symbType (*it);
    if (actual_param != formal_param)  {
      _internal->ok = false;
      messaging << TypeMismatch (actual_param,formal_param,**pit);
      
    }
  }
  
  }
  
}

