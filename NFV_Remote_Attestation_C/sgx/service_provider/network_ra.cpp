/*
 * Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "network_ra.h"
#include "service_provider.h"

int port = 8000;

//server- either the IP address or domain of the server
int send_request(const char *server,
                 int server_port,
                 const ra_samp_request_header_t *request,
                 ra_samp_response_header_t *response){
    struct sockaddr_in *server_socket;
    struct addrinfo hints;
    struct addrinfo *serverinfo, *iter;
    unsigned char *transmit_buffer;
    unsigned char *recv_buffer = NULL;
    int buffer_size, bytes_received, received_size = 0, buffer_offset = 0;
    char buff_port[100];

    int rc, bytes_received, socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        printf("Could not get a client socket\n");
        return -1;
    }

    rc = sprintf(buff_port, "%i", server_port);
    if(rc > 99){
        printf("Buffer overflow in sprintf of port #. There were %i characters written. Abort.\n", rc);
        return -1;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_family = SOCK_STREAM;
    if((rc = getaddrinfo(server, buff_port, &hints, &serverinfo)) != 0){
        printf("getaddrinfo failed with error: %i", rc);
        return -1;
    }

    iter = serverinfo;
    while(iter != NULL && iter->ai_family != AF_INET){
        iter = iter->ai_next;
    }

    if(serverinfo == NULL){
        printf("getaddrinfo found no results\n");
        return -1;
    }

    server_socket = (struct sockaddr_in*)iter->ai_addr;

    rc = connect(socket_fd, (struct sockaddr*)server_socket, sizeof(struct sockaddr_in));
    if(rc < 0){
        printf("Error connecting to socket\n");
        return -1;
    }

    transmit_buffer = (unsigned char*)request;
    buffer_size = sizeof(ra_samp_request_header_t) + request->size;

    rc = send(socket_fd, transmit_buffer, buffer_size, 0);
    if(rc < 0){
        printf("Send failed\n");
        return -1;
    }

    do{
        received_size += 1000;
        recv_buffer = (unsigned char*)realloc(recv_buffer, buffer_size);
        if(recv_buffer == NULL){
            printf("Alloc failed\n");
            return -1;
        }
        bytes_received = recv(socket_fd, recv_buffer + buffer_offset, 1000, 0);
        if(bytes_received < 0){
            printf("Error when receiving\n");
            return -1;
        }

        buffer_offset += 1000;

        ra_samp_response_header_t current_response = (ra_samp_response_header_t)recv_buffer;

        int current_response_size = current_response.size + sizeof(ra_samp_response_header_t);
        if(received_size >= current_response_size){
            break;
        }
    } while(bytes_received > 0);

    response = (ra_samp_request_header_t)recv_buffer;
    return 0;
}


// Used to send requests to the service provider sample.  It
// simulates network communication between the ISV app and the
// ISV service provider.  This would be modified in a real
// product to use the proper IP communication.
//
// @param server_url String name of the server URL
// @param p_req Pointer to the message to be sent.
// @param p_resp Pointer to a pointer of the response message.

// @return int

int ra_network_send_receive(const char *server_url,
    const ra_samp_request_header_t *p_req,
    ra_samp_response_header_t **p_resp)
{
    int ret = 0;
    ra_samp_response_header_t* p_resp_msg;

    if((NULL == server_url) ||
        (NULL == p_req) ||
        (NULL == p_resp))
    {
        return -1;
    }
/*
    switch(p_req->type)
    {

    case TYPE_RA_MSG0:
        ret = sp_ra_proc_msg0_req((const sample_ra_msg0_t*)((uint8_t*)p_req
            + sizeof(ra_samp_request_header_t)),
            p_req->size);
        if (0 != ret)
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg1_req fail [%s].",
                __FUNCTION__);
        }
        break;

    case TYPE_RA_MSG1:
        ret = sp_ra_proc_msg1_req((const sample_ra_msg1_t*)((uint8_t*)p_req
            + sizeof(ra_samp_request_header_t)),
            p_req->size,
            &p_resp_msg);
        if(0 != ret)
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg1_req fail [%s].",
                __FUNCTION__);
        }
        else
        {
            *p_resp = p_resp_msg;
        }
        break;

    case TYPE_RA_MSG3:
        ret =sp_ra_proc_msg3_req((const sample_ra_msg3_t*)((uint8_t*)p_req +
            sizeof(ra_samp_request_header_t)),
            p_req->size,
            &p_resp_msg);
        if(0 != ret)
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg3_req fail [%s].",
                __FUNCTION__);
        }
        else
        {
            *p_resp = p_resp_msg;
        }
        break;

    default:
        ret = -1;
        fprintf(stderr, "\nError, unknown ra message type. Type = %d [%s].",
            p_req->type, __FUNCTION__);
        break;
    }
*/
    ret = send_request(server_url, port, p_req, p_resp_msg);
    *p_resp = p_resp_msg;
    return ret;
}

// Used to free the response messages.  In the sample code, the
// response messages are allocated by the SP code.
//
//
// @param resp Pointer to the response buffer to be freed.

void ra_free_network_response_buffer(ra_samp_response_header_t *resp)
{
    if(resp!=NULL)
    {
        free(resp);
    }
}
