
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
 *  8fa2f2ac3953ae0fee15caaf21f1ba23.71c43fa9af8536ae238627f44a15a9d25c1a460a2b55cebf5ec91a6ad150ed58
 *
 *  The part before the dot is the initialization vector.
 *  The second part is 32 bytes of aes encrypted characters
 *  in three fields.
 *  The first is 8 byte server token, and is a constant that
 *  verifies the etag originated from this server.
 *  The second 8 bytes is the domain token and is constant
 *  for each use of the tag. It prevents some replay attacks.
 *  The final 16 bytes is the payload provided by the calling
 *  program and is returned if the server token validates.
 *  A zero payload is a failure.  Failures are logged to stderr.
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

extern double gettime(void);

static bool debug = false;
#define DEBUG if (debug) fprintf

/*
 *  make_crypto_etag
 *
 *  Return a text string that is a hex char array 
 *  0123456789abcdef0123456789abcdef.0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
 * 
 *  The first part is random 128 bits.
 *  The second part is aes encrypted,
 *  a 64 bit constant, server_token, to prove validity,
 *  a 64 bit domain token, to prove its purpose
 *  a 128 bit non-zero payload.
 *
 *  The result must be freed.
 */

static int request_id;

unsigned char* make_crypto_etag(unsigned char key[16], uint64_t server_token,
    uint64_t domain_token, unsigned char payload[16]){

    request_id++;
    debug = false;

    FILE* errlog = NULL;
    if (debug){
        errlog = fopen("errlog", "a");
    }
    DEBUG (errlog, "%f %d %s:%d domain_token=%"PRIu64" payload=%s\n",
        gettime(), request_id, __func__, __LINE__,
        domain_token, payload);

    // Never create a 0 payload, a zero payload is always an error.
    bool all_zeros = true;
    int i;
    for (i=0; i<16; i++){
        if (payload[i] != 0) all_zeros = false;
    }
    if (all_zeros) return NULL;

    // the domain token can not be zero either
    if (domain_token == 0) return NULL;

    int ivec_hex_len = (IVEC_LEN+1)*2; // twice the length for hex.
    unsigned char ivec[ivec_hex_len];
    bzero(ivec, ivec_hex_len);
    qzrandomch(ivec, 16, last_not_null);

    unsigned char ch_in[32];
    memcpy(ch_in, &server_token, 8);
    memcpy(&(ch_in[8]), &domain_token, 8);
    memcpy(&(ch_in[16]), payload, 16);

    unsigned char ch_out[CRYPT_DATA_LEN];

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    int enclen;
    int rc;

    rc = EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL,  key, ivec);
    if (rc != 1){

        DEBUG(errlog, "%f %d %s:%d EVP_EncryptInit failed\n",
            gettime(), request_id, __func__, __LINE__);

        return NULL;
    }
    rc = EVP_EncryptUpdate(ctx, ch_out, &enclen, ch_in, CRYPT_DATA_LEN);
    if (rc != 1){

        DEBUG(errlog, "%f %d %s:%d EVP_EncryptUpdate failed\n",
            gettime(), request_id, __func__, __LINE__);

        return NULL;
    }

    DEBUG(errlog, "%f %d %s:%d EVP_EncryptUpdate enclen=(%d)\n",
        gettime(), request_id, __func__, __LINE__, enclen);

    EVP_CIPHER_CTX_free(ctx);

    // Fill out the etag with hex
    static char etag[ETAG_STR_LEN+2];
    char* iv_hex  = uchar_to_hex(ivec, IVEC_LEN);
    char* data_hex = uchar_to_hex(ch_out, CRYPT_DATA_LEN);
    snprintf(etag, ETAG_STR_LEN+2, "%s.%s%c", iv_hex, data_hex, '\0');

    free(iv_hex);
    free(data_hex);
    if (debug) fclose(errlog);
    return etag;
}

/*
 *  validate_crypto_etag
 *
 *  Decrypt the etag, check if the server token matches, 
 *  if so return the payload, otherwise return 0 for
 *  any kind of error.  0 is never a valid payload.
 */

struct cryptotag
validate_crypto_etag(unsigned char key[16], uint64_t server_token, char* etag){

    FILE* errlog = NULL;
    if (debug){
        errlog = fopen("errlog", "a");
    }

    struct cryptotag ctag;
    struct cryptotag errortag;
    bzero(&errortag, sizeof(struct cryptotag));
    int rc;

    DEBUG(errlog, "%f %d %s:%d validate_crypto_etag %s\n",
        gettime(), request_id, __func__, __LINE__, etag);

    if (strlen(etag) != ETAG_STR_LEN){

        DEBUG(errlog, "%f %d %s:%d invalid etag not %d chars (%zu)\n",
            gettime(), request_id, __func__, __LINE__,
            ETAG_STR_LEN, strlen(etag));

        return errortag;
    }

    if (etag[(IVEC_LEN*2)] != '.'){

        DEBUG(errlog, "%f %d %s:%d invalid etag misplaced period\n",
            gettime(), request_id, __func__, __LINE__);

        return errortag;
    }
    etag[(IVEC_LEN*2)] = '\0';
    char* hex_ivec = etag;
    char* hex_data = etag + (IVEC_LEN*2) + 1;
    // 
    // These length checks are redundant, but they 
    // ease my paranoia
    //
    if (strlen(hex_ivec) != (IVEC_LEN*2)){

        DEBUG(errlog, "%f %d %s:%d invalid etag ivec not %d chars\n",
            gettime(), request_id, __func__, __LINE__, (IVEC_LEN*2));

        return errortag;
    }
    int c;
    for(c=0; c<(IVEC_LEN*2); c++){
        if (isxdigit(hex_ivec[c]) == 0){

            DEBUG(errlog, "%f %d %s:%d non hex digit in ivec\n",
            gettime(), request_id, __func__, __LINE__);

            return errortag;
        }
    }
    if (strlen(hex_data) != (CRYPT_DATA_LEN*2)){

        DEBUG(errlog, "%f %d %s:%d invalid etag hex_data not %d chars\n",

            gettime(), request_id, __func__, __LINE__,
            (CRYPT_DATA_LEN*2));

        return errortag;
    }
    for(c=0; c<(CRYPT_DATA_LEN*2); c++){
        if (isxdigit(hex_data[c]) == 0){

            DEBUG(errlog, "%f %d %s:%d non hex digit in hex_data\n",
                gettime(), request_id, __func__, __LINE__);

            return errortag;
        }
    }

    DEBUG(errlog, "%f %d %s:%d passed tests\n",
        gettime(), request_id, __func__, __LINE__);

    unsigned char* ivec = hex_to_uchar(hex_ivec);

    if (ivec == NULL){ 

        DEBUG(errlog, "%f %d %s:%d ivec is null\n",
            gettime(), request_id, __func__, __LINE__);

        return errortag;
    } // ivec must now be freed

    unsigned char* crypt_data = hex_to_uchar(hex_data);
    if (crypt_data == NULL) {

        DEBUG(errlog, "%f %d %s:%d crypt_data is null\n",
            gettime(), request_id, __func__, __LINE__);

        free(ivec);
        return errortag;
    } // crypt_data must now be freed

    DEBUG(errlog, "%f %d %s:%d strlen(hex_data)=%zu\n",
        gettime(), request_id, __func__, __LINE__, strlen(hex_data));

    DEBUG(errlog, "%f %d %s:%d hex_to_uchar complete\n",
        gettime(), request_id, __func__, __LINE__);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    DEBUG(errlog, "%f %d %s:%d cipher context init complete\n",
        gettime(), request_id, __func__, __LINE__);

    rc = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, ivec);
    if (rc != 1){

        DEBUG(errlog, "%f %d %s:%d EVP_DecryptInit failed\n",
            gettime(), request_id, __func__, __LINE__);

        free(ivec);
        free(crypt_data);
        return errortag;
    }

    DEBUG(errlog, "%f %d %s:%d decrypt init complete\n",
        gettime(), request_id, __func__, __LINE__);

    int declen;
    char ch_out[CRYPT_DATA_LEN*2];

    rc = EVP_DecryptUpdate(ctx, ch_out, &declen, crypt_data, (CRYPT_DATA_LEN+1));
    if (rc != 1){

        DEBUG(errlog, "%f %d %s:%d EVP_DecryptUpdate failed\n",
                gettime(), request_id, __func__, __LINE__);

        free(ivec);
        free(crypt_data);
        return errortag;
    }

    DEBUG(errlog, "%f %d %s:%d decrypt update complete (%d)\n",
        gettime(), request_id, __func__, __LINE__, declen);

    uint64_t check_token;
    memcpy(&check_token, ch_out, 8);
    if (check_token != server_token){

        DEBUG(errlog, "%f %d %s:%d token validation failed\n",
            gettime(), request_id, __func__, __LINE__);

        free(ivec);
        free(crypt_data);
        return errortag;
    }

    // domain token
    memcpy(&(ctag.domain_token), &(ch_out[8]), 8);
    memcpy(&(ctag.payload), &(ch_out[16]), 16);

    DEBUG(errlog, "%f %d %s:%d ctag.domain_token=%"PRIu64" ctag.payload=%s\n",
        gettime(), request_id, __func__, __LINE__,
        ctag.domain_token, ctag.payload);

    EVP_CIPHER_CTX_free(ctx);

    DEBUG(errlog, "%f %d %s:%d payload val set\n",
        gettime(), request_id, __func__, __LINE__);

    free(ivec);
    free(crypt_data);

    DEBUG(errlog, "%f %d %s:%d free complete\n",
        gettime(), request_id, __func__, __LINE__);
    if (debug) fclose(errlog);

    return ctag;
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

    debug = false;
    unsigned char* key = hex_to_uchar("013efa48188c5ad9692474d746d94f6c");

    // unsigned char* testdata = "739dd9415d2469b5d44af51e9762a058";
    unsigned char* testcrypt = malloc(64);
    unsigned char* testout = malloc(64);
    bzero(testcrypt,35);
    bzero(testout,35);

    unsigned char payload[16] = "This is stupid.";
    printf("payload=%s\n", payload);

    double start = gettime();

    char* etag = make_crypto_etag(key, (uint64_t) 42, (uint64_t) 420, payload);

    printf("etag: %s\n", etag);
    printf("etag length is %zu\n", strlen(etag));

    double created = gettime();

    struct cryptotag ctag  = validate_crypto_etag(key, (uint64_t) 42, etag);

    double validated = gettime();

    // validate_crypto_etag(payload, key, 42, etag);
    printf("validate returned\n");
    printf("domain token %"PRIu64"\n", ctag.domain_token);
    printf("payload = %s\n", ctag.payload);

    printf("time to make %f, time to validate %f\n", created - start, validated - created );

    // Make the etag invalid by shifting bytes
    //printf("make it fail with bad data\n");
    //etag = make_crypto_etag(key, (uint64_t) 42, payload);
    //printf("valid etag:   %s\n", etag);
    //unsigned char ch;
    //int k;
    //for(k=0; k<6; k++){
    //    ch = etag[k];
    //    etag[k] = etag[3+k];
    //    etag[3+k] = ch;
    //}
    //printf("invalid etag: %s\n", etag);
    //pload = validate_crypto_etag(key, (uint64_t) 42, etag);
    //printf("invalid validate returned\n");
    //printf("payload = %"PRIu64"\n", pload);
    return 0;
}
#endif
