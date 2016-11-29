#include "stdarg.h"
#include "stdio.h"     /* vsnprintf */

#include "enclave1.h"
#include "enclave1_t.h"  /* print_string */
#include "sgx_tcrypto.h"

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

void ecall_process_packet(unsigned char* data, unsigned int length){
//void ecall_process_packet(void){
    ocall_print("In enclave\n");
    int data_length = length;
    int i;
    ocall_print("Received data:\n0x");
    for(i=0; i<data_length; i++){
        ocall_print_hexbyte(data[i]);
    }
    ocall_print("\n");
}
