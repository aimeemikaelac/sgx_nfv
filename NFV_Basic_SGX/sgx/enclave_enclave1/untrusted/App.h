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

#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "stdlib.h"

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif
/*
 To build an example that is linked against the library:
 g++ -Ienclave_enclave1/untrusted -I/opt/intel/sgxsdk/include/ -Lenclave_enclave1/ -L/opt/intel/sgxsdk/lib64 test.cpp -o test.o -lapp -lsgx_urts_sim -lsgx_uae_service_sim -lpthread
 */


# define TOKEN_FILENAME   "enclave.token"
# define ENCLAVE1_FILENAME "enclave1.signed.so"

void run_server();
void handle_connection(int socket_fd);
int initialize_enclave();

#endif /* ENCLAVE_ENCLAVE1_APP_APP_H_ */
