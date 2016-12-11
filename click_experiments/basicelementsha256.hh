#ifndef BASIC_ELEMENT_SHA_256_HH
#define BASIC_ELEMENT_SHA_256_HH
#include <click/element.hh>
CLICK_DECLS

class BasicElementSHA256: public Element {

public:
  BasicElementSHA256();
  ~BasicElementSHA256() {}
  const char *class_name() const { return "BasicElementSHA256"; }
  const char *port_count() const { return PORTS_1_1; }
  const char *processing() const { return PUSH; }
  void push(int port, Packet *p);
private:
  int count = 0;
  long int sendData(unsigned char *data, unsigned int length);
};
CLICK_ENDDECLS
#endif 
