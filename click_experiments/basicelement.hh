#ifndef BASIC_ELEMENT_HH
#define BASIC_ELEMENT_HH
#include <click/element.hh>
CLICK_DECLS

class BasicElement: public Element {

public:
  BasicElement();
  ~BasicElement() {}
  const char *class_name() const { return "BasicElement"; }
  const char *port_count() const { return PORTS_1_1; }
  const char *processing() const { return PUSH; }
  void add_handlers() CLICK_COLD;
  void push(int port, Packet *p);
  long int sgx_sum = 0;
private:
  int count = 0;
  long int sendData(unsigned char *data, unsigned int length);
  static String sgx_read_handler(Element *, void *) CLICK_COLD;
};
CLICK_ENDDECLS
#endif
