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


%token END 0 "end of file"
%token <std::string>    IDENTIFIER
%token <std::int64_t>   NUMBER
%token <std::int64_t>   CHAR

%token <Type>    TYPE

%locations

%%

prgm : decllist stmtlist  {}
decllist :  decllist decl | decl
decl : TYPE IDENTIFIER SEMI { builder.DeclareStmt ($2,$1,@$);}
       

stmtlist : stmtlist stmt {builder.SequenceStmt (@$);} | stmt

stmt : simpstmt  | selectivestmt | iterativestmt

selectivestmt : IF LPARAN expr RPARAN LBRACE stmtlist RBRACE ELSE LBRACE stmtlist RBRACE {builder.IfStmt (@$);}
              | IF LPARAN expr RPARAN LBRACE stmtlist RBRACE {builder.SkipStmt (@$);builder.IfStmt (@$);}

iterativestmt : WHILE LPARAN expr RPARAN LBRACE stmtlist RBRACE {builder.WhileStmt (@$);}

simpstmt : IDENTIFIER ASS expr SEMI { builder.AssignStmt ($1,@$);}
| SKIP SEMI {builder.SkipStmt (@$);}
| DEREF expr ASS expr SEMI {builder.MemAssignStmt (@$);}
| ASSERT LPARAN expr RPARAN SEMI {builder.AssertStmt (@$);} 
| ASSUME LPARAN expr RPARAN SEMI {builder.AssumeStmt (@$);} 

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
             | arith_factor

arith_factor : NUMBER {builder.NumberExpr ($1,@$);}
             | CHAR {builder.NumberExpr ($1,@$);builder.CastExpr(Type::SI8,@$);}
             | IDENTIFIER {builder.IdentifierExpr ($1,@$);}
             | LPARAN expr RPARAN
	     | NONDET {builder.UndefExpr (Whiley::Type::SI8,@$);}
             | NONDETTYPE TYPE {builder.UndefExpr ($2,@$);}
             | DEREFEXPR expr  AS TYPE DEREFEXPR{builder.DerefExpr ($4,@$); }
             | LPARAN expr AS TYPE RPARAN {builder.CastExpr ($4,@$);}

%%


void  Whiley::Parser::error( const location_type& l, const std::string &err_message )
{
  std::stringstream str;
  str << "Error: " << err_message << " at " << l << "\n";
  messager << StringMessage (str.str());
}
