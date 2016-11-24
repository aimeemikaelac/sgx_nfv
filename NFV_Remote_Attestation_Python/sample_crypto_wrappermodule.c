#include <Python.h>
#include "sample_libcrypto.h"
#include "stdio.h"
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/objects.h>
#include "string.h"
#include "sgx_tcrypto.h"

/**
 * To build an executable:
 * gcc -I/usr/include/python2.7 -I$(pwd)/sample_libcrypto -L$(pwd)/sample_libcrypto -o test sample_crypto_wrappermodule.c -lcrypto -lsample_libcrypto -lpython2.7
 *
 * To build as a python module:
 * gcc -shared -fPIC -I/usr/include/python2.7 -I$(pwd)/sample_libcrypto -L$(pwd)/sample_libcrypto -o sample_crypto_wrappermodule.so sample_crypto_wrappermodule.c -lcrypto -lsample_libcrypto -lpython2.7
 */

#define EC_KEY_SIZE 32
//#define OPENSSL_CURVE NID_secp256k1
#define OPENSSL_CURVE "prime256v1"
//#define OPENSSL_CURVE "secp256r1"

static void _getOpenSSLKeys(unsigned char* ga_x, unsigned char* ga_y, unsigned char* ga_priv){
	int nid = OBJ_sn2nid(OPENSSL_CURVE);
	if(nid == NID_undef){
		printf("Finding curve NID failed");
		return;
	}
	EC_KEY *ssl_ec = EC_KEY_new_by_curve_name(nid);
	if(ssl_ec == NULL){
		printf("Error in ec_key_new\n");
	}
	EC_KEY_generate_key(ssl_ec);
	BIGNUM *ssl_priv = EC_KEY_get0_private_key(ssl_ec);
	EC_POINT *ssl_pub = EC_KEY_get0_public_key(ssl_ec);
	EC_GROUP *ssl_group = EC_KEY_get0_group(ssl_ec);
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();
	if (EC_POINT_get_affine_coordinates_GFp(ssl_group, ssl_pub, x, y, NULL)) {
		printf("SSL X:\n0x");
		BN_print_fp(stdout, x);
		printf("\nSSL Y:\n0x");
		BN_print_fp(stdout, y);
		printf("\n");
	}
	printf("SSL priv:\n0x");
	BN_print_fp(stdout, ssl_priv);
	printf("\n");

	memset(ga_x, 0, EC_KEY_SIZE);
	memset(ga_y, 0, EC_KEY_SIZE);
	memset(ga_priv, 0, EC_KEY_SIZE);
	//output BIGNUM to the char arrays. since these things should only be 32 bytes
	//in size, we can just check and throw an error if this not true
	if(BN_num_bytes(x) > EC_KEY_SIZE){
		printf("X coordinate has too many bytes");
	} else{
		BN_bn2bin(x, ga_x);
	}

	if(BN_num_bytes(y) > EC_KEY_SIZE){
		printf("Y coordinate has too many bytes");
	} else{
		BN_bn2bin(y, ga_y);
	}

	if(BN_num_bytes(ssl_priv) > EC_KEY_SIZE){
		printf("Private key has too many bytes");
	} else{
		BN_bn2bin(ssl_priv, ga_priv);
	}

}

static PyObject* _generateDHKeys_OpenSSL(){
	unsigned char ga_x[EC_KEY_SIZE], ga_y[EC_KEY_SIZE], ga_priv[EC_KEY_SIZE];
	_getOpenSSLKeys(ga_x, ga_y, ga_priv);
	return Py_BuildValue("[s#, s#, s#]", ga_x, 32, ga_y, 32, ga_priv, 32);
}

static PyObject* py_generateDHKeys(PyObject* self, PyObject* args){
//	sample_ecc_state_handle_t ecc_state = NULL;
//	sample_status_t sample_ret = sample_ecc256_open_context(&ecc_state);
////	if(SAMPLE_SUCCESS != sample_ret)
////	{
////		fprintf(stderr, "\nError, cannot get ECC context in [%s].",
////						 __FUNCTION__);
////		ret = -1;
////		break;
////	}
//	sample_ec256_public_t pub_key = {{0},{0}};
//	sample_ec256_private_t priv_key = {{0}};
//	sample_ret = sample_ecc256_create_key_pair(&priv_key, &pub_key, ecc_state);

//    if(SAMPLE_SUCCESS != sample_ret)
//    {
//        fprintf(stderr, "\nError, cannot generate key pair in [%s].",
//                __FUNCTION__);
//        ret = SP_INTERNAL_ERROR;
//        break;
//    }

//	EC_KEY *ssl_ec = EC_KEY_new_by_curve_name(NID_secp256k1);
//	if(ssl_ec == NULL){
//		printf("Error in ec_key_new\n");
//	}
//	EC_KEY_generate_key(ssl_ec);
//	BIGNUM *ssl_priv = EC_KEY_get0_private_key(ssl_ec);
//	EC_POINT *ssl_pub = EC_KEY_get0_public_key(ssl_ec);
//	EC_GROUP *ssl_group = EC_KEY_get0_group(ssl_ec);
//	BIGNUM *x = BN_new();
//	BIGNUM *y = BN_new();
//	if (EC_POINT_get_affine_coordinates_GFp(ssl_group, ssl_pub, x, y, NULL)) {
//		printf("SSL X:\n0x");
//		BN_print_fp(stdout, x);
//		printf("\nSSL Y:\n0x");
//		BN_print_fp(stdout, y);
//		printf("\n");
//	}
//	printf("SSL priv:\n0x");
//	BN_print_fp(stdout, ssl_priv);
//	printf("\n");

//    return Py_BuildValue("[s#, s#, s#]", pub_key.gx, 32, pub_key.gy, 32, priv_key.r, 32);
	return _generateDHKeys_OpenSSL();
}


static PyMethodDef sample_crypto_wrappermodule_methods[] = {
		{"generateDHKeys", py_generateDHKeys, METH_VARARGS},
		{NULL, NULL}
};

void initsample_crypto_wrappermodule(){
	(void) Py_InitModule("sample_crypto_wrappermodule", sample_crypto_wrappermodule_methods);
}

void main(){
	int i;

	unsigned char ga_x_1[EC_KEY_SIZE], ga_y_1[EC_KEY_SIZE], ga_priv_1[EC_KEY_SIZE],
				  ga_x_2[EC_KEY_SIZE], ga_y_2[EC_KEY_SIZE], ga_priv_2[EC_KEY_SIZE];

	sample_status_t sample_ret;

	sample_ecc_state_handle_t ecc_state;
	sample_ec256_public_t pub_key_1; //= {{0},{0}};
	sample_ec256_private_t priv_key_1; //= {{0}};
	sample_ec256_public_t pub_key_2; //= {{0},{0}};
	sample_ec256_private_t priv_key_2; //= {{0}};
	sample_ec256_dh_shared_t test_dh_key_1, test_dh_key_2;

//	for(i=0; i<1000; i++){
//		sample_ret = sample_ecc256_open_context(&ecc_state);
//		sample_ret = sample_ecc256_create_key_pair(&priv_key, &pub_key, ecc_state);
//		sample_ecc256_close_context(ecc_state);
//	}
	sample_ret = sample_ecc256_open_context(&ecc_state);
	sample_ret = sample_ecc256_create_key_pair(&priv_key_1, &pub_key_1, ecc_state);
	sample_ecc256_close_context(ecc_state);

	sample_ret = sample_ecc256_open_context(&ecc_state);
	sample_ret = sample_ecc256_create_key_pair(&priv_key_2, &pub_key_2, ecc_state);
	sample_ecc256_close_context(ecc_state);


//	sample_ec256_public_t pub_key = {{0},{0}};
//	sample_ec256_private_t priv_key = {{0}};
//	sample_ret = sample_ecc256_create_key_pair(&priv_key, &pub_key, ecc_state);
//	sample_ecc256_close_context(ecc_state);
	printf("Pub key x 1: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", pub_key_1.gx[i]);
	}
	printf("\n");
	printf("Pub key y 1: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", pub_key_1.gy[i]);
	}
	printf("\n");
	printf("Priv key r 1: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", priv_key_1.r[i]);
	}
	printf("\n\n");

	printf("Pub key x 2: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", pub_key_2.gx[i]);
	}
	printf("\n");
	printf("Pub key y 2: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", pub_key_2.gy[i]);
	}
	printf("\n");
	printf("Priv key r 2: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", priv_key_2.r[i]);
	}
	printf("\n\n");

	sample_ret = sample_ecc256_open_context(&ecc_state);
	if(sample_ret != SAMPLE_SUCCESS){
		printf("Failed to open sample ecc256 context\n");
	}
	sample_ret = sample_ecc256_compute_shared_dhkey(&priv_key_1, &pub_key_2, &test_dh_key_1, ecc_state);
	if(sample_ret != SAMPLE_SUCCESS){
		printf("Error getting shared test dh key 1 using priv_key_1 and pub_key_2. Error code: %i\n", sample_ret);
	}

	sample_ret = sample_ecc256_compute_shared_dhkey(&priv_key_2, &pub_key_1, &test_dh_key_2, ecc_state);
	if(sample_ret != SAMPLE_SUCCESS){
		printf("Error getting shared test dh key 2 using priv_key_2 and pub_key_1. Error code: %i\n", sample_ret);
	}

	printf("First test shared dh key:\n0x");
	for(i=0; i<SAMPLE_ECP256_KEY_SIZE; i++){
		printf("%02x", test_dh_key_1.s[i]);
	}
	printf("\nSecond test shared dh key:\n0x");
	for(i=0; i<SAMPLE_ECP256_KEY_SIZE; i++){
		printf("%02x", test_dh_key_2.s[i]);
	}
	printf("\n");
//	EC_KEY *ssl_ec = EC_KEY_new_by_curve_name(NID_secp256k1);
//	if(ssl_ec == NULL){
//		printf("Error in ec_key_new\n");
//	}
//	EC_KEY_generate_key(ssl_ec);
//	BIGNUM *ssl_priv = EC_KEY_get0_private_key(ssl_ec);
//	EC_POINT *ssl_pub = EC_KEY_get0_public_key(ssl_ec);
//	EC_GROUP *ssl_group = EC_KEY_get0_group(ssl_ec);
//	BIGNUM *x = BN_new();
//	BIGNUM *y = BN_new();
//	if (EC_POINT_get_affine_coordinates_GFp(ssl_group, ssl_pub, x, y, NULL)) {
//		printf("SSL X:\n0x");
//		BN_print_fp(stdout, x);
//		printf("\nSSL Y:\n0x");
//		BN_print_fp(stdout, y);
//		printf("\n");
//	}
//	printf("SSL priv:\n0x");
//	BN_print_fp(stdout, ssl_priv);
//	printf("\n");
//	_generateDHKeys_OpenSSL();

	printf("\nGet SGX Trusted Crypto Keys\n");
	sgx_ec256_public_t sgx_ga_1, sgx_ga_2;
	sgx_ec256_private_t sgx_priv_1, sgx_priv_2;

	sgx_ecc_state_handle_t sgx_ecc_state;
	sgx_status_t sgx_ret = sgx_ecc256_open_context(&sgx_ecc_state);
	sgx_ecc256_create_key_pair(&sgx_priv_1, &sgx_ga_1, sgx_ecc_state);
	sgx_ecc256_create_key_pair(&sgx_priv_2, &sgx_ga_2, sgx_ecc_state);

	printf("SGX Pub key x 1: \n0x");
	for(i=0; i<SGX_ECP256_KEY_SIZE; i++){
		printf("%02x", sgx_ga_1.gx[i]);
	}
	printf("\n");
	printf("SGX Pub key y 1: \n0x");
	for(i=0; i<SGX_ECP256_KEY_SIZE; i++){
		printf("%02x", sgx_ga_1.gy[i]);
	}
	printf("\n");
	printf("SGX Priv key r 1: \n0x");
	for(i=0; i<SGX_ECP256_KEY_SIZE; i++){
		printf("%02x", sgx_priv_1.r[i]);
	}
	printf("\n\n");

	printf("SGX Pub key x 2: \n0x");
	for(i=0; i<SGX_ECP256_KEY_SIZE; i++){
		printf("%02x", sgx_ga_2.gx[i]);
	}
	printf("\n");
	printf("SGX Pub key y 2: \n0x");
	for(i=0; i<SGX_ECP256_KEY_SIZE; i++){
		printf("%02x", sgx_ga_2.gy[i]);
	}
	printf("\n");
	printf("SGX Priv key r 2: \n0x");
	for(i=0; i<SGX_ECP256_KEY_SIZE; i++){
		printf("%02x", sgx_priv_2.r[i]);
	}
	printf("\n\n");



	printf("\nGet OpenSSL keys\n");
	sample_ret = sample_ecc256_open_context(&ecc_state);
	if(sample_ret != SAMPLE_SUCCESS){
		printf("Failed to open sample ecc256 context\n");
	}

	sample_ec256_public_t ga_1, ga_2;
	sample_ec256_private_t priv_1, priv_2;
	sample_ec256_dh_shared_t dh_key_1, dh_key_2;

	_getOpenSSLKeys(ga_x_1, ga_y_1, ga_priv_1);
	printf("\n");
	_getOpenSSLKeys(ga_x_2, ga_y_2, ga_priv_2);
	printf("\n");

	memcpy(ga_1.gx, ga_x_1, EC_KEY_SIZE);
	memcpy(ga_1.gy, ga_y_1, EC_KEY_SIZE);
	memcpy(ga_2.gx, ga_x_2, EC_KEY_SIZE);
	memcpy(ga_2.gy, ga_y_2, EC_KEY_SIZE);
	memcpy(priv_1.r, ga_priv_1, EC_KEY_SIZE);
	memcpy(priv_2.r, ga_priv_2, EC_KEY_SIZE);
	memset(&dh_key_1, 0, sizeof(sample_ec256_dh_shared_t));
	memset(&dh_key_2, 0, sizeof(sample_ec256_dh_shared_t));

//	sample_ret = sample_ecc256_compute_shared_dhkey(&priv_1, &ga_2, &dh_key_1, ecc_state);
//	if(sample_ret != SAMPLE_SUCCESS){
//		printf("Error getting shared dh key 1 using priv_1 and ga_2. Error code: %i\n", sample_ret);
//	}
//	sample_ret = sample_ecc256_compute_shared_dhkey(&priv_2, &ga_1, &dh_key_2, ecc_state);
//	if(sample_ret != SAMPLE_SUCCESS){
//		printf("Error getting shared dh key 2 using priv_2 and ga_1. Error code: %i\n", sample_ret);
//	}
	printf("\nCompute shared DH keys\n");
	sample_ret = sample_ecc256_compute_shared_dhkey(&priv_1, &pub_key_1, &dh_key_1, ecc_state);
	if(sample_ret != SAMPLE_SUCCESS){
		printf("Error getting shared dh key 1 using priv_1 and test sample pub_key_1. Error code: %i\n", sample_ret);
	}
	sample_ret = sample_ecc256_compute_shared_dhkey(&priv_key_1, &ga_1, &dh_key_2, ecc_state);
	if(sample_ret != SAMPLE_SUCCESS){
		printf("Error getting shared dh key 1 using test sample priv_key_1 and ga_1. Error code: %i\n", sample_ret);
	}

	printf("First shared dh key:\n0x");
	for(i=0; i<SAMPLE_ECP256_KEY_SIZE; i++){
		printf("%02x", dh_key_1.s[i]);
	}
	printf("\nSecond sharded dh key:\n0x");
	for(i=0; i<SAMPLE_ECP256_KEY_SIZE; i++){
		printf("%02x", dh_key_2.s[i]);
	}
	printf("\n");

}
