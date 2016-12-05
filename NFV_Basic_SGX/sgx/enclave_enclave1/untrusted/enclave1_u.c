#include "enclave1_u.h"
#include <errno.h>

typedef struct ms_ecall_process_packet_t {
	unsigned char* ms_packet_data;
	unsigned int ms_data_len;
	unsigned char* ms_hash_out;
} ms_ecall_process_packet_t;

typedef struct ms_ocall_print_t {
	char* ms_str;
} ms_ocall_print_t;

typedef struct ms_ocall_print_hexbyte_t {
	unsigned char ms_byteval;
} ms_ocall_print_hexbyte_t;

typedef struct ms_ocall_print_int_t {
	int ms_val;
} ms_ocall_print_int_t;

static sgx_status_t SGX_CDECL enclave1_ocall_print(void* pms)
{
	ms_ocall_print_t* ms = SGX_CAST(ms_ocall_print_t*, pms);
	ocall_print((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave1_ocall_print_hexbyte(void* pms)
{
	ms_ocall_print_hexbyte_t* ms = SGX_CAST(ms_ocall_print_hexbyte_t*, pms);
	ocall_print_hexbyte(ms->ms_byteval);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave1_ocall_print_int(void* pms)
{
	ms_ocall_print_int_t* ms = SGX_CAST(ms_ocall_print_int_t*, pms);
	ocall_print_int(ms->ms_val);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[3];
} ocall_table_enclave1 = {
	3,
	{
		(void*)enclave1_ocall_print,
		(void*)enclave1_ocall_print_hexbyte,
		(void*)enclave1_ocall_print_int,
	}
};
sgx_status_t ecall_process_packet(sgx_enclave_id_t eid, unsigned char* packet_data, unsigned int data_len, unsigned char* hash_out)
{
	sgx_status_t status;
	ms_ecall_process_packet_t ms;
	ms.ms_packet_data = packet_data;
	ms.ms_data_len = data_len;
	ms.ms_hash_out = hash_out;
	status = sgx_ecall(eid, 0, &ocall_table_enclave1, &ms);
	return status;
}

