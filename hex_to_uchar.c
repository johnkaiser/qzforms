
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>

/*
 *  hex_to_uchar
 *
 *  Convert a hex character string to an array of 
 *  unsigned chars.
 *
 *  The result must be freed.
 *
 *  There are two algorithms implementing this function.
 *
 *  If USE_BN is not defined, a home rolled 
 */
 
#ifndef USE_BN

unsigned char*  hex_to_uchar(char* str){
    if (str==NULL) return NULL;

    int len = strlen(str);
    if (len == 0) return NULL;

    bool is_odd = (len % 2 == 1);
    //printf("len=%d is_odd=%c  ", len, 
    //    (is_odd) ? 't':'f' );

    unsigned char* uch; 

    // +1 so an odd length allocates correctly
    uch = malloc((len+1)/2);

    int in; 
    int out = 0;
    unsigned char ch[3];

    // If the input string contains a non-hex digit
    // then return null.
    if(!isxdigit(str[0])){
        free(uch);
        return NULL;
    }
    ch[0] = str[0];
    if (is_odd){
        ch[1] = '\0';
        in = 1;
        out = 1;
    }else{
        ch[1] = str[1];
        ch[2] = '\0';
        in = 2;
        out = 1;
        if(!isxdigit(str[1])){
            free(uch);
            return NULL;
        }
    }
    sscanf(ch, "%hhx", uch);
    // printf("%s=%x, ", ch, uch[0]);

    while(in<len){

         ch[0] = str[in];
         ch[1] = str[in+1];
         ch[2] = '\0';
         if ( (!isxdigit(ch[0])) || (!isxdigit(ch[1])) ){
             free(uch);
             return NULL;
         }
         sscanf(ch, "%hhx", uch + out);

         // printf("%s=%x, ", ch, uch[out]);
         out++;
         in+=2;
    }

    return uch;
}
#endif


#ifdef USE_BN
#include <openssl/bn.h>

unsigned char*  hex_to_uchar(const char* str){
    //BIGNUM* bn = BN_new();
    BIGNUM bn;
    BIGNUM *bnp = &bn;

    BN_init(&bn);

    char* to;
    int size;

    //printf("BN_hex2bn length = %d ", BN_hex2bn(&bnp, str));
    size = BN_num_bytes(&bn);
    to = calloc(1, size+1);
    //printf("BN_bn2bin length = %d\n", BN_bn2bin(&bn, to));

    //BN_clear_free(bn);
    BN_clear(&bn);
    return to;
}
#endif

#ifdef HEX_TO_UCHAR_MAIN 

int main(void){
    char* test[] = { 
        "",
        "1",
        "12",
        "123",
        "1234",
        "12345",
        "123456",
        "123456789abcdef",
        "0",
        "00",
        "01",
        "001",
        "0012",
        "012X",
        "0123X",
        NULL
    };
    int n_test = 15;
    int nt;

    unsigned char* uch;
    int len;

    struct timeval start;
    struct timeval fin;
    
    for(nt=0; nt<n_test; nt++){
    
        printf("converting: %s  ", test[nt]);
        len = (strlen(test[nt])+1)/2;
        gettimeofday(&start, NULL); 
        uch = hex_to_uchar(test[nt]);
        gettimeofday(&fin, NULL); 

        long t = ((fin.tv_sec*1000000 + fin.tv_usec)
             - (start.tv_sec*1000000 + start.tv_usec));

        printf(" (%ld usec) ", t);
    
        int n;
        if (uch != NULL){
            printf("output: ");
            for(n=0; n<len; n++){
                printf("%02x", uch[n]);
            }
            printf("\n");
        }else{
            printf("output is null\n");
        }
    }
    return 0;
}   
        
#endif
