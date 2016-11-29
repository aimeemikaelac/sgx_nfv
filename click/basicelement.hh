#ifndef BASIC_ELEMENT_HH
#define BASIC_ELEMENT_HH
#include <click/element.hh>
CLICK_DECLS

class BasicElement: public Element {

public:
  BasicElement() {}
  ~BasicElement() {}
  const char *class_name() const { return "BasicElement"; }
  const char *port_count() const { return PORTS_1_1; }
  const char *processing() const { return PUSH; }
  void push(int port, Packet *p);
private:
  int count = 0;
};
CLICK_ENDDECLS
#endif 
