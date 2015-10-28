
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

/*
 *  Turn a file name into an xml node tree,
 *  The handler is required for logging, but may be null
 *  for use without logging.
 */  
xmlDocPtr doc_from_file( struct handler_args* h, char* requested_docname ){

    fprintf(h->log, "%f %d %s:%d begin doc_from_file %s\n", 
        gettime(), h->request_id, __func__, __LINE__,
        requested_docname); 

    if (h->doc != NULL){
        fprintf(h->log, "%f %d %s:%d danger %s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            "doc_from_file called with h->doc defined");
    }
    xmlDocPtr doc;

    char* docname; 
    static char default_docname[] = "base.xml";
    char* full_path;

    if ((requested_docname == NULL) || (strlen(requested_docname)==0)){
        docname = default_docname;
    }else{    
        docname = requested_docname;
    }

    asprintf(&full_path, "%s%s%s", 
        h->conf->template_path, PATH_SEPARATOR, docname);

    doc = xmlParseFile(full_path);

    if (doc == NULL){
        fprintf(h->log, "%f %d %s:%d xmlParseFile failed %s\n", 
            gettime(), h->request_id, __func__, __LINE__, full_path);
       
        free(full_path);
        full_path = NULL;
        return NULL;
    }

    free(full_path);
    full_path = NULL;

    add_jscss_links(h, doc);

    fprintf(h->log, "%f %d %s:%d doc_from_file complete\n", 
        gettime(), h->request_id, __func__, __LINE__); 
    
    return doc;
}


