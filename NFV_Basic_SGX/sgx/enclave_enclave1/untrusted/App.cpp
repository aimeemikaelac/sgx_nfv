#include "App.h"

sgx_enclave_id_t global_eid = 0;
volatile unsigned char* data_buffer_in[50];
volatile int buffer_in_write;
volatile int buffer_in_read;
volatile unsigned int data_buffer_out[50];
volatile int buffer_out_write;
volatile int buffer_out_read;
volatile unsigned int packet_length;
volatile unsigned int valid = 0;
volatile unsigned int iterations = 0;
volatile int count = -1;
unsigned int iterations_sum = 0;
bool initial_call = false;
pthread_t sgx_thread;

using namespace std;

void ocall_print(const char* str) {
    printf("%s", str);
}

void ocall_print_hexbyte(unsigned char byteval){
    printf("%02x", byteval);
}

void ocall_print_int(int val){
    printf("%i", val);
}

void print_error_message(sgx_status_t ret)
{
    std::cout << "Encountered SGX error: " << ret << std::endl;
}

void ocall_print_pointer_message(const char* str, void* ptr){
  printf(str, ptr);
}

void ocall_print_int_message(char *message, unsigned int val){
    printf(message, val);
}

const unsigned int SHA256::sha256_k[64] = //UL = uint32
            {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

void SHA256::transform(const unsigned char *message, unsigned int block_nb)
{
    uint32 w[64];
    uint32 wv[8];
    uint32 t1, t2;
    const unsigned char *sub_block;
    int i;
    int j;
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
        for (j = 0; j < 16; j++) {
            SHA2_PACK32(&sub_block[j << 2], &w[j]);
        }
        for (j = 16; j < 64; j++) {
            w[j] =  SHA256_F4(w[j -  2]) + w[j -  7] + SHA256_F3(w[j - 15]) + w[j - 16];
        }
        for (j = 0; j < 8; j++) {
            wv[j] = m_h[j];
        }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) {
            m_h[j] += wv[j];
        }
    }
}

void SHA256::init()
{
    m_h[0] = 0x6a09e667;
    m_h[1] = 0xbb67ae85;
    m_h[2] = 0x3c6ef372;
    m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f;
    m_h[5] = 0x9b05688c;
    m_h[6] = 0x1f83d9ab;
    m_h[7] = 0x5be0cd19;
    m_len = 0;
    m_tot_len = 0;
}

void SHA256::update(const unsigned char *message, unsigned int len)
{
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    tmp_len = SHA224_256_BLOCK_SIZE - m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&m_block[m_len], message, rem_len);
    if (m_len + len < SHA224_256_BLOCK_SIZE) {
        m_len += len;
        return;
    }
    new_len = len - rem_len;
    block_nb = new_len / SHA224_256_BLOCK_SIZE;
    shifted_message = message + rem_len;
    transform(m_block, 1);
    transform(shifted_message, block_nb);
    rem_len = new_len % SHA224_256_BLOCK_SIZE;
    memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
    m_len = rem_len;
    m_tot_len += (block_nb + 1) << 6;
}

void SHA256::final(unsigned char *digest)
{
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    int i;
    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9)
                     < (m_len % SHA224_256_BLOCK_SIZE)));
    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;
    memset(m_block + m_len, 0, pm_len - m_len);
    m_block[m_len] = 0x80;
    SHA2_UNPACK32(len_b, m_block + pm_len - 4);
    transform(m_block, block_nb);
    for (i = 0 ; i < 8; i++) {
        SHA2_UNPACK32(m_h[i], &digest[i << 2]);
    }
}

void test_process_packet(unsigned char *data, unsigned int length){
  int index=0, count = 0, sequence_len = ceil(length/500);
  unsigned char *search_seq = data;
  unsigned char *curr = data;
  while(index<(length-sequence_len)){
    if(memcmp(curr, search_seq, sequence_len) == 0){
      curr += sequence_len;
      index += sequence_len;
      count ++;
    } else{
      curr++;
      index++;
    }
  }
}

int initialize_enclave() {
    char* token_path = TOKEN_FILENAME;
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
    ret = sgx_create_enclave(ENCLAVE1_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
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

void call_process_packet_sgx_sha256(unsigned char *data, unsigned int length){
  sgx_status_t status;
  unsigned char hash[SHA256::DIGEST_SIZE];
  memset(hash, 0, SHA256::DIGEST_SIZE);
  status = ecall_process_packet_sha256(global_eid, data, length, hash);
  if(status != SGX_SUCCESS){
    printf("Error encountered in calling enclave function");
  }
}

// static void *init_sgx_thread(void *args){
//     sgx_status_t status = ecall_process_packet(global_eid,
//                                                (unsigned char**)data_buffer_in,
//                                                (unsigned int*)data_buffer_out,
//                                                (int*)&buffer_in_write,
//                                                (int*)&buffer_in_read,
//                                                (int*)&buffer_out_write,
//                                                (int*)&buffer_out_read,
//                                                (int*)&packet_length);
//     if(status != SGX_SUCCESS){
//       printf("Error encountered in calling enclave function");
//     }
// }

int call_process_packet_no_sgx_test(unsigned char *data, unsigned int length){
  int sequence_len = ceil((double)(length/500));
  int i;
  int index = 0;
  int count=0;
  unsigned char *ptr = data;
  while(index <= (length - sequence_len)){
    bool found = true;
    if(ptr[0] != data[index]){
      index++;
      continue;
    } else{
      for(i=0; i<sequence_len; i++){
        if(ptr[i] != data[index + i]){
          found = false;
          break;
        }
      }
      if(found){
        count++;
        index+=sequence_len;
      } else{
        index++;
      }
    }
  }
  // ocall_print_int_message("Count: %i\n", count);
  return count;
}

int call_process_packet_sgx(unsigned char *data, unsigned int length){
//     int i, rc;
//     static int iterations = 0;
//     if(initial_call == false){
//         rc = pthread_create(&sgx_thread, NULL, init_sgx_thread, NULL);
//         if(rc){
//             printf("Error creating thread: %i\n", rc);
//         } else{
//             initial_call = true;
//             buffer_in_write = 0;
//             buffer_in_read = 0;
//             buffer_out_write = 0;
//             buffer_out_read = 0;
//         }
//     }
//     while(buffer_in_write == (buffer_in_read - 1) % 50 ){
// //        printf("Stuck here: %i, %i\n", buffer_in_write, buffer_in_read);
//         __asm__ __volatile__("");
//     }
//     data_buffer_in[buffer_in_write] = data;
//     // printf("Enqueueing %p\n", data);
//     packet_length = length;
//     buffer_in_write = (buffer_in_write + 1) % 50;
//     while(buffer_out_read == buffer_out_write){
// //        printf("Stuck here in %i: %i, %i\n", iterations, buffer_out_read, buffer_out_write);
//         __asm__ __volatile__("");
//     }
//     int current_out = data_buffer_out[buffer_out_read];
//     buffer_out_read = (buffer_out_read + 1) % 50;
//     iterations++;
//     // printf("Dequeded val: %i\n", current_out);
//     return current_out;
  int sgx_return;
  sgx_status_t status = ecall_process_packet(global_eid,
                                             &sgx_return,
                                             data,
                                             length);
  if(status != SGX_SUCCESS){
    printf("Error encountered in calling enclave function");
  }
  return sgx_return;
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
    int enclave_return = 0;

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
    call_process_packet_sgx(data, data_length);
    memset(response, 0, 100);
    sprintf((char*)response, "OK:%i", enclave_return);
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

//int main(int argc, char const *argv[]) {
//    int i, enclave_return;
//    if (initialize_enclave() < 0) {
//        std::cout << "Enclave initialization failed" << std::endl;
//    }
//
//    unsigned char data[] = {'t','e', 's', 't', '\n'};
//    unsigned int data_len = 5;
//    printf("Expected data:\n0x");
//    for(i=0; i<5; i++){
//        printf("%02x", data[i]);
//    }
//    printf("\n");
//    sgx_status_t status = ecall_process_packet(global_eid, &enclave_return, data, data_len);
//    if (status != SGX_SUCCESS) {
//        std::cout << "SGX error" << std::endl;
//    } else{
//        printf("Received %i from enclave\n", enclave_return);
//    }
//    run_server();
//    return 0;
//}
