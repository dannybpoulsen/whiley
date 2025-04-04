%{
/* C++ string header, for string ops below */
#include <string>
#include "scanner.h"
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
"var"      {return token::VAR;}
"ui8"      {return token::UI8;}
"si8"      {return token::SI8;}
"while"      {return token::WHILE;}
"if"      {return token::IF;}
"else"      {return token::ELSE;}
"skip"      {return token::SKIP;}
"+"  {return token::PLUS;}
"-"  {return token::MINUS;}
"/"  {return token::DIV;}
"*"  {return token::MUL;}
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
"assert" {return token::ASSERT;}
"assume" {return token::ASSUME;}
"as" {return token::AS;}



<<EOF>> {return token::END;}
[a-z]+       {yylval->emplace<std::string> (yytext); return token::IDENTIFIER; }
[0-9]+  {yylval->emplace<std::int8_t> (std::stoi(yytext)); return token::NUMBER; }
-[1-9][0-9]*  {yylval->emplace<std::int8_t> (std::stoi(yytext)); return token::NUMBER; }
\'[a-zA-Z0-9]\' {yylval->emplace<std::int8_t> (yytext[1]); return token::NUMBER;}

[\n] {loc->newline (); }
[\t ' '] {loc->movecol(strlen(yytext));}
			    
.           {std::cerr << "Unexpected text" << yytext << std::endl;}
%%
