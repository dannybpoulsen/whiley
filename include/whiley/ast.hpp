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

  namespace Whiley {
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
    class SkipStatement;
    class AssertStatement;
    class AssumeStatement;

    enum class Type {
      Untyped,
      UI8,
      SI8,
      UI16,
      SI16,
      UI32,
      SI32,
      UI64,
      SI64,
    };

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
      virtual void visitSequenceStatement (const SequenceStatement& ) = 0;
      
    };
    
    class NodeVisitor : public ExpressionVisitor,
			public StatementVisitor
    {
    public:
      virtual ~NodeVisitor () {}
      

    };

    class Declaration {
    public:
      Declaration (std::string name, Type ty) : name(std::move(name)),type(ty) {}
      Declaration (const Declaration&) = default;
      auto& getName () const {return name;}
      Type getType () const {return type;};
      
    private:
      std::string name;
      Type type;
      
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
      Identifier (std::string name, const location_t& loc = location_t{}) : Expression(loc), name(std::move(name)) {}
      auto getName () const {return name;}
      void accept (ExpressionVisitor& v) const {v.visitIdentifier (*this);} 
      bool isConstant () const override {return false;}
      
    private:
      std::string name;
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
      NEq
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
    
    /*class NonDetAssignStatement  : public Statement{
    public:
      NonDetAssignStatement (std::string assignName, Type type, const location_t& loc) : Statement(loc),
											 assignName(std::move(assignName)),
											 type(type)
      {}
      
      void accept (StatementVisitor& v) const override {v.visitNonDetAssignStatement(*this);}
      auto& getAssignName () const {return assignName;}
      auto getType () const {return type;}
     private:
      std::string assignName;
      Type type;
    };
    */
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
      Program (std::vector<Declaration>&& vars, Statement_ptr&& stmt) : declarations(std::move(vars)),
									      stmt(std::move(stmt)) {}

      auto& getStmt () const {return *stmt;}
      auto& getVars () const {return declarations;}
											     
	       
    private:
      std::vector<Declaration> declarations;
      Statement_ptr stmt;
    };

    inline std::ostream& operator<< (std::ostream& os, const Program& prgm) {
      std::for_each (prgm.getVars().begin(),prgm.getVars().end(),[&os](auto& s){
	os << s.getType() << " " << s.getName() << ";\n";
      });
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
	exprStack.insert (std::make_unique<Identifier> (name,l));
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

      void AssertStmt (const location_t& l) {

	auto expr = exprStack.pop ();
	  
	stmtStack.insert (std::make_unique<AssertStatement> (std::move(expr),l));
	
      }

      void AssumeStmt (const location_t& l) {

	auto expr = exprStack.pop ();
	  
	stmtStack.insert (std::make_unique<AssumeStatement> (std::move(expr),l));
	
      }
      
      void DeclareStmt (std::string name,  Type type,const location_t&) {
	declarations.emplace_back (name,type);
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
      
      auto get () {
	if (!stmtStack.size())
	  SkipStmt ({0,0,0,0});
	return Program (std::move(declarations),stmtStack.pop ());
	
      }
	
      
    private:
      Stack<Expression_ptr> exprStack;
      Stack<Statement_ptr> stmtStack;
      std::vector<Declaration> declarations;
      
      
    };
    
    
  }


#endif
