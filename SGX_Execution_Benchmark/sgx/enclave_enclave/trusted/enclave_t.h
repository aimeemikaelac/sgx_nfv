#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif


int ecall_enclave_sample();
void ecall_stress(unsigned long long in, unsigned long long in2);

sgx_status_t SGX_CDECL ocall_enclave_sample(const char* str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
