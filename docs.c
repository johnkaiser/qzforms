
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


void init_doc(struct handler_args* h){

    char fetch_doc_query[] = 
        "SELECT div_id, "
        "'<div>' || data || '</div>' \"data\", "
        "el_class "
        "FROM qz.doc "
        "WHERE form_name = $1 "
        "AND action = $2 "
        "ORDER BY action ";

    PGresult* rs = PQprepare(h->session->conn, "fetch_doc",
        (const char*) fetch_doc_query, 0, NULL);

    char* error_msg = nlfree_error_msg(rs);
    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d %s %s %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        "fetch_doc_query",
        has_data(error_msg) ? "fail": "",
        PQresStatus(PQresultStatus(rs)), error_msg);

    pthread_mutex_unlock(&log_mutex);
    free(error_msg);
    error_msg = NULL;
    PQclear(rs);
}

void doc_adder(struct handler_args* h){

    char* params[3];
    params[0] = get_uri_part(h, QZ_URI_FORM_NAME);
    params[1] = get_uri_part(h, QZ_URI_ACTION);
    params[2] = NULL;
    static char view[] = "view";

    if ( ! has_data(params[1]) && strcmp("menupage", h->handler_name)==0){
        params[1] = view;
    }

    PGresult* doc_adder_rs = PQexecPrepared(h->session->conn,
        "fetch_doc", 2, (const char * const *) params, NULL, NULL, 0);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d %s/%s %s %d tuples\n",
        gettime(), h->request_id, __func__, __LINE__,
        params[0], params[1], PQresStatus(PQresultStatus(doc_adder_rs)),
        PQntuples(doc_adder_rs));
    pthread_mutex_unlock(&log_mutex);

    if (PQresultStatus(doc_adder_rs) == PGRES_TUPLES_OK){
        if (PQntuples(doc_adder_rs) > 0){
            int nt;
            for (nt=0; nt < PQntuples(doc_adder_rs); nt++){
                 xmlNodePtr add_where = NULL;
                 xmlNodePtr newnode = NULL;

                 char* data = get_value(doc_adder_rs, nt, "data");
                 char* node_name = get_value(doc_adder_rs, nt, "div_id");
                 char* el_class = get_value(doc_adder_rs, nt, "el_class");

                 int data_len = strlen(data);
                 add_where = qzGetElementByID(h, node_name);

                 if ((data_len > 0) && (add_where != NULL)){

                     /*
                      *  This is a bit of notable black magic.
                      *  The context variable is not referenced directly.
                      */
                     xmlParseInNodeContext(add_where, data, strlen(data),
                         XML_PARSE_NOERROR|XML_PARSE_NOWARNING|XML_PARSE_NONET,
                         &newnode);
                         
                     xmlErrorPtr xerr = xmlCtxtGetLastError(h->ctx);
                     if (xerr != NULL){ 
                         pthread_mutex_lock(&log_mutex);
                         fprintf(h->log, "%f %d %s:%d xml parse error %s\n",
                             gettime(), h->request_id, __func__, __LINE__,
                             xerr->message);
                         pthread_mutex_unlock(&log_mutex);
                     }
                     if (has_data(el_class)){
                         xmlNewProp(newnode, "class", el_class);
                     }
                     if (newnode != NULL){
                         xmlAddChild(add_where, newnode);
                     }
                     
                 }else{
                     if (h->conf->log_doc_details){
                         pthread_mutex_lock(&log_mutex);
                         fprintf(h->log, "%f %d %s:%d doc not added %s node %s %s\n",
                             gettime(), h->request_id, __func__, __LINE__,
                             (data_len > 0) ? "has data":"no data",
                             node_name,
                             (add_where != NULL) ? "found":"not found"
                             );
                         pthread_mutex_unlock(&log_mutex);
                     }
                 }
            }
        }
    }else{
        char* error_msg = nlfree_error_msg(doc_adder_rs);
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d %s %s %s %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            "doc_adder", has_data(error_msg) ? "fail":"",
            PQresStatus(PQresultStatus(doc_adder_rs)), error_msg);
        pthread_mutex_unlock(&log_mutex);
        free(error_msg);
        error_msg = NULL;

   }
   PQclear(doc_adder_rs);
}
