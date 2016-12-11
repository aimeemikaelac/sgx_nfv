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

#include "sgx_tcrypto.h"

#include "stdlib.h"
#include "math.h"

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif
/*
 To build an example that is linked against the library:
 g++ -Ienclave_enclave1/untrusted -I/opt/intel/sgxsdk/include/ -Lenclave_enclave1/ -L/opt/intel/sgxsdk/lib64 test.cpp -o test.o -lapp -lsgx_urts_sim -lsgx_uae_service_sim -lpthread

 env. variables to build with click:
 export CPPFLAGS="-I/opt/intel/sgxsdk/include/ -I/home/michael/sgx_nfv/NFV_Basic_SGX/sgx/enclave_enclave1/untrusted/"
 export LDFLAGS="-L/home/michael/sgx_nfv/NFV_Basic_SGX/sgx/enclave_enclave1/ -L/opt/intel/sgxsdk/lib64"
 export LIBS="-lapp -lsgx_urts_sim -lsgx_uae_service_sim -lpthread -lcrypto"

 need to set these variables in the shell and then run ./configure again

 if we are using hardware-mode sgx, need to change the variables to:
 export LIBS="-lapp -lsgx_urts -lsgx_uae_service -lpthread -lcrypto"
 */


# define TOKEN_FILENAME   "enclave.token"
# define ENCLAVE1_FILENAME "enclave1.signed.so"

void run_server();
void handle_connection(int socket_fd);
int initialize_enclave();
void call_process_packet_sgx(unsigned char *data, unsigned int length);
void call_process_packet_sgx_sha256(unsigned char *data, unsigned int length);

#endif /* ENCLAVE_ENCLAVE1_APP_APP_H_ */
