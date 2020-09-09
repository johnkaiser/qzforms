
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
#define QZ_COOKIE_MAX_EXPIRE_LENGTH 64


/*
 *  make_cookie
 *
 *  Turn function parameters into a cookie request.
 */

void make_cookie(struct handler_args* h,
                           char* name, 
                           char* value, 
                           char* path, 
                           char* domain,
                           int lifetime_seconds, 
                           bool secure, 
                           bool http_only){

    char* cookie = NULL;

    char expires[QZ_COOKIE_MAX_EXPIRE_LENGTH]; 
    expires[0] = '\0';

    if (lifetime_seconds>0){  
        time_t expires_t = time(NULL) + lifetime_seconds;
        char* fmt="Expires=%a, %d-%b-%Y %T %Z; "; 
        struct tm* now_tm = gmtime(&expires_t); 

        strftime(expires, QZ_COOKIE_MAX_EXPIRE_LENGTH,fmt,now_tm);
    }

    //                 value;   exp; domain;  path; secure; samsite http
    asprintf(&cookie, "%s=%s; " "%s" "%s%s%s" "%s%s%s" "%s" "%s" "%s"
    ,
        name,
        (value != NULL) ? value:"",
        expires,
        (domain != NULL) ? "Domain=":"",
        (domain != NULL) ? domain:"",
        (domain != NULL) ? "; ":"",
        (path != NULL) ? "Path=":"",
        (path != NULL) ? path:"",
        (path != NULL) ? "; ":"",
        (secure) ? "Secure; ":"",
        (secure) ? "SameSite=Strict; ":"",
        (http_only) ? "HttpOnly; ":""
        );

    add_header(h, "Set-Cookie", cookie);

    if (h->conf->log_cookie_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d Set-Cookie: %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            cookie);
        pthread_mutex_unlock(&log_mutex);
    } 
    free(cookie);
}     

/*
 *  parse_cookie
 *
 *  Setup the call to the parser.
 *  Turn the string of text cookies into a hash table.
 */
void parse_cookie(struct handler_args* hargs, char* ck){

    int buflen = strlen(ck) + 2;
    hargs->cookie_buf = calloc(1, buflen);
    memcpy(hargs->cookie_buf, ck, buflen-2);

    hargs->cookiesin = parse_key_eq_val(hargs, hargs->cookie_buf, ';', false);

    return;
}

