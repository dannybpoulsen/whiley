%{
/* C++ string header, for string ops below */
#include <string>
#include "scanner.h"
#include "whiley/ast.hpp"

/* Implementation of yyFlexScanner */ 
#undef  YY_DECL
#define YY_DECL int Whiley::Scanner::yylex(Whiley::Parser::semantic_type * const lval, Whiley::Parser::location_type *location )

/* using "token" to make the returns for the tokens shorter to type */
using token = Whiley::Parser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return( token::END )

#define YY_USER_ACTION  loc->movecol(strlen(yytext));

%}

%option debug
%option nodefault
%option yyclass="Whiley::Scanner"
%option noyywrap
%option yylineno
%option c++

%%
%{
yylval = lval;
loc = location;
loc->begin = loc->end;
%}
"param"    {return token::PARAM;}
"output"    {return token::OUTPUT;}
"var"      {return type_res (Type::SI8);}
"si8"      {return type_res(Type::SI8);}
"ui8"      {return type_res(Type::UI8);}
"si16"      {return type_res(Type::SI16);}
"ui16"      {return type_res(Type::UI16);}
"si32"      {return type_res(Type::SI32);}
"ui32"      {return type_res(Type::UI32);}
"si64"      {return type_res(Type::SI64);}
"ui64"      {return type_res(Type::UI64);}
"ptr"      {return type_res(Type::Pointer);}

"while"      {return token::WHILE;}
"if"      {return token::IF;}
"else"      {return token::ELSE;}
"skip"      {return token::SKIP;}
"+"  {return token::PLUS;}
"-"  {return token::MINUS;}
"/"  {return token::DIV;}
"*"  {return token::MUL;}
"%"  {return token::MOD;}
"#"  {return token::DEREF;}
"$"  {return token::DEREFEXPR;}
"<="  {return token::LEQ;}
">="  {return token::GEQ;}
"<"  {return token::LT;}
">"  {return token::GT;}
"=="  {return token::EQ;}
"!="  {return token::NEQ;}
"="  {return token::ASS;}
";"  {return token::SEMI;}
"("  {return token::LPARAN;}
")"  {return token::RPARAN;}
"{"  {return token::LBRACE;}
"}"  {return token::RBRACE;}
"?"  {return token::NONDET;}
"??"  {return token::NONDETTYPE;}
"fn " {return token::FUNCTION;}
"->"  { return token::ARROW;}

"assert" {return token::ASSERT;}
"assume" {return token::ASSUME;}
"as" {return token::AS;}
"choose" { return token::CHOOSE;}
"::" { return token::SELECTOR;}
"," {return token::COMMA;}
"return" {return token::RETURN;}
"alloc" {return token::ALLOC;}
"free" {return token::FREE;}
"for" {return token::FOR;}
"++" {return token::INCREMENT;}

<<EOF>> {return token::END;}
[a-zA-Z_][a-zA-Z0-9_]*       {yylval->emplace<std::string> (yytext); return token::IDENTIFIER; }
[0-9]+  {yylval->emplace<std::int64_t> (std::stoll(yytext)); return token::NUMBER; }
-[1-9][0-9]*  {yylval->emplace<std::int64_t> (std::stoll(yytext)); return token::NUMBER; }
\'[a-zA-Z0-9]\' {yylval->emplace<std::int64_t> (yytext[1]); return token::CHAR;}
0[xX][0-9a-fA-F]+  {yylval->emplace<std::int64_t> (std::stoll(yytext,0,16)); return token::NUMBER; }


[\n] {loc->newline (); }
[\t ' '] {loc->movecol(strlen(yytext));}
			    
.           {std::cerr << "Unexpected text" << yytext << std::endl;}
%%
