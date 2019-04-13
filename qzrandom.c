
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
 *  qzrandom64()
 *
 *  Returns a hopefully high quality random 64 bit unsigned integer
 *
 *  There are three sources of randomness available.
 *
 *    1. arc4random library call
 *    2. Device file
 *    3a. OpenSSL RAND_bytes library call without seeding
 *    3b. OpenSSL RAND_bytes library call seeding with a local program
 *
 *
 *  If QZ_ARC4RANDOM is set, then arc4random_buf will be called
 *  from stdlib.h.
 *
 *  If QZ_RAND_DEV is a character device file, then it is set
 *  in the application as the source of randomness.
 *
 *  arc4random is recommended if available.  A device file will work
 *  just fine.
 *
 *  John Kaiser
 *  2013-11-11
 */
 
#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "qzrandom.h"

/* Other includes are specific to a defined algorithm. */

/**
 **  arc4random library call
 **
 **/
#ifdef QZ_ARC4RANDOM

#include <stdlib.h>

    void qzrandom_init(void){ return; }

    uint64_t qzrandom64(void){
        uint64_t rnbr;
        arc4random_buf(&rnbr, sizeof(rnbr));
        return rnbr;
    }

#endif

/**
 **
 **  getrandom
 **
 */
#ifdef QZ_GETRANDOM
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <errno.h>
#include <linux/random.h>

    void qzrandom_init(void){ return; }

    uint64_t qzrandom64(void){
        uint64_t rnbr;
        int gr;

        gr = syscall(SYS_getrandom, &rnbr, sizeof(rnbr), 0);

        if (gr == sizeof(rnbr)){
            return rnbr;
        }else{
            // According to the man page, this can happen
            // when a signal interrupts the call.
            return qzrandom64();
        }
    }

#endif

/**
 ** A named device file
 **
 **/

#ifdef QZ_RAND_DEV
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

    int randdev_fd;

    void qzrandom_init(void){
       randdev_fd = open(QZ_RAND_DEV, O_RDONLY, 0);
       if (randdev_fd < 0){
           perror(QZ_RAND_DEV);
           exit(41);
       }
       return;
    }

    uint64_t qzrandom64(void){

        uint64_t rnbr;
        ssize_t bytesread;

        bytesread = read(randdev_fd, &rnbr, 8);
        if (bytesread != 8){
            perror(QZ_RAND_DEV);
            exit(42);
        }
        return rnbr;
    }

#endif

/*
 *  qzrandomch
 *
 *  Fill up buf with bytes number of characters,
 *  making sure that each byte is not zero,
 *  optionally setting the last byte to zero.
 */
void qzrandomch(char* buf, uint8_t bytes, enum last_char_rule rule){

    unsigned int bt;
    unsigned int src=sizeof(uint64_t);
    char rchar[sizeof(uint64_t)];
    uint64_t num;

    bt = 0;
    while (bt < bytes){

        if (src >= sizeof(uint64_t)){
            src=0;
            num = qzrandom64();
            memcpy(rchar, &num, sizeof(uint64_t));
        }
        if (rchar[src] != '\0'){
            buf[bt] = rchar[src];
            bt++;
        }
        src++;
    }
    if (rule == last_is_null) buf[bytes-1] = '\0';
}
/*
 * gen_random_key
 *
 * Fill the buffer with 63 chars that represent a
 * real big random number.
 */
void gen_random_key(char keybuf[], int key_length ){

    // All characters here must be valid for a file name suffix, 63 of them.
    // The first 4 will be used more than the others.
    char base[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";

    uint64_t rnbr;
    int rch = 0;
    int j,k;
    unsigned char * ch = (void*) &rnbr;
    int base_len = strlen(base);

    rnbr = qzrandom64();

    for (j=0; j<key_length; j++){
      if (rch >= 8){
        rnbr = qzrandom64();
        rch = 0;
        }
        /*
         * Condense the keybuf from 2^8 to 2^6-1 (base_len 63)
         * expressed as a char in base.
         */

        k = ch[rch++] % base_len;
        keybuf[j] = base[k];
    }
    keybuf[key_length-1] = '\0';
}
#ifdef TEST_QZRANDOM

#include <stdio.h>
#include <sys/time.h>
#include "hex_to_uchar.h"

double timeval_diff(struct timeval* starttv, struct timeval* fintv){

    double startd, find;

    startd = starttv->tv_sec + (starttv->tv_usec * 0.000001);
    find = fintv->tv_sec + (fintv->tv_usec * 0.000001);

    return find - startd;
}

int main(void){

    int j;
    int rounds = 1000000;
    uint64_t rnd;
    struct timeval start_init;
    struct timeval fin_init;
    struct timeval complete;

    gettimeofday(&start_init, NULL);
    qzrandom_init();
    gettimeofday(&fin_init, NULL);

    // Time a bunch of rounds.
    for(j=0; j<rounds; j++){
        rnd = qzrandom64();
    }
    gettimeofday(&complete, NULL);

    printf("init time %f\n", timeval_diff(&start_init, &fin_init));

    printf("run time for %d rounds %f\n",
        rounds,
        timeval_diff(&fin_init, &complete)
    );

    char keybuf[32];
    gen_random_key(keybuf, 32);

    printf( "gen_random_key=%s\n", keybuf);

    unsigned char chd[16];
    unsigned char* hex;

    struct timeval start_qzrandomch;
    struct timeval fin_qzrandomch;
    gettimeofday(&start_qzrandomch, NULL);

    for (j=0; j<1000; j++){
        qzrandomch(chd, 16, last_is_null);
        hex = uchar_to_hex(chd, 16);
        printf("%3d %s\n", j, hex);
        free(hex);
    }
    gettimeofday(&fin_qzrandomch, NULL);
    printf("run time for %d rounds of qzrandomch 128 bits %f\n",
        1000, timeval_diff(&start_qzrandomch, &fin_qzrandomch)
    );

    return 0;
}
#endif
