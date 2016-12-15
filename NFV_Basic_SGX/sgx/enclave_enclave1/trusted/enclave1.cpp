#include "stdarg.h"
#include "stdio.h"     /* vsnprintf */

#include "enclave1.h"
#include "enclave1_t.h"  /* print_string */
#include "sgx_tcrypto.h"
#include "string.h"
#include "math.h"

int global_test = 0;

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

void ecall_process_packet(unsigned char** data_in_buffer,
                          unsigned int *data_out_buffer,
                          int* buffer_in_write,
                          int* buffer_in_read,
                          int* buffer_out_write,
                          int* buffer_out_read,
                          int* length){
  int i, index=0, count = 0, sequence_len, iteration=0, wait_count=0;// = ceil(length/500);
  unsigned char *search_seq;// = data;
  unsigned char *curr;//= data;
  unsigned char *data;
  char buff[100];
  ocall_print("enclave start\n");
//  volatile int *buffer_in_read_local = buffer_in_read;
//  volatile int *buffer_in_write_local = buffer_in_write;
//  volatile int *buffer_out_read_local = buffer_out_read;
//  volatile int *buffer_out_write_local = buffer_out_write;
  while(true){
    count = 0;
    while(*((volatile int*)buffer_in_read) == *((volatile int*)buffer_in_write)){
      __asm__ __volatile__("");
    }
    data = data_in_buffer[*buffer_in_read];
    *((volatile int*)buffer_in_read) = (*((volatile int*)buffer_in_read) + 1) % 50;
//    sequence_len = ceil(*length/500);
//    search_seq = data;
//    curr = data;
//    while(index<(*length-sequence_len)){
//      if(memcmp(curr, search_seq, sequence_len) == 0){
//        curr += sequence_len;
//        index += sequence_len;
//        count ++;
//      } else{
//        curr++;
//        index++;
//      }
//    }
    for(i=0; i<*length; i++){
        if(data[i] == 0x0a){
            count++;
        }
    }
    while(*((volatile int*)buffer_out_write) == (*((volatile int*)buffer_out_read) - 1) % 50){
      __asm__ __volatile__("");
    }
    data_out_buffer[*buffer_out_write] = count;
    *((volatile int*)buffer_out_write) = (*((volatile int*)buffer_out_write) + 1) % 50;
    iteration++;
//    ocall_print_int_message("Current iteration: %i\n", iteration);
  }
}
