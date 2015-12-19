
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
 *  hex_to_val
 *
 *  Turn a single ascii charecter into its numerical value.
 *  Return -1 in case of a bad character.
 */

inline int hex_to_val(unsigned char xd){

    if ((xd >= '0') && (xd <= '9')){
        return xd - '0';
    }
    if ((xd >= 'a') && (xd <= 'f')){
        return xd - 'a' + 10;
    }
    if ((xd >= 'A') && (xd <= 'F')){
        return xd - 'A' + 10;
    }

    return -1;
}


/*
 *  hex_to_uchar
 *
 *  Convert a hex character string to an array of 
 *  unsigned chars.
 *  An even number of hex chars must be given or 
 *  null is returned.
 *
 *  The result must be freed.
 */
 
unsigned char* hex_to_uchar(char* instr){
    if (instr==NULL) return NULL;

    int slen = strlen(instr);

    bool is_odd = (slen % 2 == 1);
    if (is_odd) return NULL; 

    // 200MB of hex or 100MB of text is the size limit

    if ((slen == 0) || (slen > 209715200)) return NULL;

    unsigned char* outstr; 
    unsigned char const canary = 0xff;

    outstr = calloc(1, slen/2 + 4);
    outstr[slen/2 + 2] = canary;

    char* outch = outstr;
    char* inch = instr;

    while (*inch != '\0'){
        int hival,loval;

        hival = hex_to_val(*inch++);
        loval = hex_to_val(*inch++);
        if ((hival == -1)||(loval == -1)){
           free(outstr);
           return NULL;
        }
        
        *outch++ = hival*16 + loval;
    }

    if ((outstr[slen/2+1] != '\0')
        || (outstr[slen/2+2] != canary)
        ||  (outstr[slen/2+3] != '\0')){
        
        exit(51);
    }    
    return outstr;
}


/*
 *  uchar_to_hex
 *
 *  Convert a character string to hex.
 *
 *  The result must be freed.
 */

unsigned char* uchar_to_hex(unsigned char* instr){
    if (instr == NULL) return NULL;

    unsigned int slen = strlen(instr);
    // If its more than 100MB then give up.
    // It's a web app after all.
    if (slen > 104857600) return NULL;

    unsigned char const canary = 0xff;

    unsigned char* outstr = calloc(2, slen + 4);
    outstr[2*slen + 2] = canary;
    unsigned char* outch = outstr;
    unsigned char* inch;

    for (inch=instr; *inch!='\0'; inch++){
        div_t hexval = div(*inch, 16);

        *outch++ = (hexval.quot < 10) ? '0'+hexval.quot : 'a'+hexval.quot-10;
        *outch++ = (hexval.rem < 10) ? '0'+hexval.rem : 'a'+hexval.rem-10;
    }
    
    if ((outstr[2*slen+1] != '\0')
        || (outstr[2*slen+2] != canary)
        || (outstr[2*slen+3] != '\0')){
        
        exit(52);
    }    
    return outstr;
}


#ifdef HEX_TO_UCHAR_MAIN 

extern double gettime(void);

int main(void){
    char* test[] = { 
        "",
        "1",
        "12",
        "486921",
        "123456",
        "0123456789abcdef",
        "0",
        "00",
        "01",
        "001",
        "1200",
        "012X",
        " 12",
        "77616765207065616365",
        "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
        NULL
    };
    int nt;

    unsigned char* uch;
    unsigned char* back;

    double prog_start = gettime();
    double prog_fin;

    double conv_start;
    double conv_back;
    double conv_fin;
    
    for(nt=0; test[nt]!=NULL; nt++){
    
        printf("converting: [%s]  ", test[nt]);
        conv_start = gettime();

        uch = hex_to_uchar(test[nt]);
        conv_back = gettime();

        back = uchar_to_hex(uch);
        conv_fin = gettime();

        printf("hex_to_uchar [%s] %f ", uch, conv_back - conv_start);
        printf("uchar_to_hex [%s] %f ", back, conv_fin - conv_back);

        if ((test[nt] != NULL) && (back != NULL)){
            printf("%s\n", (strcmp(test[nt],back) == 0) ? "OK":"??");
        }else{
            printf("NULL\n");
        }    
        free(uch);
        free(back);
    }

    prog_fin = gettime();
    printf("run time: %f\n", prog_fin - prog_start);
    return 0;
}   
        
#endif
