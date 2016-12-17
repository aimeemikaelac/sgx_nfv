#ifndef BASIC_ELEMENT_HH
#define BASIC_ELEMENT_HH
#include <click/config.h>
#include <click/element.hh>
#include <list>
CLICK_DECLS

static int list_size = 100;

class BasicElement: public Element {

public:
  BasicElement();
  ~BasicElement() {}
  const char *class_name() const { return "BasicElement"; }
  const char *port_count() const { return PORTS_1_1; }
  const char *processing() const { return PUSH; }
  void push(int port, Packet *p);
private:
  int count = 0;
  long int sgx_sum = 0;
  int index=0;
  // std::list<Packet*> packet_list;
  Packet *packet_list[10000];
  unsigned char* data_list[10000];
  long int sendData(unsigned char *data, unsigned int length);
};
CLICK_ENDDECLS
#endif
