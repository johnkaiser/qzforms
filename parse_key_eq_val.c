
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

#include "qz.h"
#include <ctype.h>

#define DEBUG_LOG "parse_key_eq_val.log"

static bool DEBUG = false;

bool percent_unescape(char* buf){
    char* from=buf;
    char* to=buf;
    char conv_buf[4] = {'\0','\0','\0','\0'};
    enum {normal, has_percent, has_first_digit, conv_error} conv_state = normal;

    FILE* db_log = NULL;
    if (DEBUG) db_log = fopen(DEBUG_LOG, "a");


    if (buf == NULL) return false;
    if (buf[0] == '\0') return true;

    for(from=buf; *from != '\0'; from++){

        switch(conv_state){
            case normal:
                if ( *from == '%' ){
                     conv_state = has_percent;
                }else{
                     // convert + to space
                     if ( *from == '+' ){
                        *to++ = ' ';
                     }else{
                        *to++ = *from;
                     }
                }
                break;

            case has_percent:
                if ( isxdigit(*from) ){
                     conv_buf[0] = *from;
                     conv_state = has_first_digit;
                }else{
                     conv_state = conv_error;
                     if(DEBUG) fprintf(db_log, "non hex digit after percent\n");
                }
                break;

            case has_first_digit:
                if ( isxdigit(*from) ){
                     conv_buf[1] = *from;
                     *to++ = (char) strtol(conv_buf, NULL, 16);
                     conv_state = normal;
                }else{
                     conv_state = conv_error;
                     if(DEBUG) fprintf(db_log, "non hex digit after percent\n");
                }
                break;

            case conv_error:
                break;

        } // switch
        if ( conv_state == conv_error ){
            if (DEBUG){ fclose(db_log); db_log = NULL;};
            return false;
        }    
    }

    // When conv_buf has no % codes, this will rewrite then existing
    // ending \0 with \0, otherwise it will shorten the string.
    *to++ = '\0';

    if (DEBUG){ fclose(db_log); db_log = NULL;};
    return true;
}

void percent_unescape_vals(void* val, void* ignore, const xmlChar* key){
    percent_unescape(val);
}

void check_utf8(void* val, void* data, const xmlChar* key){
    bool* is_valid_utf8 = data;
    
    if ( !xmlCheckUTF8(key) || !xmlCheckUTF8(val) ){
        *is_valid_utf8 = false;
    }

}

/*
 * A legal char according to rfc3986 are the reserved chars
 * alpha digit - . _ ~
 * plus + for a space
 * plus % for encoding
 * The * is not, except firefox sends it unencoded.
 */
bool is_legal_char(char ch){
    if ( isalnum(ch) ) return true;
    if ( ch == '-' ) return true;
    if ( ch == '.' ) return true;
    if ( ch == '_' ) return true;
    if ( ch == '~' ) return true;
    if ( ch == '+' ) return true;
    if ( ch == '%' ) return true;
    if ( ch == '*' ) return true;
    return false;
}


/*
 * parse_key_eq_val
 *
 *  Table of next states for present state and input character
 *
 *  +---------+------------+------------+------------+--------+
 *  |state\ch | =          | fieldsep   | legal char | space  |
 *  +---------+------------+------------+------------+--------+
 *  | inkey   | nextval[1] | error      | no action  | error  | 
 *  +---------+------------+------------+------------+--------+
 *  | inval   | error      | nextkey[2] | no action  | error  | 
 *  +---------+------------+------------+------------+--------+
 *  | nextkey | error      | error      | inkey[4]   | ignore |
 *  +---------+------------+------------+------------+--------+
 *  | nextval | error      | nextkey[3] | inval[5]   | ignore |
 *  +---------+------------+------------+------------+--------+
 *
 *  nextval[1] - std case, ch to null 
 *  nextkey[2] - std case, ch to null, state to nextkey
 *  nextkey[3] - value is null, ch to null, save previous key to a null record
 *  inkey[4]   - std case, save start of key, 
 *  inval[5]   - std case, start of new val, save key and val pointers
 *  Not =,&, or legal char is an error
 */
xmlHashTablePtr parse_key_eq_val(struct handler_args* hargs, char* kvstr, 
    char fieldsep, bool unescape){

    char* akey = NULL;
    char* ch;
    enum {nextkey, inkey, nextval, inval, parse_error} searchstate;
    searchstate = nextkey;
    static const char* blank = "\0\0\0"; 
    FILE* db_log = NULL;
    if (DEBUG) db_log = fopen(DEBUG_LOG, "a");

    /* 50 is just a random guess */
    // XXXXX Add to config file.
    xmlHashTablePtr pt = xmlHashCreate(50);

    for ( ch = kvstr; *ch != '\0'; ch++ ){

        switch(searchstate){

            case nextkey:
                if ( is_legal_char(*ch) ){
                    akey = ch;
                    searchstate = inkey;
                }else if ( !isspace(*ch) ){
                    searchstate = parse_error;

                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d fail %s %d\n", 
                        gettime(), hargs->request_id, __func__, __LINE__,
                        "not a legal char for nextkey dec", *ch);
                    pthread_mutex_unlock(&log_mutex);

                } // else leading space - ignore
              break;

            case inkey:
                if ( *ch == '=' ){
                    *ch = '\0';
                    searchstate = nextval;
                }else if ( !is_legal_char(*ch) ){
                    searchstate = parse_error;

                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d fail %s %d\n", 
                        gettime(), hargs->request_id, __func__, __LINE__,
                        "not a legal char for nextkey dec", *ch);
                    pthread_mutex_unlock(&log_mutex);
                }
              break;

            case nextval:
                if ( is_legal_char( *ch ) ){
                    char* aval = ch;
                    /* s                        tore data here */
                    if(unescape){
                        percent_unescape(akey);
                    }
                    searchstate = inval;                          

                    if (xmlHashLookup(pt, akey) != NULL){
                        
                        searchstate = parse_error;

                        pthread_mutex_lock(&log_mutex);
                        fprintf(hargs->log, "%f %d %s:%d fail %s [%s]\n", 
                            gettime(), hargs->request_id, __func__, __LINE__,
                            "duplicate key in data", akey);
                        pthread_mutex_unlock(&log_mutex);
                    }else{     
                        if ( xmlHashAddEntry( pt, akey, aval ) == -1 ){
                            searchstate = parse_error;
    
                            pthread_mutex_lock(&log_mutex);
                            fprintf(hargs->log, "%f %d %s:%d fail %s [%s] %s [%s]\n", 
                                gettime(), hargs->request_id, __func__, __LINE__,
                                "xmlHashAddEntry failed on key", akey,
                                "val", aval);
                            pthread_mutex_unlock(&log_mutex);
                        }
                    }    
                }else if ( *ch == fieldsep ){
                    *ch = '\0';
                    /* store a null value */
                    if (unescape){
                        percent_unescape(akey);
                    }
                    searchstate = nextkey;
                    if (xmlHashAddEntry( pt, akey,(void*) blank ) == -1){
                        searchstate = parse_error;

                        pthread_mutex_lock(&log_mutex);
                        fprintf(hargs->log, "%f %d %s:%d fail %s [%s]\n", 
                            gettime(), hargs->request_id, __func__, __LINE__,
                            "xmlHashAddEntry failed on key", akey);
                        pthread_mutex_unlock(&log_mutex);
                    }    
                }else if ( !isspace(*ch) ){
                    searchstate = parse_error;

                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d fail %s [%c]\n", 
                        gettime(), hargs->request_id, __func__, __LINE__,
                        "unexpected char", *ch);
                    pthread_mutex_unlock(&log_mutex);
                } // else leading space - ignore
                break;

            case inval:
                if ( *ch == fieldsep ){
                    *ch = '\0';
                    searchstate = nextkey;
                }else if ( !is_legal_char(*ch) ){
                    searchstate = parse_error;

                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d fail %s %d\n", 
                        gettime(), hargs->request_id, __func__, __LINE__,
                        "not a legal char for inval dec",*ch);
                    pthread_mutex_unlock(&log_mutex);
                }
                break;

            case parse_error:
              break;

        } // switch
    } // for ch


    if ( searchstate == nextval ){
        // means "x=a,y=b,z=' z has no value and must be set
        if (unescape) percent_unescape(akey);

        if (xmlHashAddEntry(pt, akey, (void*) blank) == -1){
            searchstate = parse_error;

            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, "%f %d %s:%d fail %s [%s]\n", 
                gettime(), hargs->request_id, __func__, __LINE__,
                "xmlHashAddEntry failed on key", akey);
            pthread_mutex_unlock(&log_mutex);
        }    
    }

    if ( searchstate == parse_error ){
        xmlHashFree(pt,NULL);
        pt = NULL;
        return NULL;
    }

    if (unescape){
        xmlHashScan(pt, (void*) percent_unescape_vals, NULL);
    }

    // Validate the utf-8 encoding
    bool is_valid_utf8 = true;
    xmlHashScan(pt, (void*) check_utf8, &is_valid_utf8);

    if (!is_valid_utf8){

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d fail invalid utf-8\n", 
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        error_page(hargs, SC_BAD_REQUEST, 
            "Post data contains invalid UTF-8 data");
        
        xmlHashFree(pt, NULL);
        pt = NULL;
    }
    
    return pt;
}

#ifdef PARSE_KEY_EQ_VAL_MAIN

pthread_mutex_t log_mutex;

void error_page( struct handler_args* hargs, int status_code, const char* msg ){
    printf("error_page status=%d msg=%s\n", status_code, msg);
}
void ht_scanner(void* val, void* ignore, const xmlChar* key){
    printf( "key=[%s], val=[%s]\n", key, (char*) val );
}
int main(int argc, char* argv[]){

    DEBUG = true;
    pthread_mutex_init(&log_mutex,NULL);

    struct handler_args shargs = {
        .log = stderr,
        .request_id = 0
    }; 
    char fieldsep = '&';
    // invalid utf8 data:
    //char* somedata = "name=Xavier+Xantico&verdict=Yes&colour=Blue&happy=sad&Utf%F6r=Send";
    char* somedata = "name=Xavier+Xantico&verdict=Yes&colour=Blue&happy=sad&action=Send";
    char* testdata;
    xmlHashTablePtr ht = xmlHashCreate(50);

    if (argc == 2){
        struct stat sb;
        stat(argv[1], &sb);
        char* buf;
        buf = malloc(sizeof(sb.st_size));
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0){
            perror("argv[1]");
            exit(17);
        }    
        if (read(fd, buf, sb.st_size) == 0){
            printf("unable to read file %s\n", buf) ;
            exit(18);
        }    
        somedata = buf;
    }

       printf("testing [%s]\n", somedata);
       int datasize = strlen(somedata);
       testdata = calloc(1, datasize + 2 );
       strlcpy(testdata, somedata, datasize + 1);

       ht = parse_key_eq_val(&shargs, testdata, fieldsep, true);
       xmlHashScan(ht, ht_scanner, NULL);
       free(testdata);


    printf("\npercent_unescape test:\n");
    char* test_data =  "percent_unescape_exc-%21_hash-%23_dol-%24_amp-%26_"
        "qu-%27_op-%28_cp-%29_ast-%2A_plus-%2B_com-%2C_sl-%2F_col-%3A_sco-%3B_"
        "eq-%3D_ques-%3F_at-%40_obkt-%5B_cbkt-%5D_sp-+XXX";

    char* test_buf;
    asprintf(&test_buf, "%s", test_data);
    printf("test_data=|%s|\nstrlen=%lu\n", test_buf, strlen(test_buf));
    percent_unescape(test_buf);
    printf("unescaped=|%s|\nstrlen=%lu\n", test_buf, strlen(test_buf));

    free(test_buf);

    printf("\npercent_unescape no percents\n");
    asprintf(&test_buf, "ABCDEFG");
    test_buf[7]='X';

    printf("test_data=|%s|\n", test_buf );
    percent_unescape(test_buf);
    printf("unescaped=|%s|\nstrlen=%lu\n", test_buf, strlen(test_buf));

    return 0;
}
#endif
