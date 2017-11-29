
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
 *  c0e9bd0cddeea0a5.c9d0522c78525077402b755f8bcd0c14
 *
 *  The part before the dot is the initialization vector.
 *  The second part is two blowfish encrypted 64 bit 
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
#include <openssl/blowfish.h>
#include <openssl/bn.h>
#include "hex_to_uchar.h"
#include "qzrandom64.h"

/*
 *  make_crypto_etag
 *
 *  Return a text string that is a hex char array 
 *  0123456789abcdef.0123456789abcdef0123456789abcdef
 * 
 *  The first part is random.
 *  The second part is blowfish encrypted,
 *  a 64 bit constant, server_token, to prove validity,
 *  a 64 bit non-zero payload. 
 *
 *  The result must be freed.
 */

unsigned char* make_crypto_etag(BF_KEY* bf_key, uint64_t server_token, uint64_t payload){

    // Never create a 0 payload, a zero payload is always an error.
    if (payload==0) return NULL;

    // the size will be:
    // 16 hex chars for the  8 bit ivec, 
    // 1 byte .,
    // 16 hex chars for the 8 byte server token,
    // 16 hex chars for the 8 byte paylod, 
    // 1 null,  1 extra null for good luck, 
    // 51 bytes total.

    unsigned char* etag = calloc(1, 51);
    unsigned char* ch = etag;

    // 8 random bytes for the ivec plus a null,
    unsigned char ivec[9];
    qzrandom64ch(ivec);

    // Save the ivec as hex to the output buffer at ch.
    int j;
    for(j=0; j<8; j++){
        // 3 is 2 for 2 hex digits + 1 null
        snprintf(ch, 3, "%02x", ivec[j]);
        ch += 2;
    }        
    *ch = '.';
    ch++;

    // Mapping 64 bit integer input to a char array for BF.
    uint64_t data[2];
    data[0] = server_token;
    data[1] = payload;

    unsigned char* data_in = (unsigned char*) &data;
    unsigned char data_out[16];


    BF_cbc_encrypt(data_in, data_out, 16, bf_key, ivec, BF_ENCRYPT);

    // Fill out the rest of the etag with hex from BF char array. 
    for(j=0; j<16; j++){
        snprintf(ch, 3, "%02x", data_out[j]);
        ch += 2;
    }
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
 *  c0e9bd0cddeea0a5:c9d0522c78525077402b755f8bcd0c14
 *
 */
uint64_t validate_crypto_etag(BF_KEY* bf_key, uint64_t server_token, char* etag){

    //fprintf(stderr, "vce,");
    uint64_t payload = 0xFFFFFFFF;

    if (strlen(etag) != 49){
        fprintf(stderr, "invalid etag not 49 chars\n");
        return 0;
    }

    if (etag[16] != '.'){
        fprintf(stderr, "invalid etag misplaced period\n");
        return 0;
    }
    etag[16] = '\0';
    char* hex_ivec = etag;
    char* hex_data = etag + 17;

    // 
    // These length checks are redundant, but they 
    // ease my paranoia
    //
    if (strlen(hex_ivec) != 16){
        fprintf(stderr, "invalid etag ivec not 16 chars\n");
        return 0;
    }
    int c;
    for(c=0; c<16; c++){
        if (isxdigit(hex_ivec[c]) == 0){
            fprintf(stderr, "invalid etag ivec not hex\n");
            return 0;
        }
    }
    if (strlen(hex_data) != 32){
        fprintf(stderr, "invalid etag data not 32 chars\n");
        return 0;
    }
    for(c=0; c<32; c++){
        if (isxdigit(hex_data[c]) == 0){
            fprintf(stderr, "invalid etag data not hex\n");
            return 0;
        }
    }

    //fprintf(stderr, "ok,");

    unsigned char* ivec = hex_to_uchar(hex_ivec);
    if (ivec == NULL){ 
        fprintf(stderr, "invalid etag ivec hex conversion failed\n");
        return 0; 
    } // ivec must now be freed

    //fprintf(stderr, "ivec,");

    unsigned char* crypt_data = hex_to_uchar(hex_data);
    if (crypt_data == NULL) {
        fprintf(stderr, "invalid etag crypt_data hex conversion failed\n");
        free(ivec);
        return 0;
    } // crypt_data must now be freed
    //fprintf(stderr, "data,");

    // I need the data to be both character and integer
    unsigned char plain_data[33];
    bzero(plain_data, 33);
    uint64_t* data = (uint64_t*) plain_data;

     
    //fprintf(stderr, "decrypt,");
    BF_cbc_encrypt(crypt_data, plain_data, 16, bf_key, ivec, BF_DECRYPT);
    //fprintf(stderr, "OK,");

    if (data[0] == server_token){
        payload = data[1];
    }else{
        fprintf(stderr, "invalid etag server token does not match\n");
        payload = 0;
    }
    free(ivec);
    free(crypt_data);

    return payload;
}

#ifdef CRYPTO_ETAG_MAIN

void print_uchar(unsigned char* uch, int len){
    int n;
    for(n=0; n<len; n++){
        printf("%02x", uch[n]);
    }
    printf("\n");
    return;
}

int main(void){
    char* hex_key  = "013efa48188c5ad9692474d746d94f6c";
    char* hex_ivec = "cc63a18ce8cf2621";
    unsigned char* testdata = "739dd9415d2469b5d44af51e9762a058";
    unsigned char* testcrypt = malloc(35);
    unsigned char* testout = malloc(35);
    bzero(testcrypt,35);
    bzero(testout,35);

    BF_KEY* bf_key;
    bf_key = malloc(sizeof(BF_KEY));
    unsigned char *keydata = hex_to_uchar(hex_key);
    unsigned char *ivec = hex_to_uchar(hex_ivec);
 
    BF_set_key(bf_key, 16, keydata);

    printf("\nbefore: %s\n", testdata);
    BF_cbc_encrypt(testdata, testcrypt, 32, bf_key, ivec, BF_ENCRYPT);

    free(ivec);
    ivec = hex_to_uchar(hex_ivec);

    BF_cbc_encrypt(testcrypt, testout, 32, bf_key, ivec, BF_DECRYPT);
    printf("after:  %s\n", testout);

    char* etag = make_crypto_etag(bf_key, (uint64_t) 42, (uint64_t) 420);
    printf("etag: %s\n", etag);

    uint64_t payload;
    payload = validate_crypto_etag(bf_key, 42, etag);
    printf("payload = %"PRIu64"\n", payload);

    free(ivec);
    free(keydata);

    return 0;
}
#endif
