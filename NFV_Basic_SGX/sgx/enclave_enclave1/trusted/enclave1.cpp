#include "stdarg.h"
#include "stdio.h"     /* vsnprintf */

#include "enclave1.h"
#include "enclave1_t.h"  /* print_string */
#include "sgx_tcrypto.h"
#include "string.h"
#include "math.h"

/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
//void printf(const char *fmt, ...)
//{
//    char buf[BUFSIZ] = {'\0'};
//    va_list ap;
//    va_start(ap, fmt);
//    vsnprintf(buf, BUFSIZ, fmt, ap);
//    va_end(ap);
//    ocall_enclave1_sample(buf);
//}
//
//int ecall_enclave1_sample()
//{
//  printf("IN ENCLAVE1\n");
//  return 0;
//}

void ecall_process_packet_sha256(unsigned char* data, unsigned int length, unsigned char hash_out[SGX_SHA256_HASH_SIZE]){
    sgx_status_t status;
    sgx_sha256_hash_t hash;
    status = sgx_sha256_msg(data, length, &hash);
    if(status != SGX_SUCCESS){
        ocall_print("Error calculating SHA256 message\n");
        return;
    }
    memcpy(hash_out, &hash, SGX_SHA256_HASH_SIZE);
}

void ecall_process_packet(unsigned int *valid, unsigned char** data_in, unsigned int *length){
  int index=0, count = 0, sequence_len;// = ceil(length/500);
  unsigned char *search_seq;// = data;
  unsigned char *curr;//= data;
  unsigned char *data;
  while(true){
    if(*valid == 0){
      continue;
    }
    ocall_print("in enclave\n");
    data = *data_in;
    sequence_len = ceil(*length/500);
    search_seq = data;
    curr = data;
    while(index<(*length-sequence_len)){
      if(memcmp(curr, search_seq, sequence_len) == 0){
        curr += sequence_len;
        index += sequence_len;
        count ++;
      } else{
        curr++;
        index++;
      }
    }
    *valid = 0;
    ocall_print("finishing enclave iteration\n");
  }
}
