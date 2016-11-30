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

int ecall_process_packet(unsigned char* data, unsigned int length){
    ocall_print("In packet processing - enclave\n");
    int i, count = 0;
    for(i=0; i<length; i++){
       if(data[i] == 0x0a){
           count++;
       }
    }
    ocall_print("Detected ");
    ocall_print_int(count);
    ocall_print(" occurrences of the byte 0a\n");
    return count;
}
