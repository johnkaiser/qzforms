#include <openssl/blowfish.h>
#include <stdlib.h>
#include <stdint.h>

extern unsigned char* make_crypto_etag(BF_KEY* bf_key, uint64_t server_token, uint64_t payload);
extern uint64_t validate_crypto_etag(BF_KEY* bf_key, uint64_t server_token, char* etag);

