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

int consume(volatile enclave_packet_queue *queue, volatile enclave_packet_data *data){
  ocall_print("In enclave consume\n");
  if(queue->divider != queue->last){
    data = queue->divider->next->payload;
    queue->divider = queue->divider->next;
    ocall_print("Out of enclave consume\n");
    return 0;
  } else{
    ocall_print("Out of enclave consume fail\n");
    return -1;
  }

}

void produce(volatile enclave_packet_queue *queue, volatile enclave_packet_data *data){
  ocall_print("In enclave produce\n");
  queue->last->next = data;
  queue->last = queue->last->next;
  while(queue->first != queue->divider){
    volatile enclave_packet_data *tmp = queue->first;
    queue->first = queue->first->next;
    //memory leak here. can only free with an ocall
    //if we make them doubly linked, tnen maybe we can free
    //in the consumer?
    //ocall_free(tmp);
  }
  ocall_print("Out of enclave produce\n");
}

void ecall_process_packet(void* queue_in, void* queue_out){
  int i, index=0, count = 0, sequence_len, iteration=0, wait_count=0;// = ceil(length/500);
  volatile enclave_packet_queue *p_queue_in = (volatile enclave_packet_queue*)(queue_in);
  volatile enclave_packet_queue *p_queue_out = (volatile enclave_packet_queue*)(queue_out);
  volatile enclave_packet_data *current_node;
  unsigned char *search_seq;// = data;
  unsigned char *curr;//= data;
  volatile unsigned char *data;
  int length;
  char buff[100];
  ocall_print("enclave start\n");
//  volatile int *buffer_in_read_local = buffer_in_read;
//  volatile int *buffer_in_write_local = buffer_in_write;
//  volatile int *buffer_out_read_local = buffer_out_read;
//  volatile int *buffer_out_write_local = buffer_out_write;
  while(true){
    count = 0;
    ocall_print("Start iteration\n");
    // while(*((volatile int*)buffer_in_read) == *((volatile int*)buffer_in_write){
    //   __asm__ __volatile__("");
    // }
    // while(p_queue_in->next == NULL){
    //   __asm__ __volatile("");
    // }
    if(consume(p_queue_in, current_node) < 0){
      ocall_print("Consuming\n");
      continue;
    }
    // p_queue_in = p_queue_in->next;
    data = current_node->packet_data;
    length = current_node->length;
    // data = data_in_buffer[*buffer_in_read];
    // *((volatile int*)buffer_in_read) = (*((volatile int*)buffer_in_read) + 1) % 50;
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
    for(i=0; i<length; i++){
        if(data[i] == 0x0a){
            count++;
        }
    }
    // while(*((volatile int*)buffer_out_write) == (*((volatile int*)buffer_out_read) - 1) % 50){
    //   __asm__ __volatile__("");
    // }
    // data_out_buffer[*buffer_out_write] = count;
    // *((volatile int*)buffer_out_write) = (*((volatile int*)buffer_out_write) + 1) % 50;
    current_node->result = count;
    // while(p_queue_out->next != NULL){
    //   p_queue_out = p_queue_out->next;
    // }
    ocall_print("Producing\n");
    produce(p_queue_out, current_node);
    // p_queue_out->next = p_queue_in;
    // p_queue_in = p_queue_in->next;
    // p_queue_in->next =
    iteration++;
    ocall_print("iterattion done\n");
//    ocall_print_int_message("Current iteration: %i\n", iteration);
  }
}
