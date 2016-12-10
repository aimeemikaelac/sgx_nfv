#include "stdarg.h"
#include "stdio.h"     /* vsnprintf */

#include "enclave1.h"
#include "enclave1_t.h"  /* print_string */
#include "sgx_tcrypto.h"
#include "string.h"

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

void ecall_process_packet(unsigned char* data, unsigned int length, unsigned char hash_out[SGX_SHA256_HASH_SIZE]){
//    ocall_print("In packet processing - enclave\n");
    sgx_status_t status;
    sgx_sha256_hash_t hash;
    status = sgx_sha256_msg(data, length, &hash);
    if(status != SGX_SUCCESS){
        ocall_print("Error calculating SHA256 message\n");
        return;
    }
    memcpy(hash_out, &hash, SGX_SHA256_HASH_SIZE);

//    int i, count = 0;
//    for(i=0; i<length; i++){
//       if(data[i] == 0x0a){
//           count++;
//       }
//    }
//    ocall_print("Detected ");
//    ocall_print_int(count);
//    ocall_print(" occurrences of the byte 0a\n");
//    return count;
}
