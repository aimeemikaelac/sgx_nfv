#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_satus_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_enclave_sample, (const char* str));

sgx_status_t ecall_enclave_sample(sgx_enclave_id_t eid, int* retval);
sgx_status_t ecall_stress(sgx_enclave_id_t eid, unsigned long long in, unsigned long long in2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
