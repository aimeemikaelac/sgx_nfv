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
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print, (const char* str));

sgx_status_t ecall_enclave_sample(sgx_enclave_id_t eid, int* retval);
sgx_status_t ecall_stress(sgx_enclave_id_t eid);
sgx_status_t ecall_stress_memory(sgx_enclave_id_t eid, unsigned char* data, int length);
sgx_status_t ecall_sha(sgx_enclave_id_t eid, unsigned char* data, int length, unsigned char* hash_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
