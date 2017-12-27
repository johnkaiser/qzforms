
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
#include "qzrandom64.h"

/* Other includes are specific to a defined algorithm. */

/**
 **  arc4random library call
 ** 
 **/
#ifdef QZ_ARC4RANDOM

#include <stdlib.h>

    void qzrandom64_init(void){ return; }
    
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

    void qzrandom64_init(void){ return; }

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
    
    void qzrandom64_init(void){
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
 *  is_null_free
 *
 *  Return true if the first 8 bytes of buf are not null
 */
bool is_null_free(uint64_t rnbr){

    bool valid = true;
    uint64_t mask = 0xff;
    int n;

    for(n=0; n<8; n++){
        if ((rnbr & mask) == 0) valid = false;
        mask = mask<<8;
    }
    
    return valid;
}


/*
 *  qzrandom64ch
 *
 *  I want a null terminated string of 8 non-null bytes
 *  About 3% of the time, a random string of 8 chars will
 *  contain at least 1 null.  (1-(254/255)**8)
 *  Test and retry until a suitable number is generated.
 *
 *  If buf is not null, then stuff it in buf, which must be
 *  at least 9 bytes, the last for the ending null.
 *  Always return it.
 */  
uint64_t qzrandom64ch(char* buf){

    uint64_t rnbr = 0;
    
    do{ 
        rnbr = qzrandom64();
        if (buf != NULL){
            memcpy(buf, &rnbr, 8);
            buf[8]='\0';
        }    
    } while ( !is_null_free(rnbr) );
    
    return rnbr;
}

/*
 *  qzrandom128ch
 *
 * Similar to qzrandom64ch, except in a 16 bytes.
 * The first 16 bytes are random not \0 chars
 * returned as a struct with an array.
 * If buf is not null, then it is assumed to be
 * at least 17 bytes long, with the same value
 * written to buf as returned.
 * The last byte is always \0.
 */

struct rand128 qzrandom128ch(char* buf){

    struct rand128 data;
    uint64_t rnbr = 0;

    do{
        rnbr = qzrandom64();
    } while ( ! is_null_free(rnbr) );

    memcpy(data.rnbr, &rnbr, 8);

    do{
        rnbr = qzrandom64();
    } while ( ! is_null_free(rnbr) );

    memcpy(data.rnbr + 1, &rnbr, 8);

    if (buf != NULL){
        memcpy(buf, data.rnbr, 16);
        buf[16] = '\0';
    }

    return data;
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

    //qzrandom64ch(rbuf);
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

/*
 *  qzprobability
 * 
 *  Return a float between 0 and 1.
 */
float qzprobability(void){
    uint64_t rnbr = qzrandom64();
    rnbr %= 1000000;

    return (float)rnbr / 1000000.;
}

#ifdef R64_MAIN

#include <stdio.h>
#include <sys/time.h>

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
    qzrandom64_init();
    gettimeofday(&fin_init, NULL);

    for(j=0; j<rounds; j++){
        rnd = qzrandom64(); 
    }
    gettimeofday(&complete, NULL);

    printf("init time %f\n", timeval_diff(&start_init, &fin_init));

    printf("run time for %d rounds %f\n", 
        rounds,
        timeval_diff(&fin_init, &complete)
    );
    unsigned char ar[9];
    unsigned char arc[9];
    uint64_t rv, rvc;
    rv = qzrandom64ch(ar);

    memcpy(arc, &rv, 8); 
    arc[8] = '\0';

    memcpy(&rvc, ar, 8);

    printf("qzrandom64ch=%"PRIu64"=%"PRIu64"\n", rv, rvc);

    for(j=0; j<8; j++){
      printf("%u:", ar[j]);
    }
    printf("\n");
    for(j=0; j<8; j++){
      printf("%u:", arc[j]);
    }
    printf("\n");

    char keybuf[32];
    gen_random_key(keybuf, 32);

    printf( "gen_random_key=%s\n", keybuf);

    bool chfilter_ok = true;
    char chbuf[16];
    uint64_t rnbr;

    for(j=0; j<10000; j++){
        rnbr = qzrandom64ch(chbuf);
        if (strlen(chbuf) != 8){
            printf("bad buf from qzrandom64ch %"PRIx64"\n", rnbr);
            chfilter_ok = false;
        }
    }
    if (chfilter_ok){
        printf("qzrandom64ch is null free %d times\n", j);
    }else{    
        printf("fail\n");
    }    


    for (j=0; j<100; j++){
        printf("p=%f\n", qzprobability());
    }

    struct rand128 rd;
    unsigned char chd[16];
    int i;

    for (j=0; j<100; j++){
        rd = qzrandom128ch(chd);
        for(i=0; i<2; i++){
            printf("%016"PRIX64, rd.rnbr[i]);
        }
        printf("\n");
    }

    return 0;
}
#endif
