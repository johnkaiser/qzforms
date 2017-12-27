
/* 
 * Copyright (c) John Kaiser, http://qzforms.com
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright 
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */ 
 
/*  
 *  A crytpo_etag looks like this:
 *  013efa48188c5ad9692474d746d94f6c.c9d0522c78525077402b755f8bcd0c14
 *
 *  The part before the dot is the initialization vector.
 *  The second part is two aes encrypted 64 bit
 *  unsigned integers.  The first is the server token,
 *  and is a constant and verifies the etag originated
 *  from this server.  The second is the payload.  It
 *  is provided by the calling program and is returned
 *  if the server token validates.  A zero payload
 *  is a failure.  Failures are logged to stderr.
 *
 *  John Kaiser
 *  2013-10-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include "hex_to_uchar.h"
#include "qzrandom64.h"
#include "crypto_etag.h"

#define DEBUG if (debug) fprintf


#ifdef CRYPTO_ETAG_MAIN
bool debug = true;
#endif

/*
 *  make_crypto_etag
 *
 *  Return a text string that is a hex char array 
 *  0123456789abcdef0123456789abcdef.0123456789abcdef0123456789abcdef
 * 
 *  The first part is random 128 bits.
 *  The second part is aes encrypted,
 *  a 64 bit constant, server_token, to prove validity,
 *  a 64 bit non-zero payload.
 *
 *  The result must be freed.
 */

unsigned char* make_crypto_etag(unsigned char key[16], uint64_t server_token, uint64_t payload){

    // Never create a 0 payload, a zero payload is always an error.
    if (payload == 0) return NULL;

    int ivec_hex_len = (IVEC_LEN+1)*2;
    unsigned char ivec[ivec_hex_len];
    bzero(ivec, ivec_hex_len);
    qzrandom128ch(ivec);

    unsigned char* ch_in = (char*)&payload;

    uint64_t data_out;
    unsigned char* ch_out = (char*) &data_out;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    int enclen;
    int rc;

    rc = EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL,  key, ivec);
    if (rc != 1){
        DEBUG(stderr, "EVP_EncryptInit failed\n");
        return NULL;
    }
    rc = EVP_EncryptUpdate(ctx, ch_out, &enclen, ch_in, CRYPT_DATA_LEN);
    if (rc != 1){
        DEBUG(stderr, "EVP_EncryptUpdate failed\n");
        return NULL;
    }

    DEBUG(stderr, "EVP_EncryptUpdate (%d)\n", enclen);

    EVP_CIPHER_CTX_free(ctx);

    // Fill out the etag with hex
    static char etag[ETAG_STR_LEN+2];
    char* iv_hex  = uchar_to_hex(ivec, IVEC_LEN);
    char* data_hex = uchar_to_hex(ch_out, CRYPT_DATA_LEN);
    snprintf(etag, ETAG_STR_LEN+2, "%s.%s%c", iv_hex, data_hex, '\0');

    free(iv_hex);
    free(data_hex);

    return etag;
}

/*
 *  validate_crypto_etag
 *
 *  Decrypt the etag, check if the server token matches, 
 *  if so return the payload, otherwise return 0 for
 *  any kind of error.  0 is never a valid payload.
 *
 *  An etag looks like this:
 *  0123456789abcdef0123456789abcdef.0123456789abcdef0123456789abcdef
 *
 */
uint64_t
validate_crypto_etag(unsigned char key[16], uint64_t server_token, char* etag){

    uint64_t payload = 0;
    int rc;

    DEBUG(stderr, "validate_crypto_etag\n");

    if (strlen(etag) != ETAG_STR_LEN){
        DEBUG(stderr, "invalid etag not %d chars (%zu)\n", ETAG_STR_LEN,
            strlen(etag));
        return 0;
    }

    if (etag[(IVEC_LEN*2)] != '.'){
        DEBUG(stderr, "invalid etag misplaced period\n");
        return 0;
    }
    etag[(IVEC_LEN*2)] = '\0';
    char* hex_ivec = etag;
    char* hex_data = etag + (IVEC_LEN*2) + 1;
    // 
    // These length checks are redundant, but they 
    // ease my paranoia
    //
    if (strlen(hex_ivec) != (IVEC_LEN*2)){
        DEBUG(stderr, "invalid etag ivec not %d chars\n", (IVEC_LEN*2));
        return 0;
    }
    int c;
    for(c=0; c<(IVEC_LEN*2); c++){
        if (isxdigit(hex_ivec[c]) == 0){
            DEBUG(stderr, "non hex digit in ivec\n");
            return 0;
        }
    }
    if (strlen(hex_data) != (CRYPT_DATA_LEN*2)){
        DEBUG(stderr, "invalid etag hex_data not %d chars\n", (CRYPT_DATA_LEN*2));
        return 0;
    }
    for(c=0; c<(CRYPT_DATA_LEN*2); c++){
        if (isxdigit(hex_data[c]) == 0){
            DEBUG(stderr, "non hex digit in hex_data\n");
            return 0;
        }
    }

    DEBUG(stderr, "passed tests\n");

    unsigned char* ivec = hex_to_uchar(hex_ivec);

    if (ivec == NULL){ 
        DEBUG(stderr, "ivec is null\n");
        return 0;
    } // ivec must now be freed

    unsigned char* crypt_data = hex_to_uchar(hex_data);
    if (crypt_data == NULL) {
        free(ivec);
        DEBUG(stderr, "crypt_data is null\n");
        free(ivec);
        return 0;
    } // crypt_data must now be freed

    DEBUG(stderr, "strlen(hex_data)=%zu\n", strlen(hex_data));
    DEBUG(stderr, "hex_to_uchar complete\n");

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    DEBUG(stderr, "cipher context init complete\n");

    rc = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, ivec);
    if (rc != 1){
        DEBUG(stderr, "EVP_DecryptInit failed\n");
        free(ivec);
        free(crypt_data);
        return 0;
    }
    DEBUG(stderr, "decrypt init complete\n");

    int declen;
    //char* ch_out = (char *)  &payload;
    char ch_out[CRYPT_DATA_LEN*2];

    rc = EVP_DecryptUpdate(ctx, ch_out, &declen, crypt_data, (CRYPT_DATA_LEN+1));
    if (rc != 1){
        DEBUG(stderr, "EVP_DecryptUpdate failed\n");
        free(ivec);
        free(crypt_data);
        return 0;
    }
    memcpy(&payload, ch_out, 8);

    DEBUG(stderr, "decrypt update complete (%d)\n", declen);

    EVP_CIPHER_CTX_free(ctx);

    DEBUG(stderr, "payload val set\n");
    free(ivec);
    free(crypt_data);

    DEBUG(stderr, "free complete\n");
    DEBUG(stderr, "validate payload = %"PRIu64"\n", payload);
    return payload;
}

#ifdef CRYPTO_ETAG_MAIN

extern double gettime(void);

void print_uchar(unsigned char* uch, int len){
    int n;
    for(n=0; n<len; n++){
        printf("%02x", uch[n]);
    }
    printf("\n");
    return;
}

int main(void){

    unsigned char* key = hex_to_uchar("013efa48188c5ad9692474d746d94f6c");

    // unsigned char* testdata = "739dd9415d2469b5d44af51e9762a058";
    unsigned char* testcrypt = malloc(64);
    unsigned char* testout = malloc(64);
    bzero(testcrypt,35);
    bzero(testout,35);

    uint64_t payload;
    payload = 420;

    double start = gettime();

    char* etag = make_crypto_etag(key, (uint64_t) 42, payload);

    printf("etag: %s\n", etag);
    printf("etag length is %zu\n", strlen(etag));

    double created = gettime();

    uint64_t pload = validate_crypto_etag(key, (uint64_t) 42, etag);

    double validated = gettime();

    // validate_crypto_etag(payload, key, 42, etag);
    printf("validate returned\n");
    printf("payload = %"PRIu64"\n", pload);

    printf("time to make %f, time to validate %f\n", created - start, validated - created );


    return 0;
}
#endif
