#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <stdlib.h>

#include <sys/time.h>
#include "limits.h"

# define MAX_PATH FILENAME_MAX


#include <sgx_urts.h>
#include "sample.h"

#include "enclave_u.h"

#include "string.h"
#include "tomcrypt.h"


/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

void ocall_print(const char* str) {
    printf("%s", str);
}

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
        printf("Error: Unexpected error occurred.\n");
}

/* Initialize the enclave:
 *   Step 1: retrive the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */
int initialize_enclave(void)
{
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;
    /* Step 1: retrive the launch token saved by last transaction */

    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;
    if (home_dir != NULL &&
        (strlen(home_dir)+strlen("/")+sizeof(TOKEN_FILENAME)+1) <= MAX_PATH) {
        /* compose the token path */
        strncpy(token_path, home_dir, strlen(home_dir));
        strncat(token_path, "/", strlen("/"));
        strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+1);
    } else {
        /* if token path is too long or $HOME is NULL */
        strncpy(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
    }

    FILE *fp = fopen(token_path, "rb");
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }
    printf("token_path: %s\n", token_path);
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

    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);

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

/* OCall functions */
void ocall_enclave_sample(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    printf("%s", str);
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

std::string sha256(std::string input)
{
    unsigned char digest[SHA256::DIGEST_SIZE];
    memset(digest,0,SHA256::DIGEST_SIZE);

    SHA256 ctx = SHA256();
    ctx.init();
    ctx.update( (unsigned char*)input.c_str(), input.length());
    ctx.final(digest);

    char buf[2*SHA256::DIGEST_SIZE+1];
    buf[2*SHA256::DIGEST_SIZE] = 0;
    for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    return std::string(buf);
}

void stress(){
  int i;
  char *data = "hello";
  // unsigned char hash[SHA256_DIGEST_LENGTH];
  // memset(hash, 0, SHA256_DIGEST_LENGTH);
  // SHA256_CTX sha256;
  // SHA256_Init(&sha256);
  // SHA256_Update(&sha256, data, strlen(data));
  // SHA256_Final(hash, &sha256);
  // for(i=0; i<10000; i++){
  //   SHA256_Init(&sha256);
  //   SHA256_Update(&sha256, hash, SHA256_DIGEST_LENGTH);
  //   SHA256_Final(hash, &sha256);
  // }
  // hash_state sha256;
  // unsigned char hash[32];
  // sha256_init(&sha256);
  // sha256_process(&sha256, (unsigned char*)data, strlen(data));
  // sha256_done(&sha256, hash);
  // for(i=0; i<10000; i++){
  //   sha256_init(&sha256);
  //   sha256_process(&sha256, hash, 32);
  //   sha256_done(&sha256, hash);
  // }
  unsigned char hash[SHA256::DIGEST_SIZE];
  SHA256 digest;
  digest.init();
  digest.update((unsigned char*)data, strlen(data));
  digest.final(hash);
  for(i=0; i<100000; i++){
    digest.init();
    digest.update(hash, strlen(data));
    digest.final(hash);
  }
}

void stress_memory(unsigned char* data, int length){
  int i;
  // unsigned char hash[SHA256::DIGEST_SIZE];
  // SHA256 digest;
  // digest.init();
  // for(i=0; i<1000; i++){
  //   digest.update(data, length);
  // }
  // digest.final(hash);
  volatile unsigned char* data_local = data;
  for(i=1; i<length; i++){
    data_local[i]+= data_local[i-1];
  }
}

void sha(unsigned char* data, int length, unsigned char* hash_out){
  SHA256 digest;
  digest.init();
  digest.update(data, length);
  digest.final(hash_out);
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    int i;

    /* Changing dir to where the executable is.*/
    char absolutePath [MAX_PATH];
    char *ptr = NULL;

    ptr = realpath(dirname(argv[0]),absolutePath);

    if( chdir(absolutePath) != 0)
    		abort();

    /* Initialize the enclave */
    if(initialize_enclave() < 0){

        return -1;
    }

//    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
//    int ecall_return = 0;
//
//    ret = ecall_enclave_sample(global_eid, &ecall_return);
//    if (ret != SGX_SUCCESS)
//        abort();
//
//    if (ecall_return == 0) {
//      printf("Application ran with success\n");
//    }
//    else
//    {
//        printf("Application failed %d \n", ecall_return);
//    }
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);

    stress();

    gettimeofday(&tv2, NULL);
    printf("Time taken in execution of no-sgx = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));

    sgx_status_t ret;
    gettimeofday(&tv1, NULL);

    ret = ecall_stress(global_eid);

    if (ret != SGX_SUCCESS)
      abort();

    gettimeofday(&tv2, NULL);
    printf("Time taken in execution of sgx = %f seconds\n",
             (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
             (double) (tv2.tv_sec - tv1.tv_sec));

    int size = 512*1024;
    unsigned char* data = NULL;
    data = (unsigned char*)malloc(size);
    if(data == NULL){
      printf("malloc failed\n");
      abort();
    }
    memset(data, 0, size);

    unsigned char hash_out[SHA256::DIGEST_SIZE];
    gettimeofday(&tv1, NULL);

    // stress_memory(data, size);

    for(i=0; i<10000; i++){
      sha(data, size, hash_out);
    }

    gettimeofday(&tv2, NULL);
    printf("Time taken in execution of no-sgx memory = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));

    unsigned char hash[SHA256::DIGEST_SIZE];
    // SHA256 digest;
    // digest.init();
    // digest.update(data, size);
    // digest.final(hash);

    // printf("No-sgx hash: 0x");
    // for(i=0; i<SHA256::DIGEST_SIZE; i++){
    //   printf("%02x", hash[i]);
    // }
    // printf("\n");

    memset(data, 0, size);
    gettimeofday(&tv1, NULL);

    // ret = ecall_stress_memory(global_eid, data, size);

    for(i=0; i<10000; i++){
      ecall_sha(global_eid, data, size, hash_out);
    }

    if (ret != SGX_SUCCESS){
      printf("SGX error: %02x\n", ret);
      abort();
    }

    gettimeofday(&tv2, NULL);
    printf("Time taken in execution of sgx memory = %f seconds\n",
            (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
            (double) (tv2.tv_sec - tv1.tv_sec));

    // digest.init();
    // digest.update(data, size);
    // digest.final(hash);

    // printf("SGX hash: 0x");
    // for(i=0; i<SHA256::DIGEST_SIZE; i++){
    //   printf("%02x", hash[i]);
    // }
    // printf("\n");

    sgx_destroy_enclave(global_eid);
    free(data);
    return 0;
}
