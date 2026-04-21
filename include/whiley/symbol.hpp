#ifndef _WHILEY_SYMBOL__
#define _WHILEY_SYMBOL__


#include <memory>
#include <variant>
#include <generator>
#include <vector>
#include <optional>

namespace Whiley {

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
      Pointer
  };
  
  struct VarDecl {
    VarDecl (Type type, bool parameter, bool output) : type(type),parameter(parameter),output(output) {}
    
    Type type;
    bool parameter;
    bool output;
  };

  struct ParamDecl {
    ParamDecl (Type type) : type(type) {}
    
    Type type;
  };
  
  
  class Statement;
  using Statement_ptr = std::shared_ptr<Statement>;

  class Expression;
  using Expression_ptr = std::shared_ptr<Expression>;
  
  class Function;
  using Function_ptr = std::shared_ptr<Function>;

  using UserData = std::variant<std::monostate,VarDecl,Function_ptr,ParamDecl,Expression_ptr>;
  
  class Symbol {
  private:
    struct Internal;
    std::shared_ptr<Internal> internal;
  public:
    std::string getName() const;
    std::string getFullName() const;
    void setUserData (UserData);
    const UserData& getUserData () const ;
    Symbol& operator=(const Symbol&) =  default;
  public:
    Symbol (const Symbol& s) : internal(s.internal) {}
    Symbol (Symbol parent,std::string);
    Symbol (std::string s);
    std::size_t hash () const;
  };
  
  class Frame {
  public:
    Frame (std::string s);
    Frame (const Frame&f )=default;
    Frame open(std::string s);
    Frame close ();
    Frame create(std::string s);

    std::optional<Symbol> resolve(const std::string& s) const;
    
    /*Symbol resolve(const std::string& s) const;
    bool resolve(const std::string& s, Symbol&) const;
    */
    
    Symbol createSymbol (std::string s);
    std::generator<Symbol> getLocalSymbols() const ; 
  private:
    struct Internal;
    std::shared_ptr<Internal> _internal;
    
    Frame (std::shared_ptr<Internal>&& s) : _internal(std::move(s)) {}
    Frame (const std::shared_ptr<Internal>& s) : _internal(s) {}
    
   };

  class Function {
  public:
    Function (Whiley::Frame f, Statement_ptr&& stmt, std::vector<Symbol>&& params,Type retType);
    ~Function();
    auto returns() const {return returnType;}
    auto& getStmt() const {return stmt;}
    auto getFrame() const {return frame;}
    auto& getParams() const {return parameters;} 
  private:
    Whiley::Frame frame;
    Type returnType;
    Statement_ptr stmt;
    std::vector<Symbol> parameters;
  };
  
  
}


#endif
