#include <Python.h>
#include "sample_libcrypto.h"
#include "stdio.h"
#include <openssl/obj_mac.h>
#include <openssl/ec.h>

/**
 * To build an executable:
 * gcc -I/usr/include/python2.7 -I$(pwd)/sample_libcrypto -L$(pwd)/sample_libcrypto -o test sample_crypto_wrappermodule.c -lcrypto -lsample_libcrypto -lpython2.7
 *
 * To build as a python module:
 * gcc -shared -fPIC -I/usr/include/python2.7 -I$(pwd)/sample_libcrypto -L$(pwd)/sample_libcrypto -o test sample_crypto_wrappermodule.c -lcrypto -lsample_libcrypto -lpython2.7
 */

static PyObject* py_generateDHKeys(PyObject* self, PyObject* args){
	sample_ecc_state_handle_t ecc_state = NULL;
	sample_status_t sample_ret = sample_ecc256_open_context(&ecc_state);
//	if(SAMPLE_SUCCESS != sample_ret)
//	{
//		fprintf(stderr, "\nError, cannot get ECC context in [%s].",
//						 __FUNCTION__);
//		ret = -1;
//		break;
//	}
	sample_ec256_public_t pub_key = {{0},{0}};
	sample_ec256_private_t priv_key = {{0}};
	sample_ret = sample_ecc256_create_key_pair(&priv_key, &pub_key, ecc_state);

//    if(SAMPLE_SUCCESS != sample_ret)
//    {
//        fprintf(stderr, "\nError, cannot generate key pair in [%s].",
//                __FUNCTION__);
//        ret = SP_INTERNAL_ERROR;
//        break;
//    }

    return Py_BuildValue("[s#, s#, s#]", pub_key.gx, 32, pub_key.gy, 32, priv_key.r, 32);
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
	sample_ecc_state_handle_t ecc_state;
	sample_ec256_public_t pub_key; //= {{0},{0}};
	sample_ec256_private_t priv_key; //= {{0}};

	for(i=0; i<1000; i++){
		sample_status_t sample_ret = sample_ecc256_open_context(&ecc_state);
		sample_ret = sample_ecc256_create_key_pair(&priv_key, &pub_key, ecc_state);
		sample_ecc256_close_context(ecc_state);
	}

//	sample_ret = sample_ecc256_open_context(&ecc_state);
//	sample_ec256_public_t pub_key = {{0},{0}};
//	sample_ec256_private_t priv_key = {{0}};
//	sample_ret = sample_ecc256_create_key_pair(&priv_key, &pub_key, ecc_state);
//	sample_ecc256_close_context(ecc_state);
	printf("Pub key x: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", pub_key.gx[i]);
	}
	printf("\n");
	printf("Pub key y: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", pub_key.gy[i]);
	}
	printf("\n");
	printf("Priv key r: \n0x");
	for(i=0; i<32; i++){
		printf("%02x", priv_key.r[i]);
	}
	printf("\n\n");

	EC_KEY *ssl_ec = EC_KEY_new_by_curve_name(NID_secp256k1);
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
}
