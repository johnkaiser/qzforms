#include <openssl/evp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

struct cryptotag {
    uint64_t domain_token;
    uint8_t  payload[16];
};

#define IVEC_LEN 16
#define CRYPT_DATA_LEN 32
// Also reference ETAG_MAX_LENGTH in tagger.h
#define ETAG_STR_LEN ((IVEC_LEN*2) + (CRYPT_DATA_LEN*2) + 1)

extern
unsigned char* make_crypto_etag(unsigned char key[16], uint64_t server_token,
    uint64_t domain_token, unsigned char payload[16]);


extern struct cryptotag validate_crypto_etag(unsigned char key[16],
    uint64_t server_token, char* etag);

