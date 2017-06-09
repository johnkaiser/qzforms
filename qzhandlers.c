
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

xmlHashTablePtr handler_hash = NULL;


/*
 *  init_hanlder_hash
 *
 *  Initialize a hash table to resolve names into function pointers.
 */  

void init_handler_hash(void){

    static struct handler static_handlers[] = 
    {
        {
            .count = 0,
            .name = "timestamp",
            .handler = timestamp
        },
        {
            .count = 0,
            .name = "login",
            .handler = req_login
        },
        {
            .count = 0,
            .name = "logout",
            .handler = logout
        },
        {
            .count = 0,
            .name = "status",
            .handler = qz_status
        },
        {
            .count = 0,
            .name = "fs",
            .handler = qzfs
        },
        {
            .count = 0,
            .name = "onetable",
            .handler = onetable 
        },
        {
            .count = 0,
            .name = "refresh",
            .handler = refresh_form_tag
        },
        {
            .count = 0,
            .name = "menupage",
            .handler = menupage
        },    
        {
            .count = 0,
            .name = "grid",
            .handler = grid 
        },    
        {
            .count = 0,
            .name = NULL,
            .handler = NULL
        }   
     };
    
    handler_hash = xmlHashCreate(31);
    int n;
    for(n=0; static_handlers[n].name != NULL; n++){
        xmlHashAddEntry( handler_hash, static_handlers[n].name,
           (void*) &static_handlers[n]);
    }        
}
