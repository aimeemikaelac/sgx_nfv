#ifndef BASIC_ELEMENT_NO_SGX_HH
#define BASIC_ELEMENT_NO_SGX_HH
#include <click/element.hh>
CLICK_DECLS

class BasicElementNoSGX: public Element {

public:
  BasicElementNoSGX();
  ~BasicElementNoSGX() {}
  const char *class_name() const { return "BasicElementNoSGX"; }
  const char *port_count() const { return PORTS_1_1; }
  const char *processing() const { return PUSH; }
  void add_handlers() CLICK_COLD;
  void push(int port, Packet *p);
  long int sgx_sum = 0;
private:
  int count = 0;
  long int sendData(unsigned char *data, unsigned int length);
  int call_process_packet_no_sgx(unsigned char *data, unsigned int length);
  static String sgx_read_handler(Element *, void *) CLICK_COLD;
};
CLICK_ENDDECLS
#endif
