#ifndef _WHILEY_COMPILER__
#define _WHILEY_COMPILER__

#include "whiley/ast.hpp"
#include "whiley/messaging.hpp"

namespace Whiley {
  class TypeChecker : private StatementVisitor,
		      private ExpressionVisitor {
  public:
    TypeChecker (MessageSystem& messaging = STDMessageSystem::get());
    ~TypeChecker ();
    bool CheckProgram (Program& prgm);
    
    void visitIdentifier (const Identifier&) override ;
    void visitNumberExpression (const NumberExpression& ) override ; 
    void visitDerefExpression (const DerefExpression& ) override ;
    void visitCastExpression (const CastExpression& ) override ;
    
    void visitBinaryExpression (const BinaryExpression& ) override ;  
    
    void visitAssignStatement (const AssignStatement& ) override ; 
    void visitAssertStatement (const AssertStatement& ) override ; 
    void visitAssumeStatement (const AssumeStatement& ) override ; 
    void visitNonDetAssignStatement (const NonDetAssignStatement& ) override ; 
    void visitIfStatement (const IfStatement& ) override ; 
    void visitSkipStatement (const SkipStatement& ) override ; 
    void visitWhileStatement (const WhileStatement& ) override ; 
    void visitSequenceStatement (const SequenceStatement& ) override ; 
    void visitMemAssignStatement (const MemAssignStatement&) override;
    
  private:
    [[nodiscard]] Type CheckExpression (Expression&);
    bool CheckStatement (Statement& s);
    
    
    struct Internal;
    std::unique_ptr<Internal> _internal;
    MessageSystem& messaging;
  };
}

#endif
