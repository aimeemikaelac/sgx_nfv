#ifndef _ENCLAVE1_H_
#define _ENCLAVE1_H_

#include <stdlib.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

void printf(const char *fmt, ...);

typedef struct enclave_packet_data {
  volatile unsigned char* packet_data;
  unsigned int length;
  long int result;
  long int packet_id;
  // volatile struct enclave_packet_data *next;
} enclave_packet_data;

typedef struct enclave_node {
  volatile enclave_packet_data *payload;
  struct enclave_node *next;
} enclave_node;

typedef struct enclave_packet_queue {
  volatile enclave_node *first;
  volatile enclave_node *divider;
  volatile enclave_node *last;
} enclave_packet_queue;



#if defined(__cplusplus)
}
#endif

#endif /* !_ENCLAVE1_H_ */
