#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parser.hh"
#include "whiley/options.hpp"

  namespace Whiley {
    class Scanner : public yyFlexLexer{
    public:
      
      Scanner(std::istream *in, Whiley::TypeFlags flags = Whiley::TypeFlags::All ()) : yyFlexLexer(in),enabled(std::move(flags)) {
    };
      
      using FlexLexer::yylex;
      
      virtual int yylex( Whiley::Parser::semantic_type * const lval, 
			 Whiley::Parser::location_type *location );   
      
    private:
      Whiley::Parser::token::token_kind_type type_res (Type t) {
	if (enabled.isSet(t))  {
	  yylval->emplace<Type> (t); return Whiley::Parser::token::TYPE;
	}
	else
	  yylval->emplace<std::string> (yytext); return Whiley::Parser::token::IDENTIFIER;
      }
      /* yyval ptr */
      Whiley::Parser::semantic_type *yylval = nullptr;
      /* location ptr */
      Whiley::Parser::location_type *loc    = nullptr;
      Whiley::TypeFlags enabled;
    };

  }

#endif 
