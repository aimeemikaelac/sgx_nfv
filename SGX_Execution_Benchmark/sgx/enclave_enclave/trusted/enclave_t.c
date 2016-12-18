#include "enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
#include <string.h> /* for memcpy etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)


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

static sgx_status_t SGX_CDECL sgx_ecall_enclave_sample(void* pms)
{
	ms_ecall_enclave_sample_t* ms = SGX_CAST(ms_ecall_enclave_sample_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_enclave_sample_t));

	ms->ms_retval = ecall_enclave_sample();


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_stress(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	ecall_stress();
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_stress_memory(void* pms)
{
	ms_ecall_stress_memory_t* ms = SGX_CAST(ms_ecall_stress_memory_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	unsigned char* _tmp_data = ms->ms_data;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_stress_memory_t));

	ecall_stress_memory(_tmp_data, ms->ms_length);


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_sha(void* pms)
{
	ms_ecall_sha_t* ms = SGX_CAST(ms_ecall_sha_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	unsigned char* _tmp_data = ms->ms_data;
	unsigned char* _tmp_hash_out = ms->ms_hash_out;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_sha_t));

	ecall_sha(_tmp_data, ms->ms_length, _tmp_hash_out);


	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv;} ecall_table[4];
} g_ecall_table = {
	4,
	{
		{(void*)(uintptr_t)sgx_ecall_enclave_sample, 0},
		{(void*)(uintptr_t)sgx_ecall_stress, 0},
		{(void*)(uintptr_t)sgx_ecall_stress_memory, 0},
		{(void*)(uintptr_t)sgx_ecall_sha, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[2][4];
} g_dyn_entry_table = {
	2,
	{
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_enclave_sample(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_enclave_sample_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_enclave_sample_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_enclave_sample_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_enclave_sample_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(0, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(1, ms);


	sgx_ocfree();
	return status;
}

