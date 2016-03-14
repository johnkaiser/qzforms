
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

#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

bool debug=false;

struct strnode {
    char* str;
    struct strnode* next;
};

#define ARRAY_SIZE_MAX 100000

bool pg_legal_char(unsigned char ch){

    if (ch < 32) return false;
    if (ch == '"') return false;
    if (ch == '\'') return false;
    if (ch == '{') return false;
    if (ch == '}') return false;
    if (ch == ',') return false;
    if (isspace(ch)) return false;
    return true;
}

/*
 * parse_pg_array
 *
 * {}
 * {,} error
 * {one}
 * { two  }
 * {a,} error
 * {a,,b} error
 * {three,four}
 * {one,"two,bad",   "three}worse"  }
 * {"a}b","c\""} 
 *
 *
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   |state\ch  | {           | legal char | }         | ,          | space      | "       | \       |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | begin    | firstval[1] | error      | error     | error      | no action  | error   | error   |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | firstval | error[7]    | inval[2]   | finish[3] | error[4]   | no action  | inquote | error   |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | findval  | error[7]    | inval[2]   | error[4]  | error[4]   | no action  | inquote | error   |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | inval    | error       | no action  | finish[6] | findval[5] |findcomma[8]| error   | escapev |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | findcomma| error       | error      | finish[9] | findval    | no action  | error   | error   |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | finish   | error       | error      | error     | error      | no action  | error   | error   |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | inquote  | inquote     | inquote    | inquote   | inquote    | inquote    |findcomma| escapeq |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | escapev  | inval       | inval      | inval     | inval      | inval      | inval   | inval   |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *   | escapeq  | inquote     | inquote    | inquote   | inquote    | inquote    | inquote | inquote |
 *   +----------+-------------+------------+-----------+------------+------------+---------+---------+
 *
 *   firstval[1] - first is different from others, it is optional 
 *   inval[2] - set start ptr
 *   finish[3] - empty array
 *   error[4] - empty values not allowed
 *   findval[5] - chg char to \0
 *   finish[6] - chg char to \0
 *   error[7] - no sub arrays, at least not yet
 *   findcomma[8] - chg char to \0
 *   finish[9] - no more vals
 */


char** parse_pg_array(char* pg_ar){

    if (pg_ar == NULL) return NULL;
    if (pg_ar[0] == '\0') return NULL;

    enum {error, begin, firstval, findval, inval, inquote, escapev, 
        escapeq, findcomma, finish} searchstate;

    char* ch;
    char* errormsg = NULL;
    int arsize = strlen(pg_ar);
    searchstate = begin;

    struct strnode* newnode;
    struct strnode* topnode;

    struct strnode base;
    base.str = NULL;
    base.next = NULL;

    topnode = &base;

    char* ar;
    asprintf(&ar, "%s", pg_ar);

    if (debug)printf("ar=[%s]\n",ar);

    for(ch = ar; *ch != '\0'; ch++){
        if (debug)printf("    ch=%c\n",*ch);

        switch(searchstate){

            case begin:

                if (*ch=='{'){
                    if(debug)printf("searchstate>%s\n", "firstval");
                    searchstate = firstval;

                }else if (!isspace(*ch)){
                    searchstate = error;
                    errormsg = "bad char in state begin";
                }
                break;

            case firstval:

                if (*ch == '{'){
                    errormsg = "nested sets not supported";
                    searchstate = error;

                }else if (*ch == '"'){
                     
                    newnode = malloc(sizeof(struct strnode));
                    newnode->str = ch+1;   // the next char starts it
                    newnode->next = NULL;
                    topnode->next = newnode;
                    topnode = newnode;

                    if (debug)printf("searchstate>%s\n", "inquote");
                    searchstate = inquote;

                }else if (pg_legal_char(*ch)){

                    newnode = malloc(sizeof(struct strnode));
                    newnode->str = ch;
                    newnode->next = NULL;
                    topnode->next = newnode;
                    topnode = newnode;

                    if (debug)printf("searchstate>%s\n", "inval");
                    searchstate = inval;

                }else if (*ch == '}'){
                    // An empty array 
                    if (debug)printf("searchstate>%s\n", "finish");
                    searchstate = finish;

                }else if (*ch == ','){
                    // This is an empty array element {a,,a} 
                    errormsg = "empty array element";
                    searchstate = error;  

                }else if (!isspace(*ch)){
                    errormsg = "unexpected branch encountered in parsing";
                    searchstate = error;
                }
                break;

            case findval:

                if (*ch == '{'){
                    errormsg = "nested sets not supported";
                    searchstate = error;

                }else if (*ch == '"'){
                     
                    newnode = malloc(sizeof(struct strnode));
                    newnode->str = ch+1;   // the next char starts it
                    newnode->next = NULL;
                    topnode->next = newnode;
                    topnode = newnode;

                    if (debug)printf("searchstate>%s\n", "inquote");
                    searchstate = inquote;

                }else if (pg_legal_char(*ch)){

                    newnode = malloc(sizeof(struct strnode));
                    newnode->str = ch;
                    newnode->next = NULL;
                    topnode->next = newnode;
                    topnode = newnode;

                    if (debug)printf("searchstate>%s\n", "inval");
                    searchstate = inval;

                }else if ((*ch == '}') || (*ch == ',')){
                    errormsg = "empty array element";
                    searchstate = error;

                }else if (!isspace(*ch)){
                    errormsg = "unexpected branch encountered in parsing";
                    searchstate = error;
                }
                break;

            case inval:

                if (*ch == '{'){
                    errormsg = "unquoted value can not be opening brace";
                    searchstate = error;
                }else if (*ch == '}'){
                    *ch = '\0';
                    if(debug)printf("searchstate>%s\n", "finish");
                    searchstate = finish;

                }else if (*ch == ','){
                    *ch = '\0';
                    if(debug)printf("searchstate>%s\n", "findval");
                    searchstate = findval;

                }else if (isspace(*ch)){
                    *ch = '\0';
                    if(debug)printf("searchstate>%s\n", "findcomma");
                    searchstate = findcomma;
                }
                break;

            case inquote:
                if (*ch == '\\'){
                    if(debug)printf("searchstate>%s\n", "escapeq");
                    searchstate = escapeq;
                }else if (*ch == '"'){
                    *ch = '\0';
                    if(debug)printf("searchstate>%s\n", "findcomma");
                    searchstate = findcomma;
                }
                break;

            case escapev:
                if(debug)printf("searchstate>%s\n", "inval");
                searchstate = inval;
                break;
                
            case escapeq:
                if(debug)printf("searchstate>%s\n", "quote");
                searchstate = inquote;
                break;
                
            case findcomma:

                if (*ch == ','){
                    if(debug)printf("searchstate>%s\n", "findval");
                    searchstate = findval;

                }else if (*ch == '}'){
                    if(debug)printf("searchstate>%s\n", "finish");
                    searchstate = finish;

                }else if (!isspace(*ch)){
                    errormsg = "unexpected char";
                    searchstate = error;
                }
                break;

            case finish:

                if (!isspace(*ch)){
                    errormsg = "data after end";
                    searchstate = error;
                }
                break;

            case error:
                break;

        } // switch
    } // end for ch

    if (searchstate == error){
        if(debug)printf("Error %s\n", errormsg);
        if(!debug)fprintf(stderr, "Error %s\n", errormsg);
        free(ar);
        return NULL;
    }

    long ptrcnt = 2;
    struct strnode* noderunner;

    for (noderunner = base.next; 
         noderunner!=NULL; 
         noderunner=noderunner->next){
            if (debug)printf("node-%s\n", noderunner->str);
            ptrcnt++;
    }
    
    long datasize = sizeof(char*)*(ptrcnt+2) + arsize + 2;

    if (datasize > INT_MAX){ 
        fprintf(stderr, "array too large");
        free(ar);
        return NULL;
    }

    char** outbuf = malloc(datasize);
    if (outbuf == NULL){
        fprintf(stderr,"malloc failed");
        free(ar);
        return NULL;
    }
    bzero(outbuf,datasize);
    
    int n=0;
    void* tmp;  // cramming two different types in one buffer
    char* st = tmp = &(outbuf[ptrcnt+1]) ;

    for (noderunner = base.next; noderunner!=NULL; ){
        int strsize;

        if (noderunner->str == NULL){
            strsize = 0;
            *st = '\0'; 
            outbuf[n] = st;   
        }else{
            strsize = strlen(noderunner->str);
            strncpy(st, noderunner->str, strsize+1); 
            outbuf[n] = st;   
        }
        outbuf[n+1] = NULL;
        if (debug)printf("added %d [%s]\n", n, st);
        n++;    
        st += strsize+1;

        struct strnode* free_this = noderunner;
        noderunner = noderunner->next;
        free(free_this);
    } 
    
    free(ar);
    return outbuf;
}

char* str_ar_to_json(char* ar[]){

    int size = 4; // "[]\0\0"
    int n;

    char* st;
    int st_len;
    char* json_ar;
    if (ar == NULL) return NULL;

    for (n=0; ar[n]!=NULL; n++){
        size += strlen(ar[n]) + 4; // "",space
    }

    st = json_ar = malloc(size);
    bzero(json_ar, size);

    st[0] = '[';
    st++;

    for (n=0; ar[n]!=NULL; n++){
       if (n!=0) {
           memcpy(st, ", ", 2);
           st+=2;
       }
       st[0] = '"';
       st++;
       st_len = strlen(ar[n]);
       memcpy(st, ar[n], st_len);
       st += st_len;
       st[0] = '"';
       st++;
    }
    st[0] = ']';

    return json_ar;
}
/*
 *  pg_ar_to_json
 *
 */
char* pg_ar_to_json(char* pg_ar){
    char** str_ar =  parse_pg_array(pg_ar);
    char* json_ar = str_ar_to_json(str_ar);
    free(str_ar);
    return json_ar;
}

#ifdef PARSE_PG_ARRAY_MAIN

#include <sys/time.h>

double gettime(void){
    struct timeval tp;
    double t;

    gettimeofday(&tp,NULL);
    t = tp.tv_sec;

    t+= ((double)tp.tv_usec)*0.000001;

    return t;
}

void test_parse( char* arraystr, char* ar_el[]){
    char** out;
    char* json_ar;
    double start = gettime();

    if (arraystr != NULL) printf("%s\n", arraystr);
    out = parse_pg_array(arraystr);
    if (out == NULL){
        printf("parse is NULL\n");
    }else{
        int n;
        for (n=0; out[n]!=NULL; n++){
           fflush(stdout);
           printf("[%s][%s] %s\n", out[n],ar_el[n],
             (strcmp(out[n],ar_el[n])== 0) ? "OK" : "ERR");
        }

        json_ar = pg_ar_to_json(arraystr);
        printf("json_ar=%s\n", json_ar);
        free(json_ar);

    }
    free(out);
    printf("time=%f\n", gettime() - start);
    printf("=======================================\n");
}

int main(int argc, char* argv[]){

    // debug = true;

    char* answer0[] = {NULL};
    test_parse( NULL, answer0 );

    char* answer1[] = {NULL};
    test_parse( "{}", answer1 );

    char* answer2[] = {NULL};
    test_parse( "{    }", answer2 );

    char* answer3[] =  {"one",NULL};
    test_parse( "{ one }", answer3 );

    char* answer4[] =  {"a","",NULL};
    test_parse( "{a,}", answer4 );

    char* answer5[] =  {"one","two",NULL}; 
    test_parse( "{ one, two }", answer5 );
    
    char* answer6[] =  {"alpha","","gamma",NULL}; 
    test_parse( "{ alpha,,gamma}", answer6 );

    char* answer7[] = {"one","two","three","four","five","six",NULL};
    test_parse( "{one,two,three,four,five,six}", answer7);

    char* answer8[] = {"one","{beta,gamma}","two",NULL};
    test_parse( "{one, {beta,gamma}, two}", answer8);

    char* answer9[] = {"alpha","beta",NULL};
    test_parse("{alpha,beta}x",answer9);

    char* answer10[] = {"alpha","beta",NULL};
    test_parse("{alpha,beta}",answer10);


    char* answer11[] = {"alpha","beta",NULL};
    test_parse("{alpha,\"beta\"}",answer11);

    char* answer12[] = {"a","b","c d","e", NULL};
    test_parse("{a,b,\"c d\",e}", answer12);

    char* answer13[] = {"a}b","a\\b",NULL};
    test_parse("{\"a}b\",\"a\\b\"}", answer13);

    char* answer14[] = {"c\\\"d","c\\\"d"};
    test_parse("{\"c\\\"d\" ,\"c\\\"d\"}", answer14);


    char* answer15[] = {"a","","b"};
    test_parse( "{a,\"\",b}", answer15);

    char* answer16[] = {};
    test_parse( "", answer16);
    
    char* json_str = pg_ar_to_json(NULL);
    printf("pg_ar_to_json(NULL)=%s\n", json_str);
    free(json_str);

    return 0;
}

#endif

