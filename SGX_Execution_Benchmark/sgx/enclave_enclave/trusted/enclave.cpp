#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "enclave.h"
#include "enclave_t.h"  /* print_string */
#include "limits.h"

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_enclave_sample(buf);
}

int ecall_enclave_sample()
{
  printf("IN ENCLAVE\n");
  return 0;
}

void ecall_stress(unsigned long long in, unsigned long long in2){
    unsigned long long i;
    unsigned long long test = in;
    for(i=0; i<1000; i++){
        unsigned long long current = 0;
        test = i^in2;
        while(current<INT32_MAX){
            current+=in;
            test ^= current;
        }
        test ^= in2;
    }
}

