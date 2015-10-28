
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
#include <string.h>
#include <limits.h>
#include <stdbool.h>

/* 
 *  str_to_array
 *
 *  Given a string and a character to split on,
 *  return an array of strings.
 *
 *  The length if the string to split is 2^30
 *  The maximun number of splits is 2^20.
 */

char** str_to_array(char* str, char split){

    char *ch;
    char *src;
    char *dst;
    int bufsize;
    char** buf;
    bool is_start_segment;
    
    if (str==NULL) return NULL;

    // I need to count the number of segments,
    // The number of segments returned will be less than
    // or equal to the number of segment markers + 1. 
    int nbr_markers = 0;
    int str_len = 0;
    for(ch = str; *ch != '\0'; ch++){
        if (*ch == split) nbr_markers++;
        str_len++;
    }

    // if the number of marker is more than 2^20
    // or the string is more than 2^30
    // then just give up.
    // This avoids an int overflow on the calloc'd size

    if ((nbr_markers+2) > 1048576) return NULL;     
    if (str_len > 1073741824) return NULL;

    // Two values go into the size of the buffer,
    // the number of segments, one pointer for each,
    // and the size of the string.
    bufsize = (sizeof(char*) * (nbr_markers + 2) ) + (str_len + 2);
    buf = calloc(1, bufsize);

    // Start on first char unless it is a split char.
    is_start_segment = (str[0] != split);

    // Warnings about pointer types occur because one buffer 
    // holds strings and string pointers, hence void* tmp. 
    void* tmp = buf + nbr_markers +2 ; 
    dst = tmp;
    int slot = 0;

    for(src=str; *src != '\0'; src++){
       if (is_start_segment){
            buf[slot++] = dst;
       }    
       if( *src == split) {
           *dst = '\0';
           is_start_segment = true;
       }else{
           *dst = *src;
           is_start_segment = false;
       }
       dst++;
     }

     return buf;
}


#ifdef STR_TO_ARRAY_MAIN

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    char** b;
    int j,k;

    for(k=1; k<argc; k++){ 
      b = str_to_array(argv[k], '/');
      //printf("%2d %p %s\n", k, b, "buf" );
    
      for(j=0; b[j] != NULL; j++){
          printf("%2d %p %s\n", j, b[j], b[j]);
      }
      free(b);
      printf("\n");
    }

    b = str_to_array("", 'x');
    printf( "zero length string reference %s\n", b[0]);


    int fd = open("/usr/share/dict/words", O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    int64_t size = sb.st_size;
    char* words = calloc(1, size+2);
    read(fd, words, size);

    char** word_ar = str_to_array(words, '\n');
    if (word_ar == NULL){
        printf("str_to_array returned null\n");
        exit(1);
    }    
    char** w;
    for (w=word_ar; *w != NULL; w++){
        printf("%s\n", *w);
    }    

    return 0;
} 

#endif
