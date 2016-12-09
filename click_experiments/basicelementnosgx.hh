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
  void push(int port, Packet *p);
private:
  int count = 0;
  long int sendData(unsigned char *data, unsigned int length);
  void call_process_packet_no_sgx(unsigned char *data, unsigned int length, unsigned char *hash);
};
CLICK_ENDDECLS
#endif 
