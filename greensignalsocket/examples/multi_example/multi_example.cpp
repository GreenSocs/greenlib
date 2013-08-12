#include "multi_example.h"
int sc_main(int argc, char** argv)
{

  Master1 m1("m1");
  Master2 m2("m2");
  Slave  s("s");
  
  m1.socket(s.socket);
  m2.socket(s.socket);
  std::string name = m1.socket.name();
  s.socket.set_source<bar>(name);
  s.socket.set_source<foo>(name);
  
  //s.socket.set_source<bar>(m2.socket.name());
  sc_core::sc_start();

  return 0;
}

