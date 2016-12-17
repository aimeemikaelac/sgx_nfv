#include "enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_enclave_sample_t {
	int ms_retval;
} ms_ecall_enclave_sample_t;

typedef struct ms_ecall_stress_t {
	unsigned long long ms_in;
	unsigned long long ms_in2;
} ms_ecall_stress_t;

typedef struct ms_ocall_enclave_sample_t {
	char* ms_str;
} ms_ocall_enclave_sample_t;

static sgx_status_t SGX_CDECL enclave_ocall_enclave_sample(void* pms)
{
	ms_ocall_enclave_sample_t* ms = SGX_CAST(ms_ocall_enclave_sample_t*, pms);
	ocall_enclave_sample((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_enclave = {
	1,
	{
		(void*)enclave_ocall_enclave_sample,
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

sgx_status_t ecall_stress(sgx_enclave_id_t eid, unsigned long long in, unsigned long long in2)
{
	sgx_status_t status;
	ms_ecall_stress_t ms;
	ms.ms_in = in;
	ms.ms_in2 = in2;
	status = sgx_ecall(eid, 1, &ocall_table_enclave, &ms);
	return status;
}

