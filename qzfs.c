
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
 *  qzfs
 *
 *  Serve up a file from pg.
 *  The file must be stored in a single attribute and
 *  must be some form of text.
 */
void qzfs(struct handler_args* h){

  
    //  The URL will be /qz/which_table/get/which_name
    //                  /0 /1          /2  /3
    //  which_table is a form_name on a table_action.
    //  "get" is the action in table_action.  
    //  which_name is in the primary key for the table. 
    //  This is a read only fs, use a form to update.
    //  table_action(which_table, get) must exist.
    //  table_action(which_table, etag_value) must exist,
    //  but may be "select 0 as etag".

    //  The following fields are used from the saved select
    //    mime_type - a string like "text/html"
    //    etag - a string of an unsigned 64 bit int 
    //    data - the contents being served

    //  The table_action will be "text" and/or "etag_value"
    //  based on incoming headers.  Both should be working.

    char* which_table = get_uri_part(h, QZ_URI_FORM_NAME);
    if (which_table == NULL){
       error_page(h, 404, "Not on file"); 
       return;
    }

    char* which_name = get_uri_part(h, QZ_URI_REQUEST_DATA);
    if (which_name == NULL){
       error_page(h, 404, "Not on file"); 
       return;
    }

    // This must come before cache validation to catch invalid names.
    struct table_action* ta = open_table(h,  which_table, "get");
    if (ta == NULL){
       error_page(h, 404, "Not on file"); 
       return;
    }

    // But wait, maybe the cache is valid.
    uint64_t payload = 0;
    char* http_if_none_match =
        FCGX_GetParam("HTTP_IF_NONE_MATCH", h->envpfcgi);

    if (http_if_none_match != NULL){

       // payload from the client is compared to the result of a table_action
        payload = validate_etag(h->session->tagger_socket_path, 
            http_if_none_match);

        fprintf(h->log, "%f %d %s:%d etag name %s payload %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            which_name, (payload > 0)  ? "OK" : "fail"  );

        if (payload > 0){
            struct table_action* etag_value_ta;
            etag_value_ta = open_table(h,  get_uri_part(h, 1), "etag_value");

            char* etag_params[2];
            etag_params[0] = which_name;
            etag_params[1] = NULL;
            PGresult* etag_rs = perform_action(h, etag_value_ta, etag_params);
            if (PQresultStatus(etag_rs) == PGRES_TUPLES_OK){ 

                // XXXXX possibly check the pg type
                char* pg_etag_str = PQgetvalue(etag_rs,0,0);
                uint64_t pg_etag_int = strtoll(pg_etag_str, NULL, 10);
                
                if ((pg_etag_int > 0) && (pg_etag_int == payload)){

                    // OK then, the cache is current
                    fprintf(h->log, "%f %d %s:%d etag validated for %s\n",
                        gettime(), h->request_id, __func__, __LINE__,
                        which_name);

                    error_page(h, SC_NOT_MODIFIED, NULL);
                    return;
                }
            }else{
                // Something other than PGRES_TUPLES_OK, like something bad.
                // Not sure if this should cause an error page, but maybe
                // a new page can be generated.

                char* error_msg = nlfree_error_msg(etag_rs);
                fprintf(h->log, "%f %d %s:%d fail etag_value %s,%s \n", 
                    gettime(), h->request_id, __func__, __LINE__, 
                    PQresStatus( PQresultStatus(etag_rs) ),
                    error_msg);
                
                free(error_msg);
            }
        }
    }
    // So the cache thing didn't work out.

    char* paramdata[2];
    char* a_name;
    asprintf(&a_name, "%s", which_name);
    paramdata[0] = a_name;
    paramdata[1] = NULL;

    PGresult* rs = perform_action(h, ta, paramdata);
    if ((PQresultStatus(rs) != PGRES_TUPLES_OK) || (PQntuples(rs) != 1)){
       error_page(h, 404, "Not on file"); 
       return;
    }

    char* mimetype = PQgetvalue(rs, 0, PQfnumber(rs, "mimetype"));
    if ((mimetype != NULL) && strlen(mimetype)>0){
       content_type(h, mimetype);
    }

    // Set the outgoing etag header.
    char* etag_str = PQgetvalue(rs, 0, PQfnumber(rs, "etag"));
    uint64_t etag_val=0;
    if ((etag_str != NULL) && (strlen(etag_str)>0)){
        etag_val = strtoull(etag_str, NULL, 10);
        if (etag_val > 0){
            // etag_header calls make_etag
            etag_header(h, etag_val);   
        }
    }

    // It.
    h->data = new_strbuf( PQgetvalue(rs, 0, PQfnumber(rs, "data")),0); 

    fprintf(h->log, "%f %d %s:%d fs serve output %s pg size=%d\n",
        gettime(), h->request_id, __func__, __LINE__, 
        which_name, PQgetlength(rs, 0, PQfnumber(rs, "data")));


    free(a_name);
    a_name = NULL;
    PQclear(rs);
    return;
}
