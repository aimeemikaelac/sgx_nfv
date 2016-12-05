#ifndef ENCLAVE1_U_H__
#define ENCLAVE1_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_satus_t etc. */

#include "sgx_trts.h"
#include "sgx_tcrypto.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print, (const char* str));
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_hexbyte, (unsigned char byteval));
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_int, (int val));

sgx_status_t ecall_process_packet(sgx_enclave_id_t eid, unsigned char* packet_data, unsigned int data_len, unsigned char* hash_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
