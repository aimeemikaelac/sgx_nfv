#include <click/config.h>
#include "basicelement.hh"
#include <iostream>
#include <sys/socket.h>
#include "string.h"
#include "stdlib.h"
CLICK_DECLS

using namespace std;

void BasicElement::push(int port, Packet *p){
  count++;
  cout << "Received packet. Updating count to: "<<count<<endl;
  unsigned char *data = p->data();
  unsigned int data_len = p->length();
  long int response_len = sendData(data, data_len);
  if(response_len != data_len){
	p.kill();
  } else{
  	output(0).push(p);
  }
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
long int BasicElement::sendData(unsigned char* data, unsigned int length){
	struct sockaddr_in *server_socket;
    struct addrinfo hints;
	struct addrinfo *serverinfo, *iter;
	unsigned char recv_buff[100];
	unsigned char header[100];
	sprintf(header, "data_len:%i|data:", length);
	unsigned int header_len = strlen(header);
	unsigned int recv_bytes = 0;
	long int received_count = -3;

	int rc, bytes_received, socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
        printf("Could not get a client socket\n");
        return -1;
    }

	hints.ai_family = AF_UNSPEC;
    hints.ai_family = SOCK_STREAM;
    if((rc = getaddrinfo("localhost", "10000", &hints, &serverinfo)) != 0){
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

	rc = connect(socket_fd, (struct sockaddr*)server_socket, sizeof(struct sockaddr_in));
    if(rc < 0){
        printf("Error connecting to socket\n");
        return -1;
    }

	rc = send(socket_fd, header, header_len, 0);
	if(rc < 0){
		printf("Failed to send the header\n");
		return -1;
	}
    
	rc = send(socket_fd, data, length, 0);
    if(rc < 0){
        printf("Failed to send data\n");
        return -1;
    }

	do{
		//TODO: add timeout
		recv_bytes = recv(socket_fd, recv_buff, 100, 0);
		if(recv_bytes < 0){
			printf("Error receiving\n");
			return -1;
		}
		recv_bytes[99] = 0;
		if(strncmp(recv_buff, "OK:", 3) == 0){
			received_count = strtol(recv_buff + 3, NULL, 0);
			if(received_count < 0){
				printf("Received negative count\n");
				return -2;
			} else{
				return received_count;
			}
		} else if(strncmp(recv_buff, "ERROR", 5) == 0){
			printf("Received error\n");
			return -2;
		} else{
			printf("Received malformed data\n");
			return -2;
		}

	} while(recv_bytes > 0);
	//unreachable?
	return -4;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BasicElement)
