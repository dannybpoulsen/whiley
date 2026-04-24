%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {Whiley}
%define api.parser.class {Parser}
%define api.location.type {Whiley::location_t}


%code requires{
    namespace Whiley {
      class Scanner;
      class ASTBuilder;
    }



#include <cstdint>
#include <string>
#include "whiley/ast.hpp"
#include "whiley/messaging.hpp"
    
 }

%parse-param { Whiley::Scanner& scanner } {Whiley::ASTBuilder& builder } {Whiley::MessageSystem& messager}
%initial-action
{

};

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include "scanner.h"
#undef yylex
#define yylex scanner.yylex
  
}

%define api.value.type variant
%define parse.assert

%token    WHILE
%token    IF
%token    ELSE
%token    SKIP
%token    PLUS
%token    MINUS
%token    DIV
%token    MUL
%token    MOD
%token    DEREF
%token    DEREFEXPR
%token    LEQ
%token    GEQ
%token    LT
%token    GT
%token    EQ
%token    NEQ
%token    ASS
%token    SEMI
%token    LPARAN
%token    RPARAN
%token    LBRACE
%token    RBRACE
%token    NONDET
%token    NONDETTYPE
%token    ASSERT
%token    ASSUME
%token    AS
%token    PARAM
%token    OUTPUT
%token    CHOOSE
%token    SELECTOR
%token    FUNCTION
%token    ARROW 
%token    COMMA
%token    RETURN
%token    ALLOC
%token    FREE
%token    FOR
%token    INCREMENT
%token    XOR
%token    BITOR
%token    BITAND
%token    LSHL
%token    CONSTANT
%token    LBRACK
%token    RBRACK


%token END 0 "end of file"
%token <std::string>    IDENTIFIER
%token <std::int64_t>   NUMBER
%token <std::int64_t>   CHAR

%token <Type>    TYPE
%type<std::size_t> stmt_list;
%type<std::size_t> expr_list; 
%locations

%%

prgm : decllist functionlist stmtlist  {}
decllist :  decllist decl | /*empty*/
decl : TYPE IDENTIFIER SEMI { builder.DeclareStmt ($2,$1,false,false,@$);}
     | PARAM TYPE IDENTIFIER SEMI { builder.DeclareStmt ($3,$2,true,false,@$);}
     | OUTPUT TYPE IDENTIFIER SEMI { builder.DeclareStmt ($3,$2,false,true,@$);}
     | CONSTANT IDENTIFIER  ASS expr SEMI {builder.Constant ($2);}

paramlist :  param | /*empty*/ | paramlist COMMA param
param : TYPE IDENTIFIER { builder.ParamDeclare ($2,$1,@$);}


functionlist : functionlist function |  /* empty */  
function : FUNCTION IDENTIFIER {builder.FunctionBegin ($2);} LPARAN paramlist  RPARAN ARROW TYPE LBRACE decllist stmtlist RBRACE  {builder.FunctionEnd ($8);} 


stmtlist : stmtlist stmt {builder.SequenceStmt (@$);} | stmt

stmt : simpstmt  | selectivestmt | iterativestmt

stmt_list : SELECTOR LBRACE stmtlist RBRACE  {$$ =1;}
| stmt_list SELECTOR LBRACE stmtlist RBRACE {$$ = $1+1;}

expr_list :  expr {$$ =1;} | /*empty */ {$$ = 0;}
| expr_list COMMA expr {$$ = $1+1;}


selectivestmt : IF LPARAN expr {builder.IfCond();}  RPARAN  LBRACE stmtlist RBRACE if_else
              | CHOOSE LBRACE stmt_list RBRACE {builder.ChooseStmt ($3,@$);}

if_else       : ELSE LBRACE stmtlist RBRACE {builder.IfStmt (@$);} | /*empty*/ {builder.SkipStmt(@$);builder.IfStmt (@$);}

iterativestmt : WHILE LPARAN expr  RPARAN {builder.WhileCond ();} LBRACE stmtlist RBRACE {builder.WhileStmt (@$);}
| FOR LPARAN IDENTIFIER ASS expr  SEMI {builder.AssignStmt ($3,@$);} expr {builder.WhileCond();} SEMI INCREMENT IDENTIFIER  RPARAN LBRACE stmtlist RBRACE {builder.Increment ($12,@$); builder.SequenceStmt (@4); builder.WhileStmt (@$);  builder.SequenceStmt (@$);} 


simpstmt : IDENTIFIER ASS expr SEMI { builder.AssignStmt ($1,@$);}
| IDENTIFIER ASS ALLOC expr SEMI { builder.AllocStmt ($1,@$);}
| FREE expr SEMI { builder.FreeStmt (@$);}
| SKIP SEMI {builder.SkipStmt (@$);}
| DEREF expr ASS expr SEMI {builder.MemAssignStmt (@$);}
| ASSERT  expr  SEMI {builder.AssertStmt (@$);} 
| ASSUME  expr SEMI {builder.AssumeStmt (@$);}
| RETURN expr SEMI {builder.ReturnStmt (@$);}
| IDENTIFIER ASS IDENTIFIER LPARAN expr_list RPARAN SEMI {builder.CallStmt ($1,$3,$5,@$);};
| IDENTIFIER LPARAN expr_list RPARAN SEMI {builder.CallStmt ("",$1,$3,@$);};

| IDENTIFIER INCREMENT SEMI {builder.Increment ($1,@$);}

expr : arith_expr | bool_expr

bool_expr    : arith_expr LEQ arith_expr{builder.BinaryExpr (Whiley::BinOps::LEq,@$);}
             | arith_expr GEQ arith_expr{builder.BinaryExpr (Whiley::BinOps::GEq,@$);}
             | arith_expr LT arith_expr{builder.BinaryExpr (Whiley::BinOps::Lt,@$);}
             | arith_expr GT arith_expr{builder.BinaryExpr (Whiley::BinOps::Gt,@$);}
             | arith_expr EQ arith_expr{builder.BinaryExpr (Whiley::BinOps::Eq,@$);}
             | arith_expr NEQ arith_expr{builder.BinaryExpr (Whiley::BinOps::NEq,@$);}

arith_expr   : arith_expr PLUS arith_term {builder.BinaryExpr (Whiley::BinOps::Add,@$);}
             | arith_expr MINUS arith_term {builder.BinaryExpr (Whiley::BinOps::Sub,@$);}
             | arith_term
	     
arith_term   : arith_term MUL arith_factor {builder.BinaryExpr (Whiley::BinOps::Mul,@$);}
             | arith_term DIV arith_factor {builder.BinaryExpr (Whiley::BinOps::Div,@$);}
             | arith_term MOD arith_factor {builder.BinaryExpr (Whiley::BinOps::Mod,@$);}
             | arith_term XOR arith_factor {builder.BinaryExpr (Whiley::BinOps::Xor,@$);}
             | arith_term BITOR arith_factor {builder.BinaryExpr (Whiley::BinOps::Or,@$);}
             | arith_term BITAND arith_factor {builder.BinaryExpr (Whiley::BinOps::And,@$);}
             | arith_term LSHL arith_factor {builder.BinaryExpr (Whiley::BinOps::LShl,@$);}
             | arith_factor

arith_factor : NUMBER {builder.NumberExpr ($1,@$);}
             | CHAR {builder.NumberExpr ($1,@$);builder.CastExpr(Type::SI8,@$);}
             | IDENTIFIER {builder.IdentifierExpr ($1,@$);}
             | LPARAN expr RPARAN
	     | NONDET {builder.UndefExpr (Whiley::Type::SI8,@$);}
             | NONDETTYPE TYPE {builder.UndefExpr ($2,@$);}
             | DEREFEXPR expr  AS TYPE DEREFEXPR{builder.DerefExpr ($4,@$); }
             | LPARAN expr AS TYPE RPARAN {builder.CastExpr ($4,@$);}
             | IDENTIFIER LBRACK expr_list RBRACK {builder.CallExpr ($1,$3,@$);}

%%


void  Whiley::Parser::error( const location_type& l, const std::string &err_message )
{
  std::stringstream str;
  str << "Error: " << err_message << " at " << l << "\n";
  messager << StringMessage (str.str());
}
