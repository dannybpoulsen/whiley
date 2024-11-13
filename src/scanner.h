#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__ 

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parser.hh"

  namespace Whiley {
    
    class Scanner : public yyFlexLexer{
    public:
      
    Scanner(std::istream *in) : yyFlexLexer(in) {
      //loc = new FMTeach::Whiley::Parser::location_type();
    };
      
      //get rid of override virtual function warning
      using FlexLexer::yylex;
      
      virtual int yylex( Whiley::Parser::semantic_type * const lval, 
			 Whiley::Parser::location_type *location );   
      
    private:
      /* yyval ptr */
      Whiley::Parser::semantic_type *yylval = nullptr;
      /* location ptr */
      Whiley::Parser::location_type *loc    = nullptr;
    };

  }

#endif 
