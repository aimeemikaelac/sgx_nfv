#include "enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_enclave_sample_t {
	int ms_retval;
} ms_ecall_enclave_sample_t;


typedef struct ms_ecall_stress_memory_t {
	unsigned char* ms_data;
	int ms_length;
} ms_ecall_stress_memory_t;

typedef struct ms_ecall_sha_t {
	unsigned char* ms_data;
	int ms_length;
	unsigned char* ms_hash_out;
} ms_ecall_sha_t;

typedef struct ms_ocall_enclave_sample_t {
	char* ms_str;
} ms_ocall_enclave_sample_t;

typedef struct ms_ocall_print_t {
	char* ms_str;
} ms_ocall_print_t;

static sgx_status_t SGX_CDECL enclave_ocall_enclave_sample(void* pms)
{
	ms_ocall_enclave_sample_t* ms = SGX_CAST(ms_ocall_enclave_sample_t*, pms);
	ocall_enclave_sample((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_print(void* pms)
{
	ms_ocall_print_t* ms = SGX_CAST(ms_ocall_print_t*, pms);
	ocall_print((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[2];
} ocall_table_enclave = {
	2,
	{
		(void*)enclave_ocall_enclave_sample,
		(void*)enclave_ocall_print,
	}
};
sgx_status_t ecall_enclave_sample(sgx_enclave_id_t eid, int* retval)
{
	sgx_status_t status;
	ms_ecall_enclave_sample_t ms;
	status = sgx_ecall(eid, 0, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_stress(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 1, &ocall_table_enclave, NULL);
	return status;
}

sgx_status_t ecall_stress_memory(sgx_enclave_id_t eid, unsigned char* data, int length)
{
	sgx_status_t status;
	ms_ecall_stress_memory_t ms;
	ms.ms_data = data;
	ms.ms_length = length;
	status = sgx_ecall(eid, 2, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t ecall_sha(sgx_enclave_id_t eid, unsigned char* data, int length, unsigned char* hash_out)
{
	sgx_status_t status;
	ms_ecall_sha_t ms;
	ms.ms_data = data;
	ms.ms_length = length;
	ms.ms_hash_out = hash_out;
	status = sgx_ecall(eid, 3, &ocall_table_enclave, &ms);
	return status;
}

