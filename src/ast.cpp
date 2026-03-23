 #include "whiley/ast.hpp"

  namespace Whiley {

    class OutputVisitor : private NodeVisitor {
    public:
      OutputVisitor (std::ostream& o) : os(o) {}
      void visitIdentifier (const Identifier& ident) override {
	os << ident.getSymbol ().getName();
      }
      
      void visitNumberExpression (const NumberExpression& number) override {
	os << static_cast<int> (number.getValue ());
		
      }

      void visitDerefExpression (const DerefExpression& expr) override {
	os << "*";
	expr.getMem ().accept (*this);
	
	
      }

      void visitUndefExpression (const UndefExpression&) override {
	os << "?";
	
	
      }
      
      void visitCastExpression (const CastExpression& expr) override {
	
	expr.getExpression ().accept (*this);
	os << " as " << expr.getType () ;
	
      }
      
      
      void visitBinaryExpression (const BinaryExpression& binary) override {
	os << "(";
	binary.getLeft ().accept(*this);
	switch (binary.getOp ()) {
	case BinOps::Add:
	  os << " + ";
	  break;
	case BinOps::Sub:
	  os << " - ";
	  break;
	case BinOps::Mul:
	  os << " * ";
	  break;
	case BinOps::Div:
	  os << "/ ";
	  break;
	case BinOps::LEq:
	  os << "  <= ";
	  break;
	case BinOps::GEq:
	  os << "  >= ";
	  break;
	case BinOps::Lt:
	  os << "  < ";
	  break;
	case BinOps::Gt:
	  os << "  > ";
	  break;
	case BinOps::Eq:
	  os << " == ";
	  break;
	case BinOps::NEq:
	  os << "  != ";
	  break;
	case BinOps::Mod:
	  os << "  % ";
	  break;
	
	}
	binary.getRight ().accept (*this);
	os << ")";
      }
      void visitAssignStatement (const AssignStatement& ass) override {
	os << ass.getAssignName () << " = ";
	ass.getExpression ().accept (*this);
	os << ";\n";
      }

      void visitAllocStatement (const AllocStatement& ass) override {
	os << ass.getAssignName () << " = alloc";
	ass.getExpression ().accept (*this);
	os << ";\n";
      }

      void visitFreeStatement (const FreeStatement& ass) override {
	os <<  "free ";
	ass.getExpression ().accept (*this);
	os << ";\n";
      }
      
      void visitAssertStatement (const AssertStatement& ass) override {
	os << "Assert ";
	ass.getExpression ().accept (*this);
	os << ";\n";
      }

      void visitAssumeStatement (const AssumeStatement& ass) override {
	os << "Assume ";
	ass.getExpression ().accept (*this);
	os << ";\n";
      }

      virtual void visitMemAssignStatement (const MemAssignStatement& assign) {
	os << "*";
	assign.getMemLoc ().accept (*this);
	os << " = ";
	assign.getExpression ().accept (*this);
	os << ";\n";
      }
      
      
      void visitIfStatement (const IfStatement& ifs) override {
	os << "if (";
	ifs.getCondition ().accept(*this);
	os <<" ) {\n";
	ifs.getIfBody ().accept(*this);
	os << "\n}\n{";
	ifs.getElseBody ().accept (*this);
	os << "}\n";
      }
      
      void visitSkipStatement (const SkipStatement& ) override {
	os << "Skip;\n";
      }

      void visitReturnStatement (const ReturnStatement& r) override {
	os << "return ";
	r.getExpr().accept(*this);
      }

      void visitCallStatement (const CallStatement& r) override {
	os << r.assignname() << " = " << r.funcname () <<  "(";
	for (auto& p : r.parameters()) {
	  p->accept(*this);
	  os << "," ;
	}
	os << ")"; 
      }
      
      void visitWhileStatement (const WhileStatement& whiles) override {
	os << "while (";
	whiles.getCondition ().accept(*this);
	os <<" ) {\n";
	whiles.getBody ().accept(*this);
	os << "\n}";
      }

      void visitChooseStatement (const ChooseStatement& whiles) override {
	os << "choose {";
	for (auto& s :  whiles.getStatements()) {
	  os << ":: ";
	  s->accept(*this);
	  os << "\n";
	}
	  
	os << "\n}";
      }

      void visitIncrementDecrementStatement (const IncrementDecrementStatement& ass)  {

	os << ass.getIncrementee () << "++;\n"; 
	  }
      
      void visitSequenceStatement (const SequenceStatement& seg) override {
	seg.getFirst ().accept(*this);
	seg.getSecond ().accept(*this);
	
      }

      template<class N>
      auto& operator() (const N& n) {
	n.accept (*this);
	return os;
      }
      
    private:
      std::ostream& os;
    };

    std::ostream& operator<< (std::ostream& os, const Statement& n)  {
      return OutputVisitor{os} (n);
      
    };
    
    
    std::ostream& operator<< (std::ostream& os, const Whiley::Frame& f) {
      for (auto s : f.getLocalSymbols ()) {
	std::visit (overloaded {
	    [&s,&os](const VarDecl& decl)->void {
	      os << decl.type << " "  << s.getName() << "\n"; 
	    },
	    [&s,&os](const Function_ptr& func)->void {
	      os << "fn " << s.getName() << " (";
	      for (auto t: func->getParams()) {
		if (std::holds_alternative<ParamDecl> (t.getUserData()))
		  os << std::get<ParamDecl> (t.getUserData()).type << " " << t.getName() <<",";
	      }
		
	      os << ") -> " << func->returns () << "{\n";
	      os << func->getFrame() << "\n";
	      os << *func->getStmt() << "\n";
	      os << "}\n";
	    },
	      [](auto&) {}
	      
	  },
	  s.getUserData ()
	  );
      }
      return os;
      
    }
  }
