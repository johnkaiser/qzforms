
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
 * A library for managing a double linked list of strings.
 *
 * john kaiser 2012-04-15
 *
 */


#include "strbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>


/*
 * new_strbuf
 *
 *  Create a new strbuf that is the larger of that necessary
 *  to hold newdata and minsize.
 *  newdata may be null, in which case minsize is required.
 *  minsize may be zero, in which case newdata is required.
 */

struct strbuf* new_strbuf(char* newdata, int minsize){

    struct strbuf* newbuf;
    int slen = 0;
    int size = minsize;

    if (newdata != NULL) {
        slen = strlen(newdata);
        if (slen > size) {
            size = slen;
        }
    }
    if( (newbuf = malloc(sizeof(struct strbuf) + size + 2)) == NULL ){ 
        perror("malloc failed in new_strbuf");
        exit(1);
    } 
    
    bzero( newbuf, sizeof(struct strbuf) );
    
    if (newdata != NULL) {
        memcpy( newbuf->str, newdata, slen+1 );
        newbuf->length = slen;
    } 
    newbuf->endnull = &(newbuf->str[size+1]);
    newbuf->bufsize = size;
    return newbuf;
}

/* 
 * strbuf_from_file
 *
 *  Read the named file into a strbuf, returning
 *  the entire file in a single strbuf.
 */

struct strbuf* strbuf_from_file(char* filename){

    struct stat sb;
    int fd;
    struct strbuf* fbuf;
   
    if (stat(filename,&sb) != 0){
        perror(filename);
        exit(2);
    }
    if ((fd = open(filename, O_RDONLY, 0) ) < 0){
        perror(filename);
        exit(3);
    }

    fbuf = new_strbuf( NULL, sb.st_size+1 );

    fbuf->length = read(fd, fbuf->str, sb.st_size); 

    (fbuf->str)[sb.st_size] = '\0';

    close(fd);
    return fbuf;
}

/*
 * strbuf_embiggen
 *
 *  Enlarge a strbuf, either in place or by moving to a larger
 *  memory block.
 *  The returned pointer may be the same or different from the 
 *  one given in sbuf.
 *  Do not embiggen a strbuf that is in a list, it may break.
 */

struct strbuf * strbuf_embiggen( struct strbuf* sbuf, int newbufsize ){

    struct strbuf* newbuf;
    int newsize = sizeof(struct strbuf) + newbufsize + 1;
    char err_msg[1024];

    if (( newbuf = realloc( sbuf, newsize )) == NULL ){
        snprintf(err_msg, 1024, 
            "realloc failed from %d to %d", 
             sbuf->bufsize,
             newbufsize); 
        perror(err_msg);
        exit(1);        
    }

    newbuf->bufsize = newbufsize;
    newbuf->endnull = &(newbuf->str[newbufsize]);
    *(newbuf->endnull) = '\0';

    return newbuf;
}


/*
 * strbuf_split
 *
 *  Split one big strbuf into a list of strbufs
 *  delimited by ch.
 *  inbuf will need to be freed by the calling program.
 */

struct strbuf* strbuf_split(struct strbuf* inbuf, char ch){

    int j;
    struct strbuf* outbuf = NULL;
    struct strbuf* tail = NULL;
    bool first = true;
    int start = 0;

    for( j=0; j<inbuf->length; j++){
       if ( inbuf->str[j] == ch ){
       inbuf->str[j] = '\0';

           if (first){
               outbuf = new_strbuf( &(inbuf->str[start]), 0 );
               tail = outbuf;
           first = false;
           }else{
               tail->next = new_strbuf( &(inbuf->str[start]), 0 );
           tail->next->prev = tail;
               tail = tail->next;
           }

       start = j+1;
       }
 
    } 

    return outbuf;
}

/*
 * strbuf_insert_before
 *
 *  Insert newstrbuf in front of the element chain.
 *  If chain is the first element in a list then it becomes the 
 *  new head.  The calling program must know and deal with this.
 */

void strbuf_insert_before(struct strbuf* chain, struct strbuf* newstrbuf){

    struct strbuf* abuf;

    abuf = chain->prev;
    chain->prev = newstrbuf;

    if (abuf != NULL) {
         abuf->next = newstrbuf;
    }
    
    newstrbuf->next = chain;
    newstrbuf->prev = abuf;
   
    return;
}

/*
 * strbuf_insert_after
 *
 *  Insert newstrbuf after the node chain.
 */

void strbuf_insert_after(struct strbuf* chain, struct strbuf* newstrbuf){

    struct strbuf* abuf;

    abuf = chain->next;
    chain->next = newstrbuf;

    if (abuf != NULL) {
        abuf->prev = newstrbuf;
    }

    newstrbuf->next = abuf;
    newstrbuf->prev = chain;

    return;
}

/*
 * strbuf_remove
 *
 *  Remove the node remove, linking the chain of the surrounding nodes,
 *  and setting the removed link pointers to null.
 */

struct strbuf* strbuf_remove( struct strbuf* list, struct strbuf* remove ){

    struct strbuf* ret;

    if (remove == NULL){
        fprintf(stderr, "can not remove NULL\n");
        exit(3);
    }

    if (remove->prev != NULL){
        remove->prev->next = remove->next;
    }

    if (remove->next != NULL){ 
        remove->next->prev = remove->prev;
    }

    if (remove == list){
        ret = remove->next;      
        
    }else{
        ret = list;
    }
    remove->next = NULL;
    remove->prev = NULL;

    return ret;
}

/*
 * strbuf_append
 *
 * Add a node to the end of list.
 */

void strbuf_append( struct strbuf* list, struct strbuf* abuf){
    struct strbuf* runner;

    for(runner=list; runner->next != NULL; runner=runner->next) 
        ;
    if (runner==NULL){ 
        fprintf(stderr,"strbuf_append runner is null\n");
        exit(7);
    } 

    if (runner->next != NULL){
        fprintf(stderr, "strbuf_append runner->next is not null\n");
        exit(8);
    }

    runner->next = abuf;
    abuf->prev = runner; 
   
    return;
}

/*
 * print_chain
 *
 *   Print to stdout all the elements in the list.
 */

void print_chain(struct strbuf* sbuf){

    struct strbuf* abuf;

    for(abuf=sbuf; abuf != NULL; abuf=abuf->next) {
    printf("%s\n", abuf->str);
    }
}


/*
 * strbuf_cmp
 *
 *  Call strcmp on two strbuf->str
 */

int strbuf_cmp(const struct strbuf* s1, const struct strbuf* s2){

    if( (s1 == NULL)||(s2 == NULL) ){ 
        fprintf(stderr, "strbuf_cmp passed a null value v1=%c v2%c\n",
            (s1 == NULL)? 'N' : 'x',  
            (s2 == NULL)? 'N' : '.');  
        exit(1);
    }
    return strcmp(s1->str, s2->str);
}

/*
 * insert_one
 *
 *  Insert one strbuf into a chain of strbuf's in sorted order.
 */
 
struct strbuf* insert_one(struct strbuf* sorted, struct strbuf* newitem){

    struct strbuf* abuf;

    // before the first item
    if( strbuf_cmp( sorted, newitem) > 0 ){
        strbuf_insert_before(sorted, newitem);
        return newitem;
    } 

    for(abuf = sorted; abuf != NULL; abuf = abuf->next){

        if( strbuf_cmp( abuf, newitem ) > 0 ){
            strbuf_insert_before(abuf, newitem); 
            return sorted;
        }
 
        if( abuf->next == NULL ){
            strbuf_insert_after(abuf, newitem);
            return sorted;
        }
    } 

    // should never happen
    fprintf(stderr, "no place to insert %s\n", newitem->str);
    exit(5);
}

/*
 * strbuf_sort
 *
 *  Sort a chain of strbufs.
 *  The sort is just an insertion sort.
 *  It works fine for short lists, < 100 good, < 1000 OK.
 *  Not intended for longer lists.
 */

struct strbuf* strbuf_sort(struct strbuf* unsorted){

    struct strbuf* abuf;
    struct strbuf* sorted;
 
    sorted = unsorted;
    unsorted = unsorted->next;
    sorted->prev = NULL;
    sorted->next = NULL;

    while(unsorted != NULL){

        abuf = unsorted;
        unsorted = unsorted->next;
        
        abuf->next = NULL;
        abuf->prev = NULL;

        sorted = insert_one(sorted, abuf);
    }    

    return sorted;
}

/*
 * strbuf_free_chain
 *
 * Free every node in a strbuf chain.
 */
void strbuf_free_chain(struct strbuf* given_chain){
    struct strbuf* abuf;
    struct strbuf* freethis;

    for( freethis=given_chain; freethis != NULL; ){
        abuf = freethis->next;
        free(freethis);
        freethis = abuf;
    }
    return;
}

#ifdef MAIN

int main(int argc, char* argv[]){

    struct strbuf* newbuf;
    struct strbuf* abuf = NULL;
    struct strbuf* splitbuf = NULL;
    struct strbuf* freethis;


    if( strcmp(argv[1], "testappend") == 0 ){

        char* alphabet = "abcdefghijklmnopqrstuvwxyz";
        newbuf = new_strbuf("0",0);

        int j; 
        for(j=0; j<26; j++){
            abuf = new_strbuf( alphabet+j, 0 );
            strbuf_append(newbuf, abuf);
        } 

        print_chain(newbuf);

        strbuf_free_chain(newbuf);
        exit(0);
    }
    newbuf = strbuf_from_file(argv[1]);

    splitbuf = strbuf_split(newbuf, '\n');
    free(newbuf);

    splitbuf = strbuf_sort(splitbuf);

    print_chain(splitbuf);

    for( freethis=splitbuf; freethis != NULL; ){
        abuf = freethis->next;
        free(freethis);
        freethis = abuf;
    }

    return 0;
}

#endif
