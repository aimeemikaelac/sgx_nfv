/*
 * App.h
 *
 *  Created on: Nov 28, 2016
 *      Author: michael
 */

#ifndef ENCLAVE_ENCLAVE1_APP_APP_H_
#define ENCLAVE_ENCLAVE1_APP_APP_H_

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <pwd.h>

#include "sgx_urts.h"
#include "App.h"
#include "enclave1_u.h"

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif


# define TOKEN_FILENAME   "enclave.token"
# define ENCLAVE1_FILENAME "enclave1.signed.so"

sgx_enclave_id_t global_eid = 0;

#endif /* ENCLAVE_ENCLAVE1_APP_APP_H_ */
