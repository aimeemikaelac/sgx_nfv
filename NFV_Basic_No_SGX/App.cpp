#include "App.h"

using namespace std;

int process_packet(unsigned char* data, unsigned int length){
    printf("In packet processing - NOT enclave\n");
    //search for occurrences of 0a
    int i, count = 0;
    for(i=0; i<length; i++){
        if(data[i] == 0x0a){
            count++;
        }
    }
    printf("Detected %i occurrences of the byte 0a\n", count);
    return count;

}

void handle_connection(int socket_fd){
    int recv_size = 1000;
    int recv_offset = 0;
    int bytes_received = 0;
    long int data_length = 0;
    int num_len_chars = 0;
    int header_len = 0;
    int total_data = 0;
    int total_received = 0;
    unsigned char *recv_buff = NULL;
    unsigned char *more_recv_buff = NULL;
    unsigned char *data;
    unsigned char response[100];

    recv_buff = (unsigned char*)malloc(recv_size);
    if(recv_buff == NULL){
        printf("Error allocating receive buffer\n");
        close(socket_fd);
        return;
    }
    memset(recv_buff + recv_offset, 0, 1000);
    bytes_received = recv(socket_fd, recv_buff, 1000, 0);
    if(strncmp("data_len:", (char *)recv_buff, 9) != 0){
        printf("Malformed header. Aborting");
        free(recv_buff);
        close(socket_fd);
        return;
    }
    total_received = bytes_received;
    data_length = strtol((char*)recv_buff+9, (char**)&data, 0);
    if(strncmp("|data:", (char*)data, 6) != 0){
        printf("Malformed header after data length\n");
        close(socket_fd);
        free(recv_buff);
        return;
    }
    num_len_chars = data - recv_buff - 9;
    data = data + 6;
    header_len = 9 + num_len_chars + 6;
    total_data = header_len + data_length;
    while(total_received < total_data){
        recv_size += 1000;
        recv_offset += 1000;
        more_recv_buff = (unsigned char*)realloc(recv_buff, recv_size);
        if(more_recv_buff == NULL){
            printf("Failed to extend buffer to %i", recv_size);
            close(socket_fd);
            free(recv_buff);
            return;
        } else{
            recv_buff = more_recv_buff;
            more_recv_buff = NULL;
        }
        bytes_received = recv(socket_fd, recv_buff + recv_offset, 1000, 0);
        if(bytes_received < 0){
            printf("Error receiving data. Aborting");
            close(socket_fd);
            free(recv_buff);
            return;
        }
        total_received += bytes_received;
    }
//    ecall_process_packet(global_eid, data, data_length);
    process_packet(data, data_length);
    memset(response, 0, 100);
    //for now, just send back the data length we received
    sprintf((char*)response, "OK:%i", (int)data_length);
    if(send(socket_fd, response, strlen((char*)response) + 1, 0) < 0){
        printf("Error sending response\n");
    }
    close(socket_fd);
    free(recv_buff);

}

void run_server(){
    int socket_fd, status, new_connection_fd;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(10000);
    char clientIp[INET_ADDRSTRLEN];

    std::cout << "Running server" <<std::endl;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        std::cout << "Error opening socket file descriptor" << std::endl;
        return;
    }

    //set socket reusable from previous program run
    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    //bind to socket
    status = bind(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    if(status < 0){
        std::cout << "Error binding on socket" << std::endl;
        return;
    } else{
        std::cout << "Running server on TCP port: "<<10000<<std::endl;
    }

    //listen on socket with a backlog of 10 connections
    status = listen(socket_fd, 100);
    if(status < 0){
        cout << "Error listening on bound socket" << endl;
        return;
    } else{
        cout << "Listening on TCP socket" << endl;
    }

    while(true){
        cout << "Accepting new connections" << endl;

        struct sockaddr connection_addr;
        socklen_t connection_addr_size = sizeof(sockaddr);

        //block on accept until new connection
        new_connection_fd = accept(socket_fd, &connection_addr, &connection_addr_size);
        if(new_connection_fd < 0){
            cout << "Error accepting connection" <<endl;
        }

        //calculate ip of client
        int ip = ((struct sockaddr_in*)(&connection_addr))->sin_addr.s_addr;
        inet_ntop(AF_INET, &ip, clientIp, INET_ADDRSTRLEN);
        cout << "Accepted new connection from: " << string(clientIp) << " in socket: " << new_connection_fd << endl;

        //handle connection single-threaded for now
        handle_connection(new_connection_fd);
    }
}

int main(){
    int i, processing_return;
    unsigned char data[] = {'t','e', 's', 't', '\n'};
    unsigned int data_len = 5;
    printf("Expected data:\n0x");
    for(i=0; i<5; i++){
        printf("%02x", data[i]);
    }
    printf("\n");
    processing_return = process_packet(data, data_len);
    printf("Received %i from processing\n", processing_return);
    run_server();
    return 0;
}
