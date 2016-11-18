#include <stdio.h>
#include <iostream>
#include "Enclave1_u.h"
#include "sgx_urts.h"
#include "sgx_ukey_exchange.h"
#include "sgx_uae_service.h"
#include "network_ra.h"
#include "remote_attestation_result.h"

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

char enclave_name[] = "Enclave1.signed.so";

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    std::cout << ret << std::endl;
}

/* Initialize the enclave:
 *   Step 1: try to retrieve the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */
int initialize_enclave() {
    char* token_path = "launch_token_file";
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    /* Step 1: try to retrieve the launch token saved by last transaction
     *         if there is no token, then create a new one.
     */
    /* try to get the token saved in $HOME */
    FILE* fp = fopen(token_path, "rb");
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }

    if (fp != NULL) {
        /* read the token from saved file */
        size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
    /* Step 2: call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(enclave_name, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        if (fp != NULL) fclose(fp);
        return -1;
    }

    /* Step 3: save the launch token if it is updated */
    if (updated == FALSE || fp == NULL) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (fp != NULL) fclose(fp);
        return 0;
    }

    /* reopen the file with write capablity */
    fp = freopen(token_path, "wb", fp);
    if (fp == NULL) return 0;
    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
    if (write_num != sizeof(sgx_launch_token_t))
        printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
    fclose(fp);
    return 0;
}

// OCall implementations
void ocall_print(const char* str) {
    printf("%s", str);
}

void ocall_print_hexbyte(unsigned char byteval){
	printf("%02x", byteval);
}

int perform_remote_attestation(const char* server_url){
	sgx_ra_context_t context;
	sgx_status_t status = SGX_SUCCESS;
	int ret;
	ra_samp_request_header_t *p_msg0_full = NULL;
	ra_samp_response_header_t *p_msg0_resp_full = NULL;
	ra_samp_request_header_t *p_msg1_full = NULL;
	ra_samp_response_header_t *p_msg2_full = NULL;
	ra_samp_response_header_t *p_att_result_msg_full = NULL;
	sgx_ra_msg3_t *p_msg3 = NULL;
    uint32_t extended_epid_group_id = 0;
    uint32_t msg3_size = 0;

	//1. init ra in enclave and get back context
	ret = enclave_init_ra(global_eid, &status, false, &context);
	if(ret != 0){
		printf("Enclave init returned %i\n", ret);
		return -1;
	} else if(status != SGX_SUCCESS){
		printf("Enclave init had an unsuccessful status returned\n");
		return -1;
	}

	//2. get epid group id
    ret = sgx_get_extended_epid_group_id(&extended_epid_group_id);
    if(ret != 0){
    	printf("Get EPID group failed\n");
    	return -1;
    }

	//3. construct msg0, send over socket to remote verifier
    p_msg0_full = (ra_samp_request_header_t*)malloc(sizeof(ra_samp_request_header_t) + sizeof(uint32_t));
    if(p_msg0_full == NULL){
    	printf("Could not allocate msg0\n");
    	return -1;
    }
    p_msg0_full->type = TYPE_RA_MSG0;
    p_msg0_full->size = sizeof(uint32_t);
    memcpy(p_msg0_full->body, &extended_epid_group_id, sizeof(uint32_t));

    printf("Sending msg0\n");
    ret = ra_network_send_receive(server_url, p_msg0_full, &p_msg0_resp_full);
    if(ret != 0){
        printf("Sending message 1 failed");
        return -1;
    }

	//4. get msg1 from enclave, send to verifier
    //TODO: send message over network to verifier
    //TODO: send msg1 to remote verifier
    p_msg1_full = (ra_samp_request_header_t*)malloc(sizeof(ra_samp_request_header_t) + sizeof(sgx_ra_msg1_t));
    if(p_msg1_full == NULL){
		printf("Could not allocate msg1\n");
		return -1;
	}
    p_msg1_full->type = TYPE_RA_MSG1;
	p_msg1_full->size = sizeof(sgx_ra_msg1_t);
	//TODO: may need to add the sleep/retry code
	ret = sgx_ra_get_msg1(context, global_eid, sgx_ra_get_ga, (sgx_ra_msg1_t*)p_msg1_full->body);
	if(ret != 0){
		printf("Failed to get msg1 from key exchange library\n");
		return -1;
	}

	//5. get msg2 from verifier, send to enclave to process
	//TODO: add retry code
	ret = sgx_ra_proc_msg2(context, global_eid, sgx_ra_proc_msg2_trusted, sgx_ra_get_msg3_trusted, (sgx_ra_msg2_t*)p_msg2_full->body, p_msg2_full->size, &p_msg3, &msg3_size);
	if(ret != 0 || msg3_size != 0){
		printf("sgx_ra_proc_msg2 failed\n");
	}
	//6. get msg3 from enclave, send to verifier to process
	//TODO: take the generated msg3 and send to remote verifier
	//7. get final result from enclave
	//TODO: receive the attestation result back from the remote verifier
	//TODO: need to check the CMAC of the returned message
	sample_ra_att_result_msg_t *p_att_result_msg_body = (sample_ra_att_result_msg_t*)p_att_result_msg_full->body;
	ret = verify_att_result_mac(global_eid, &status, context,
	                            (uint8_t*)&(p_att_result_msg_body->platform_info_blob), sizeof(ias_platform_info_blob_t),
	                            (uint8_t*)&(p_att_result_msg_body->mac), sizeof(sgx_mac_t));
	if(ret != 0 || status != SGX_SUCCESS){
	    printf("Verification of MAC failed");
	    return -1;
	}

	//If the attestation passed, extract the shared secret from the final message
	bool attestation_passed = true;
	if(0 != p_att_result_msg_full->status[0] || 0 != p_att_result_msg_full->status[1]){
        printf("Attestation failed\n");
        attestation_passed = false;
        return -1;
    }

	if(attestation_passed){
	    ret = put_secret_data(global_eid,
                              &status,
                              context,
                              p_att_result_msg_body->secret.payload,
                              p_att_result_msg_body->secret.payload_size,
                              p_att_result_msg_body->secret.payload_tag);
        if(ret != 0  || status != SGX_SUCCESS){
            printf("Failed to extract shared secret\n");
            return -1;
        }
}
	return 0;
}

int main(int argc, char const *argv[]) {
    if (initialize_enclave() < 0) {
        std::cout << "SGX error" << std::endl;
    }
    int ptr;
    sgx_status_t status = generate_random_number(global_eid, &ptr);
    std::cout << status << std::endl;
    if (status != SGX_SUCCESS) {
        std::cout << "SGX error" << std::endl;
    }
    printf("Random number: %d\n", ptr);

    status = generate_key_pair(global_eid, &ptr);
    std::cout << status << std::endl;
	if (status != SGX_SUCCESS) {
		std::cout << "SGX error" << std::endl;
	}
    return 0;
}
