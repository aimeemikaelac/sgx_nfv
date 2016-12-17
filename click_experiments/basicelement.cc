#include <click/config.h>
#include "basicelement.hh"
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include "string.h"
#include "stdlib.h"
#include <unistd.h>
#include "App.h"
CLICK_DECLS

BasicElement *global_ref = NULL;
pthread_mutex_t callback_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct click_packet_queue{
  Packet *p;
  long int packet_id;
  struct click_packet_queue *next;
} click_packet_queue;


click_packet_queue click_packet_queue_head;

using namespace std;

static void enclave_callback(long int result, long int packet_id){
  pthread_mutex_lock(&callback_mutex);
  printf("In cs2\n");
  click_packet_queue *head_copy = &click_packet_queue_head;
  click_packet_queue *next = head_copy->next;
  click_packet_queue *prev = head_copy;
  while(next != NULL){
    if(next->packet_id == packet_id){
      break;
    }
    prev = next;
    next = next->next;
  }
  pthread_mutex_unlock(&callback_mutex);
  printf("Out of cs2\n");
  if(next == NULL){
    printf("Could not find packet id %li in click packet queue\n", packet_id);
  } else{
    printf("Result for packet id %li: %li\n", result);
    pthread_mutex_lock(&callback_mutex);
    prev->next = next->next;
    pthread_mutex_unlock(&callback_mutex);
    global_ref->output(0).push(next->p);
    free(next);
  }
  printf("Done with callback\n");
}

BasicElement::BasicElement(){
  printf("Started constructor\n");
  int ret = initialize_enclave();
  if(ret < 0){
    printf("Initializing enclave failed\n");
    exit(-1);
  }
  ret = initialize_enclave_threads(enclave_callback);
  if(ret < 0){
    printf("Initializing enclave thread failed\n");
    exit(-1);
  }
  click_packet_queue_head.next = NULL;
  global_ref = this;
  printf("Finished constructor\n");
}

String BasicElement::sgx_read_handler(Element *e, void *thunk){
  BasicElement *sgx = (BasicElement*)e;
  return String(sgx->sgx_sum);
}

void BasicElement::add_handlers(){
  add_read_handler("sgx_sum", sgx_read_handler, 0);
}

void BasicElement::push(int port, Packet *p){
  printf("In push\n");
  count++;
//  cout << "Received packet. Updating count to: "<<count<<endl;
  // unsigned char *data = (unsigned char*)p->data();
  // unsigned int data_len = p->length();
  // int sgx_count = call_process_packet_sgx(data, data_len);
  // cout << "Count: " << sgx_count << endl;
//  printf("Count: %i\n", sgx_count);
  // sgx_sum += sgx_count;

//  long int response_count = sendData(data, data_len);
/*  if(response_len != data_len){
	p->kill();
  } else{
  	output(0).push(p);
  }*/
  //cout << "Count received from processor of 0x0A bytes was: "<<response_count << endl;
  // output(0).push(p);
  click_packet_queue *new_item = (click_packet_queue*)malloc(sizeof(click_packet_queue));
  new_item->p = p;
  new_item->packet_id = packet_id;
  pthread_mutex_lock(&callback_mutex);
  printf("In cs\n");
  new_item->next = click_packet_queue_head.next;
  printf("In cs2\n");
  click_packet_queue_head.next = new_item;
  printf("In cs3\n");
  pthread_mutex_unlock(&callback_mutex);
  printf("Out of cs\n");
  call_process_packet_sgx((unsigned char*)p->data(), p->length(), packet_id);
  ++packet_id;
  printf("out of push\n");
}

/**
  * Send packet data over a socket and receive a response.
  * Response follows the protocol:
  * OK:<int> - The characters "OK:" folowed by an int representing
  *   the amount of data originally sent. If negative, considered an error
  * ERROR - The characters "ERROR" indicating an error occurred.
  *
  * Returns:
  *   <long int> the response from the client or a negative number indicating
  *   an error code
 **/
long int BasicElement::sendData(unsigned char *data, unsigned int length){
	struct sockaddr_in *server_socket;
    struct addrinfo hints;
	struct addrinfo *serverinfo, *iter;
	unsigned char recv_buff[100];
	unsigned char header[100];
	sprintf((char*)header, "data_len:%i|data:", length);
	unsigned int header_len = strlen((char*)header);
	int recv_bytes = 0;
	long int received_count = -3;

	int rc;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
        printf("Could not get a client socket\n");
        return -1;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
//	hints.ai_family = AF_UNSPEC;
    hints.ai_family = SOCK_STREAM;
    hints.ai_family = AF_INET;
    if((rc = getaddrinfo("localhost", "10000", &hints, &serverinfo)) != 0){
        printf("getaddrinfo failed with error: %i", rc);
        close(socket_fd);
        return -1;
    }

	iter = serverinfo;
    while(iter != NULL && iter->ai_family != AF_INET){
        iter = iter->ai_next;
    }

    if(serverinfo == NULL){
        printf("getaddrinfo found no results\n");
        freeaddrinfo(serverinfo);
        close(socket_fd);
        return -1;
    }

    server_socket = (struct sockaddr_in*)iter->ai_addr;

	rc = connect(socket_fd, (struct sockaddr*)server_socket, sizeof(struct sockaddr_in));
    if(rc < 0){
        printf("Error connecting to socket\n");
        freeaddrinfo(serverinfo);
        close(socket_fd);
        return -1;
    }

	rc = send(socket_fd, header, header_len, 0);
	if(rc < 0){
		printf("Failed to send the header\n");
        freeaddrinfo(serverinfo);
        close(socket_fd);
		return -1;
	}

	rc = send(socket_fd, data, length, 0);
    if(rc < 0){
        printf("Failed to send data\n");
        freeaddrinfo(serverinfo);
        close(socket_fd);
        return -1;
    }

	do{
		//TODO: add timeout
		recv_bytes = recv(socket_fd, recv_buff, 100, 0);
		if(recv_bytes < 0){
			printf("Error receiving\n");
            freeaddrinfo(serverinfo);
            close(socket_fd);
			return -1;
		}
		recv_buff[99] = 0;
		if(strncmp((char*)recv_buff, "OK:", 3) == 0){
			received_count = strtol((char*)recv_buff + 3, NULL, 0);
			if(received_count < 0){
				printf("Received negative count\n");
                freeaddrinfo(serverinfo);
                close(socket_fd);
				return -2;
			} else{
                freeaddrinfo(serverinfo);
                close(socket_fd);
				return received_count;
			}
		} else if(strncmp((char*)recv_buff, "ERROR", 5) == 0){
			printf("Received error\n");
            freeaddrinfo(serverinfo);
            close(socket_fd);
			return -2;
		} else{
			printf("Received malformed data\n");
            freeaddrinfo(serverinfo);
            close(socket_fd);
			return -2;
		}

	} while(recv_bytes > 0);
	//unreachable?
    freeaddrinfo(serverinfo);
    close(socket_fd);
	return -4;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BasicElement)
