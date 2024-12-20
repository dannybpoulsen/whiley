#include "whiley/compiler.hpp"
#include <unordered_map>

namespace FMTeach {
  namespace Whiley {
    struct Compiler::Internal {
      FMTeach::IR::CFA cfa;
      FMTeach::IR::Location_ptr start;
      FMTeach::IR::Location_ptr end;
      FMTeach::IR::Expr_ptr expr;
      std::unordered_map<std::string,FMTeach::IR::Register_ptr> vars;
    };

    FMTeach::IR::CFA Compiler::Compile (const FMTeach::Whiley::Program& prgm) {
      Internal mystore;
      _internal = &mystore;

      for (auto& var : prgm.getVars ()) {
	mystore.vars.emplace (var,mystore.cfa.makeRegister (var));
      }

      mystore.start = mystore.cfa.makeLocation ("Init",true);

      prgm.getStmt ().accept(*this);
      
      return mystore.cfa;
    }
      
    
    void Compiler::visitIdentifier (const Identifier& ids) {
      _internal->expr = _internal->vars.at(ids.getName ());
    }
    
    void Compiler::visitNumberExpression (const NumberExpression& num) {
      _internal->expr = std::make_shared<FMTeach::IR::Constant> (num.getValue ());
    }

    void Compiler::visitDerefExpression (const DerefExpression& num) {
      num.getMem ().accept (*this);
      _internal->expr = std::make_shared<FMTeach::IR::DerefExpr> (_internal->expr);
    }
    
    
    void Compiler::visitBinaryExpression (const BinaryExpression& be) {
      be.getLeft ().accept (*this);
      auto le = _internal->expr;
      be.getRight ().accept (*this);
      auto right = _internal->expr;
      switch (be.getOp ()) {
	case BinOps::Add:
	  _internal->expr = std::make_shared<FMTeach::IR::AddExpr> (std::move(le),std::move(right));
	  break;
      case BinOps::Sub:
	_internal->expr = std::make_shared<FMTeach::IR::SubExpr> (std::move(le),std::move(right));
	break;
      case BinOps::Mul:
	_internal->expr = std::make_shared<FMTeach::IR::MulExpr> (std::move(le),std::move(right));
	break;
      case BinOps::Div:
	_internal->expr = std::make_shared<FMTeach::IR::DivExpr> (std::move(le),std::move(right));
	break;
      case BinOps::LEq:
	_internal->expr = std::make_shared<FMTeach::IR::LEqExpr> (std::move(le),std::move(right));
	break;
      case BinOps::GEq:
	_internal->expr = std::make_shared<FMTeach::IR::GEqExpr> (std::move(le),std::move(right));
	break;
      case BinOps::Lt:
	_internal->expr = std::make_shared<FMTeach::IR::LtExpr> (std::move(le),std::move(right));
	break;
      case BinOps::Gt:
	_internal->expr = std::make_shared<FMTeach::IR::GtExpr> (std::move(le),std::move(right));
	break;
      case BinOps::Eq:
	_internal->expr = std::make_shared<FMTeach::IR::EqExpr> (std::move(le),std::move(right));
	break;
      case BinOps::NEq:
	_internal->expr = std::make_shared<FMTeach::IR::NEqExpr> (std::move(le),std::move(right));
	break;
	
	
      };
    }
    
    void Compiler::visitAssignStatement (const AssignStatement& ass) {
      auto reg = _internal->vars.at (ass.getAssignName ());
      ass.getExpression ().accept(*this);
      _internal->start->setName (static_cast<std::string> (ass.getFileLocation ()));
      auto end = _internal->cfa.makeLocation ("",false);
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Assign> (reg,_internal->expr),end);
      _internal->end = end;
    }

    void Compiler::visitAssertStatement (const AssertStatement& ass) {
      
      ass.getExpression ().accept(*this);
      auto expr = _internal->expr;
      auto nexpr = std::make_shared<FMTeach::IR::NegationExpr> (expr);
      auto assert_violated = _internal->cfa.makeLocation ("AssertViolation",false,true);
      _internal->start->setName (static_cast<std::string> (ass.getFileLocation ()));
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Assume> (nexpr),assert_violated);
      
      // continuation
      auto nloc = _internal->cfa.makeLocation ("",false);
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Assume> (expr),nloc);
      _internal->end = nloc;
    }

    void Compiler::visitAssumeStatement (const AssumeStatement& ass) {
      
      ass.getExpression ().accept(*this);
      auto expr = _internal->expr;
      
      // continuation
      auto nloc = _internal->cfa.makeLocation ("",false);
      _internal->start->setName (static_cast<std::string> (ass.getFileLocation ()));
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Assume> (expr),nloc);
      _internal->end = nloc;
    }
    
    void Compiler::visitNonDetAssignStatement (const NonDetAssignStatement& ass) {
      auto reg = _internal->vars.at (ass.getAssignName ());
      auto end = _internal->cfa.makeLocation ("",false);
      _internal->start->setName (static_cast<std::string> (ass.getFileLocation ()));
      _internal->start->addEdge (std::make_shared<FMTeach::IR::NonDetAssign> (reg),end);
      _internal->end = end;
    }
    
    void Compiler::visitIfStatement (const IfStatement& ifs ) {
      ifs.getCondition ().accept(*this);
      auto posExpr = _internal->expr;
      auto negExpr = std::make_shared<FMTeach::IR::NegationExpr> (_internal->expr);
      auto my_start = _internal->start;
      _internal->start->setName (static_cast<std::string> (ifs.getFileLocation ()));
      auto my_end = _internal->cfa.makeLocation ("",false);
      
      //IfBody
      {
	auto nloc = _internal->cfa.makeLocation ("",false);
	my_start->addEdge (std::make_shared<FMTeach::IR::Assume> (posExpr),nloc);
	_internal->start = nloc;
	ifs.getIfBody ().accept(*this);
	_internal->end->addEdge (std::make_shared<FMTeach::IR::Skip> (),my_end);
	
      }

      //else body
      {
	auto nloc = _internal->cfa.makeLocation ("",false);
	my_start->addEdge (std::make_shared<FMTeach::IR::Assume> (negExpr),nloc);
	_internal->start = nloc;
	ifs.getElseBody ().accept(*this);
	_internal->end->addEdge (std::make_shared<FMTeach::IR::Skip> (),my_end);
	
      }

      
      // Construct start edge for


      _internal->end = my_end;
    }
    void Compiler::visitSkipStatement (const SkipStatement& ass) {
      auto end = _internal->cfa.makeLocation ("",false);
      _internal->start->setName (static_cast<std::string> (ass.getFileLocation ()));
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Skip> (),end);
      _internal->end = end;
      
    }

    void Compiler::visitWhileStatement (const WhileStatement& whiles) {
      whiles.getCondition ().accept(*this);
      auto posExpr = _internal->expr;
      auto negExpr = std::make_shared<FMTeach::IR::NegationExpr> (_internal->expr);
      _internal->start->setName (static_cast<std::string> (whiles.getFileLocation ()));
      auto my_start = _internal->start;
      auto my_end = _internal->cfa.makeLocation ("",false);
      auto body =  _internal->cfa.makeLocation ("",false);
      
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Assume> (negExpr),my_end);
      _internal->start->addEdge (std::make_shared<FMTeach::IR::Assume> (posExpr),body);
      _internal->start = body;
      whiles.getBody ().accept(*this);
      _internal->end->addEdge (std::make_shared<FMTeach::IR::Skip> (),my_start);
      
      _internal->end = my_end;
    }

    
    void Compiler::visitMemAssignStatement (const MemAssignStatement& assign) {
        assign.getMemLoc ().accept (*this);
	auto mem = _internal->expr;
	assign.getExpression ().accept (*this);
	auto assigne = _internal->expr;
	auto my_end = _internal->cfa.makeLocation ("",false);
	_internal->start->setName (static_cast<std::string> (assign.getFileLocation ()));
      
	_internal->start->addEdge (std::make_shared<FMTeach::IR::Store> (assigne,mem),my_end);
	_internal->end = my_end;
	
      }
      
    
    void Compiler::visitSequenceStatement (const SequenceStatement&  s) {
      s.getFirst().accept (*this);
      _internal->start = _internal->end;
      s.getSecond().accept (*this);
      
    }
      
    
  }
}
