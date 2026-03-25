#ifndef _WHILEY_AST__
#define _WHILEY_AST__


#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <utility>
#include <generator>
#include <algorithm>

#include "whiley/symbol.hpp"

  namespace Whiley {
    template<class... Ts> struct overloaded : Ts... { 
      using Ts::operator()...; 
    };
    
    class Identifier;
    class NumberExpression;
    class NumberExpression;
    class UndefExpression;
    class BinaryExpression;
    class DerefExpression;
    class CastExpression;
    
    class AssignStatement;
    class NonDetAssignStatement;
    class MemAssignStatement;
    class DeclareStatement;
    class IfStatement;
    class WhileStatement;
    class SequenceStatement;
    class ChooseStatement;
    class SkipStatement;
    class AssertStatement;
    class AssumeStatement;
    class ReturnStatement;
    class CallStatement;
    class AllocStatement;
    class FreeStatement;
    class IncrementDecrementStatement;

    inline std::size_t bytesize(Type t) {
      switch (t) {
      case Type::SI8:
      case Type::UI8:
	return 1;
      case Type::SI16:
      case Type::UI16:
	return 2;
      case Type::SI32:
      case Type::UI32:
	return 4;
      case Type::SI64:
      case Type::UI64:
	return 8;
      case Type::Untyped:
      default:
	return 0;
      };
    }

    inline bool isInteger (Type t) {
      switch(t) {
      case Type::Untyped:
      case Type::Pointer:
	return false;
      default:
	return true;
      }
    }
    
    inline std::ostream& operator<< (std::ostream& os, Type t) {
      switch (t) {
      case Type::Untyped:
	return os << "Untyped";
      case Type::UI8:
	return os << "ui8";
      case Type::SI8:
	return os << "si8";
      case Type::UI16:
	return os << "ui16";
      case Type::SI16:
	return os << "si16";
      case Type::UI32:
	return os << "ui32";
      case Type::SI32:
	return os << "si32";
      case Type::UI64:
	return os << "ui64";
      case Type::SI64:
	return os << "si64";
      case Type::Pointer:
	return os << "ptr";
      default:
	std::unreachable();
      }
    }
    
    class ExpressionVisitor {
    public:
      virtual ~ExpressionVisitor () {} 
      virtual void visitIdentifier (const Identifier&) = 0;
      virtual void visitNumberExpression (const NumberExpression& ) = 0;
      virtual void visitBinaryExpression (const BinaryExpression& ) = 0;
      virtual void visitDerefExpression (const DerefExpression& ) = 0;
      virtual void visitCastExpression (const CastExpression& ) = 0;
      virtual void visitUndefExpression (const UndefExpression& ) = 0;
      
    };

    class StatementVisitor {
    public:
      virtual ~StatementVisitor () {}
      virtual void visitAssertStatement (const AssertStatement& ) = 0;
      virtual void visitAssignStatement (const AssignStatement& ) = 0;
      virtual void visitAssumeStatement (const AssumeStatement& ) = 0;
      //virtual void visitNonDetAssignStatement (const NonDetAssignStatement& ) = 0;
      virtual void visitMemAssignStatement (const MemAssignStatement& ) = 0;
      
      virtual void visitIfStatement (const IfStatement& ) = 0;
      virtual void visitSkipStatement (const SkipStatement& ) = 0;
      
      virtual void visitWhileStatement (const WhileStatement& ) = 0;
      virtual void visitChooseStatement (const ChooseStatement& ) = 0;
      
      virtual void visitSequenceStatement (const SequenceStatement& ) = 0;
      virtual void visitReturnStatement (const ReturnStatement& ) = 0;
      virtual void visitCallStatement (const CallStatement& ) = 0;
      virtual void visitAllocStatement (const AllocStatement& ) = 0;
      virtual void visitFreeStatement (const FreeStatement& ) = 0;
      virtual void visitIncrementDecrementStatement (const IncrementDecrementStatement& ) = 0;
    };
    
    class NodeVisitor : public ExpressionVisitor,
			public StatementVisitor
    {
    public:
      virtual ~NodeVisitor () {}
      

    };

    class Declaration {
    public:
      Declaration (Whiley::Symbol symb, Type ty,bool parameter = false,bool out = false) : name(std::move(symb)),type(ty),parameter(parameter),output(out) {}
      Declaration (const Declaration&) = default;
      auto getName () const {return name.getName();}
      Whiley::Symbol getSymbol () const {return name;}
      
      Type getType () const {return type;};
      bool isParamter () const {return parameter;}
      bool isOutput () const {return output;}
    private:
      Whiley::Symbol name;
      Type type;
      bool parameter;
      bool output;
    };

    class FuncDeclaration {
    public:
      FuncDeclaration (Whiley::Symbol symb,Whiley::Function_ptr f) : symb(symb),func(f) {}
      auto getFunction () {return func;}
      auto getSymbol () {return symb;}
      
    private:
      Whiley::Symbol symb;
      Whiley::Function_ptr func;
    };
    
    struct fileloc_t {
      std::size_t line{1};
      std::size_t col{1};
      operator std::string () const {
	std::stringstream str;
	str << line << ":" << col;
	return str.str();
      }
    };

    inline std::ostream& operator<< (std::ostream& os, const fileloc_t& loc) {
      return os << loc.line <<":"<<loc.col;
    }
    
    struct location_t {
      fileloc_t begin;
      fileloc_t end;

      void step () {begin = end; end = fileloc_t{};}
      void movecol (std::size_t s) {end.col+=s;}
      void newline () {
	end.line++;
	end.col = 1;
	begin = end;
      }
    };

    inline std::ostream& operator<< (std::ostream& os, const location_t& loc) {
      return os << loc.begin << " - " << loc.end;
    }
    

    
    
    class Node {
    public:
      Node (const location_t& loc = location_t{}) : location (loc) {}
      virtual ~Node () {}
      auto& getFileLocation () const {return location.begin;}
    private:
      location_t location;
    };

    
    using Node_ptr = std::unique_ptr<Node>;

    using expr_t = std::size_t;

    
    
    class Expression : public Node {
    public:
      Expression (const location_t& loc) : Node(loc) {}
      virtual ~Expression () {}
      virtual bool isConstant () const = 0;
      Type getType () const {return type;}
      void setType (Type t) {type =t;}
      virtual void accept (ExpressionVisitor&) const = 0;
      
    private:
      Type type {Type::Untyped};
    };
    
    using Expression_ptr = std::unique_ptr<Expression>;
    
    
    class Identifier : public Expression {
    public:
      Identifier (Whiley::Symbol name, const location_t& loc = location_t{}) : Expression(loc), symb(std::move(name)) {}
      auto getName () const {return symb.getName();}
      auto getSymbol () const {return symb;}
      
      void accept (ExpressionVisitor& v) const {v.visitIdentifier (*this);} 
      bool isConstant () const override {return false;}
      
    private:
      Whiley::Symbol symb;
    };

    class NumberExpression : public Expression {
    public:
      NumberExpression (std::int64_t value, const location_t& loc) : Expression(loc),value(value) {}
      auto getValue () const {return value;}
      void accept (ExpressionVisitor& v) const {v.visitNumberExpression (*this);}
      bool isConstant () const override {return true;}
    private:
      std::int64_t value;
    };
    
    enum class BinOps {
      Add,
      Sub,
      Div,
      Mul,
      LEq,
      GEq,
      Lt,
      Gt,
      Eq,
      NEq,
      Mod
    };

    class BinaryExpression : public Expression {
    public:
      BinaryExpression (Expression_ptr&& l, Expression_ptr&& r, BinOps op, const location_t& loc) : Expression(loc),
												    left(std::move(l)),
												    right(std::move(r)),
												    type(op) {}
      auto getOp () const {return type;}
      auto& getLeft () const {return *left;}
      auto& getRight () const {return *right;}
      bool isConstant () const override {return false;}
      void accept (ExpressionVisitor& v) const {v.visitBinaryExpression (*this);}
      
    private:
      Expression_ptr left;
      Expression_ptr right;
      BinOps type;
    };

    class DerefExpression : public Expression {
    public:
      DerefExpression (Expression_ptr&& l, Type load, const location_t& loc) : Expression(loc),
									       left(std::move(l)),
									       loadtype(load)
									       
      {}
								   
      auto& getMem () const {return *left;}
      bool isConstant () const override {return false;}
      void accept (ExpressionVisitor& v) const {v.visitDerefExpression (*this);}
      auto getLoadType() const {return loadtype;}
    private:
      Expression_ptr left;
      Type loadtype;
    };

    class UndefExpression : public Expression {
    public:
      UndefExpression (Whiley::Type type, const location_t& loc) : Expression(loc),type(type)	 {}
								   
      bool isConstant () const override {return false;}
      void accept (ExpressionVisitor& v) const {v.visitUndefExpression (*this);}
      auto getUndefType() const {return type;}
    private:
      Whiley::Type type;
    };
    
    class CastExpression : public Expression {
    public:
      CastExpression (Expression_ptr&& l, Type t,const location_t& loc) : Expression(loc),
									  left(std::move(l)),
									  type(t)
      {}

      auto& getExpression () const {return *left;}
      auto getType () const {return type;}
      bool isConstant () const override {return false;}
      void accept (ExpressionVisitor& v) const {v.visitCastExpression (*this);}
      
    private:
      Expression_ptr left;
      Type type;
    };
    
    
    class Statement : public Node {
    public:
      Statement (const location_t& loc) : Node(loc) {}
      virtual ~Statement () {}
      virtual void accept (StatementVisitor&) const = 0;
      
    };
    
    using Statement_ptr = std::unique_ptr<Statement>;

    class SkipStatement  : public Statement{
    public:
      SkipStatement (const location_t& loc) : Statement(loc) {}
      void accept (StatementVisitor& v) const {v.visitSkipStatement (*this);}
    };
    
    class AssignStatement  : public Statement{
    public:
      AssignStatement (std::string assignName, Expression_ptr&& expr, const location_t& loc) : Statement(loc),
											       assignName(std::move(assignName)),
											       expr(std::move(expr)) {}
      
      void accept (StatementVisitor& v) const override {v.visitAssignStatement(*this);}
      auto& getAssignName () const {return assignName;}
      auto& getExpression () const {return *expr;}
      
      private:
      std::string assignName;
      Expression_ptr expr;

    };

    class IncrementDecrementStatement  : public Statement{
    public:
      IncrementDecrementStatement (std::string assignName, bool decrement,const location_t& loc) : Statement(loc),
												   assignName(std::move(assignName)),
												   decrement(decrement)
										     {}
      
      void accept (StatementVisitor& v) const override {v.visitIncrementDecrementStatement(*this);}
      auto& getIncrementee () const {return assignName;}
      
      private:
      std::string assignName;
      bool decrement{false};
    };

    class AllocStatement  : public Statement{
    public:
      AllocStatement (std::string assignName, Expression_ptr&& expr, const location_t& loc) : Statement(loc),
											       assignName(std::move(assignName)),
											       expr(std::move(expr)) {}
      
      void accept (StatementVisitor& v) const override {v.visitAllocStatement(*this);}
      auto& getAssignName () const {return assignName;}
      auto& getExpression () const {return *expr;}
      
      private:
      std::string assignName;
      Expression_ptr expr;
    };

    class FreeStatement  : public Statement{
    public:
      FreeStatement (Expression_ptr&& expr, const location_t& loc) : Statement(loc),									       
											       expr(std::move(expr)) {}
      
      void accept (StatementVisitor& v) const override {v.visitFreeStatement(*this);}
      auto& getExpression () const {return *expr;}
      
      private:
      Expression_ptr expr;
    };
    
    class AssertStatement : public Statement {
    public:
      AssertStatement (Expression_ptr&& expr, const location_t& loc) : Statement(loc),expr(std::move(expr)) {}
      
      void accept (StatementVisitor& v) const override {v.visitAssertStatement(*this);}
      auto& getExpression () const {return *expr;}
      
    private:
      Expression_ptr expr;
    };

    class AssumeStatement : public Statement {
    public:
      AssumeStatement (Expression_ptr&& expr, const location_t& loc) : Statement(loc),expr(std::move(expr)) {}
      
      void accept (StatementVisitor& v) const override {v.visitAssumeStatement(*this);}
      auto& getExpression () const {return *expr;}
      
    private:
      Expression_ptr expr;
    };
    
    class MemAssignStatement  : public Statement{
    public:
      MemAssignStatement (Expression_ptr&& mem, Expression_ptr&& expr, const location_t& loc) : Statement(loc),
												memLoc(std::move(mem)),
												expr(std::move(expr)) {}
      
      void accept (StatementVisitor& v) const override {v.visitMemAssignStatement(*this);}
      auto& getMemLoc () const {return *memLoc;}
      auto& getExpression () const {return *expr;}
      
      private:
      Expression_ptr memLoc;
      Expression_ptr expr;
    };
    
    class IfStatement  : public Statement{
    public:
      IfStatement (Expression_ptr&& cond,Statement_ptr ifb, Statement_ptr elseb, const location_t& loc) : Statement(loc),
													  cond(std::move(cond)) ,
													  if_body(std::move(ifb)),
													  else_body(std::move(elseb)) {}
      
      void accept (StatementVisitor& v) const override {v.visitIfStatement(*this);}
      auto& getCondition () const {return *cond;}
      auto& getIfBody () const {return *if_body;}
      auto& getElseBody () const {return *else_body;}
      
    private:
      Expression_ptr cond;
      Statement_ptr if_body;
      Statement_ptr else_body;
      
    };

    class ChooseStatement  : public Statement {
    public:
      ChooseStatement (std::vector<Statement_ptr>&& stmts,const location_t& loc) : Statement(loc),
										   statements(std::move(stmts)) {}
      
      
      void accept (StatementVisitor& v) const override {v.visitChooseStatement(*this);}
      const auto& getStatements() const {return statements;}
    private:
      std::vector<Statement_ptr> statements;
    };
    
    class WhileStatement  : public Statement{
    public:
      WhileStatement (Expression_ptr&& cond,Statement_ptr body, const location_t& loc) : Statement(loc),
											 cond(std::move(cond)) ,
											 body(std::move(body)) {}
      
      void accept (StatementVisitor& v) const override {
	v.visitWhileStatement (*this);
      }

      auto& getCondition () const {return *cond;}
      auto& getBody () const {return *body;}
      
    private:
      Expression_ptr cond;
      Statement_ptr body;
      
    };

    class SequenceStatement  : public Statement{
    public:
      SequenceStatement (Statement_ptr first,Statement_ptr second, const location_t& loc) : Statement(loc),
											    first(std::move(first)) ,
											    second(std::move(second)) {}
      
      void accept (StatementVisitor& v) const {
	v.visitSequenceStatement (*this);
      }
      
      auto& getFirst () const {return *first;}
      auto& getSecond () const {return *second;}
      
    private:

      Statement_ptr first;
      Statement_ptr second;
      
    };

    class ReturnStatement : public Statement {
    public:
      ReturnStatement (Expression_ptr expr,const location_t& loc) : Statement(loc),
								    expr(std::move(expr)) {}
      void accept (StatementVisitor& v) const {
	v.visitReturnStatement (*this);
      }

      auto& getExpr() const {return *expr;}
      
    private:
      Expression_ptr expr;
    };

    class CallStatement : public Statement {
    public:
      CallStatement (std::string assignname, std::string funcname, std::vector<Expression_ptr> params ,const location_t& loc) : Statement(loc),

																assign_name(assignname),
																func_name(funcname),
      
      params(std::move(params)) {}
      
      void accept (StatementVisitor& v) const {
	v.visitCallStatement (*this);
      }


      auto& assignname ()const {return assign_name;}
      auto& funcname ()const {return func_name;}
      auto& parameters () const {return params;}
    private:
      std::string assign_name;
      std::string func_name;
      
      std::vector<Expression_ptr> params;
    };


    std::ostream& operator<< (std::ostream&, const Statement& );
    
    
    template<class T>
    class Stack {
    public:
      auto size () const {return stack.size();}
      void insert (T&& t) { stack.push_back(std::move(t));}
      auto pop () {
	if (stack.size ()) {
	  auto r = std::move(stack.back ());
	  stack.pop_back ();
	  return r;
	}
	throw std::runtime_error ("Missing element on stack");	
      
      }
    private:
      std::vector<T> stack;
    };

    
    
    class Program {
    public:
      Program (Whiley::Frame&& frame,  Statement_ptr&& stmt) : 	stmt(std::move(stmt)),									       frame(std::move(frame))
      {}
      
      auto& getStmt () const {return *stmt;}
      
      std::generator<Declaration> getVars () const {
	for (auto symb : frame.getLocalSymbols()) {
	  auto& data = symb.getUserData();
	  if (std::holds_alternative<VarDecl> (data)) {
	    auto v = std::get<VarDecl> (data);
	    co_yield Declaration {symb,v.type,v.parameter,v.output};
	  }
	}
	co_return;
      }

      std::generator<FuncDeclaration> getFunctions () const {
	for (auto symb : frame.getLocalSymbols()) {
	  auto& data = symb.getUserData();
	  if (std::holds_alternative<Function_ptr> (data)) {
	    auto v = std::get<Function_ptr> (data);
	    co_yield FuncDeclaration {symb,v};
	  }
	}
	co_return;
      }

      auto getFrame () const {return frame;}
      
    private:
      Statement_ptr stmt;
      Whiley::Frame frame;
      
    };

    std::ostream& operator<< (std::ostream& os, const Whiley::Frame&);
    
    inline std::ostream& operator<< (std::ostream& os, const Program& prgm) {
      os << prgm.getFrame() << "\n";
      return os << prgm.getStmt ();
    }
    
    class ASTBuilder {
    public:
      
      
      void NumberExpr (std::int64_t val, const location_t& l) {
	exprStack.insert (std::make_unique<NumberExpression> (val,l));
      }

      
      void UndefExpr (Whiley::Type type, const location_t& l) {
	exprStack.insert (std::make_unique<UndefExpression> (type,l));
      }
      
      
      void IdentifierExpr (const std::string name, const location_t& l) {
        Symbol symb{"HH"};
	if (frame.resolve(name,symb)) {
          exprStack.insert(std::make_unique<Identifier>(symb, l));
        } else {
	  NumberExpr (0,l);
	}          
      }

      void DerefExpr (Type t, const location_t& l) {
	auto left = exprStack.pop ();  
	exprStack.insert (std::make_unique<DerefExpression> (std::move(left),t,l));
      }

      void CastExpr (Type type, const location_t& l) {
	auto left = exprStack.pop ();  
	exprStack.insert (std::make_unique<CastExpression> (std::move(left),type,l));
      }
      
      
      void BinaryExpr (BinOps op, const location_t& l) {
	auto right = exprStack.pop ();
	auto left = exprStack.pop ();  
	exprStack.insert (std::make_unique<BinaryExpression> (std::move(left),std::move(right),op,l));
      }

      void AssignStmt (std::string name, const location_t& l) {
	
	auto expr = exprStack.pop ();
	
	stmtStack.insert (std::make_unique<AssignStatement> (name,std::move(expr),l));
      }

      void AllocStmt (std::string name, const location_t& l) {
	
	auto expr = exprStack.pop ();
	
	stmtStack.insert (std::make_unique<AllocStatement> (name,std::move(expr),l));
      }

      void FreeStmt (const location_t& l) {
	
	auto expr = exprStack.pop ();
	
	stmtStack.insert (std::make_unique<FreeStatement> (std::move(expr),l));
      }
      
      void AssertStmt (const location_t& l) {

	auto expr = exprStack.pop ();
	  
	stmtStack.insert (std::make_unique<AssertStatement> (std::move(expr),l));
	
      }

      void AssumeStmt (const location_t& l) {

	auto expr = exprStack.pop ();
	  
	stmtStack.insert (std::make_unique<AssumeStatement> (std::move(expr),l));
	
      }
      
      void DeclareStmt (std::string name,  Type type,bool parameter,bool out, const location_t&) {
	auto symb = frame.createSymbol (name);
	symb.setUserData (VarDecl {type,parameter,out});
      }

      void ParamDeclare (std::string name,  Type type,const location_t&) {
	auto symb = frame.createSymbol (name);
	symb.setUserData (ParamDecl {type});
	params.push_back(symb);
      }
      
      void IfStmt (const location_t& l)  {
	auto expr = exprStack.pop ();
	auto elseb = stmtStack.pop ();
	auto ifb = stmtStack.pop ();
	stmtStack.insert (std::make_unique<IfStatement> (std::move(expr),
							 std::move(ifb),
							 std::move(elseb),
							 l)
			  );
      }

      void ChooseStmt (std::size_t bufs, const location_t& l)  {
	std::vector<Statement_ptr> statements;
	for (std::size_t i = 0; i< bufs; ++i) {
	  statements.push_back (std::move(stmtStack.pop ()));
	}
	stmtStack.insert (std::make_unique<ChooseStatement> (std::move(statements),l));
      }
      
       void SkipStmt (const location_t& l) {
	
	 stmtStack.insert (std::make_unique<SkipStatement> (l));
      }

      void MemAssignStmt (const location_t& l) {
	auto assign_val = exprStack.pop ();
	auto mem = exprStack.pop ();
	
	stmtStack.insert (std::make_unique<MemAssignStatement> (std::move(mem),
								std::move(assign_val),
								l)
			  );
      }
      
      
      void WhileStmt (const location_t& l) {
	auto expr = exprStack.pop ();
	auto body = stmtStack.pop ();
	
	stmtStack.insert (std::make_unique<WhileStatement> (std::move(expr),
							    std::move(body),
							    l
							    )
			  
			  );
	
        }

      void SequenceStmt ( const location_t& l) {
	auto second = stmtStack.pop ();
	auto first = stmtStack.pop ();
	
	stmtStack.insert (std::make_unique<SequenceStatement> (std::move(first),
							       std::move(second),
							       l)
			  );
	
      }

      void Increment (const std::string name, location_t& l) {
	stmtStack.insert (std::make_unique<IncrementDecrementStatement> (name,false,l));
      }

      void FunctionBegin (const std::string name) {
	funcname = name;
	frame = frame.create (name);
	params.clear();
      }
      
      void FunctionEnd (Type ty) {
	auto oldframe = frame;
	frame = frame.close();
	auto symbol = frame.createSymbol (funcname);
	
	symbol.setUserData (std::make_shared<Whiley::Function> (oldframe,stmtStack.pop(),std::move(params),ty));
      }

      void ReturnStmt (const location_t& l) {
	auto expr = exprStack.pop ();
	
	stmtStack.insert(std::make_unique<ReturnStatement> (std::move(expr),l));
	
      }
      
      void CallStmt (std::string ass, std::string funcname, std::size_t nbExprs, const location_t& loc) {
	std::vector<Expression_ptr> exprs;
	for (std::size_t i = 0; i< nbExprs; ++i) {
	  exprs.push_back (std::move(exprStack.pop ()));
	}
	std::reverse(exprs.begin(),exprs.end());
	stmtStack.insert(std::make_unique<CallStatement> (ass,funcname,std::move(exprs),loc));
      }

      auto get () {
	if (!stmtStack.size())
	  SkipStmt ({0,0,0,0});
	return Program (std::move(frame),stmtStack.pop ());
	
      }

      
      
    private:
      Stack<Expression_ptr> exprStack;
      Stack<Statement_ptr> stmtStack;
      
      Whiley::Frame frame{""};
      std::string funcname;
      std::vector<Symbol> params;
    };
    
    
  }


#endif
