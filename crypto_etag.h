#include <openssl/evp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define IVEC_LEN 16
#define CRYPT_DATA_LEN 16
// Also reference ETAG_MAX_LENGTH in tagger.h
#define ETAG_STR_LEN ((IVEC_LEN*2) + (CRYPT_DATA_LEN*2) + 1)

bool debug;

extern unsigned char* make_crypto_etag(unsigned char key[16],
    uint64_t server_token, uint64_t payload);

extern uint64_t validate_crypto_etag(unsigned char key[16],
    uint64_t server_token, char* etag);

