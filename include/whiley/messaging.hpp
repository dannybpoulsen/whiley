#ifndef _MESSAGING__
#define _MESSAGING__

#include <string>
#include <memory>


namespace Whiley {
  class Message {
  public:
    ~Message () {}
    virtual std::string to_string () const = 0;
    
  };

  class StringMessage : public Message{
  public:
    StringMessage (std::string m) : m(std::move(m)) {}
    std::string to_string () const {return m;}
  private:
    std::string m;
  };
  
  class MessageSystem {
  public:
    ~MessageSystem () {}
    virtual MessageSystem& operator<< (const Message&) = 0;
  };

  class STDMessageSystem : public MessageSystem{
  public:
    MessageSystem& operator<< (const Message& m) override {
      std::cout << m.to_string () << "\n";
      return *this;
    }

    static STDMessageSystem&  get () {
      static STDMessageSystem system;
      return system;
    }
    
  private:
    STDMessageSystem () {}
    
  };

  
}

#endif
