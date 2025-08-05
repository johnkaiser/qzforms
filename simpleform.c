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

void simpleform_view(struct handler_args* h, char* form_name, xmlNodePtr divqz){

    struct table_action* view_ta = open_table(h, form_name, "view");

    if (view_ta == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log,"%f %d open_table %s, view) failed\n",
            gettime(), h->request_id, form_name);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_BAD_REQUEST, "form_name not found");
        return;
    }

    PGresult* view_rs = perform_post_action(h, view_ta);

    if (view_rs == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s %d fail action returned null\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_BAD_REQUEST, "action returned null");
        return;
    }

 
    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s %d perform action returned %d rows\n",
        gettime(), h->request_id, __func__, __LINE__, PQntuples(view_rs));
    pthread_mutex_unlock(&log_mutex);

    // not found.
    if (PQntuples(view_rs) == 0){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s %d Record not found.\n",
            gettime(), h->request_id, __func__, __LINE__ );
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_NOT_FOUND, "Record not found.");
        PQclear(view_rs);
        return;
    }
    // Should be exactly 1 so that PQgetvalue can be called for row 0.
    if (PQntuples(view_rs) != 1){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s %d Wrong data count %d should be 1.\n",
            gettime(), h->request_id, __func__, __LINE__,
            PQntuples(view_rs));
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_BAD_REQUEST, "Wrong data count");
        PQclear(view_rs);
        return;
    }

    // If table_action action is not defined, then open_table will
    // return null, then edit_form will leave the submit button off
    // the form.
    static char* ACTION = "action";
    struct table_action* action_ta = open_table(h, form_name, ACTION);

    char* next_action = NULL;
    if (action_ta != NULL){
        next_action = ACTION;
    }    

    edit_form(h, next_action, view_ta, view_rs, form_name, divqz);

    PQclear(view_rs);
    
    return;
}

void simpleform_action(struct handler_args* h, char* form_name, xmlNodePtr divqz){

    struct table_action* action_ta = open_table(h, form_name, "action");

    if (action_ta == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d update_ta is null from %s, %s\n",
            gettime(), h->request_id,  __func__, __LINE__,
            form_name, "action");
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_NOT_FOUND, "Not Found");
        return;
    }

    // This is it right here.
    PGresult* action_rs = perform_post_action(h, action_ta);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d table action returned %s %s\n",
        gettime(), h->request_id,  __func__, __LINE__,
        PQresStatus(PQresultStatus(action_rs)),
        PQcmdStatus(action_rs));
    pthread_mutex_unlock(&log_mutex);

    if ( (PQresultStatus(action_rs) == PGRES_TUPLES_OK)
         ||
         (PQresultStatus(action_rs) == PGRES_COMMAND_OK) ){
        // Yeah, it worked.

        // Redisplay the first screen.
        char* simpleform_view;
        asprintf(&simpleform_view, "/%s/%s/view",
            h->uri_parts[QZ_URI_BASE_SEGMENT], form_name);

        location(h, simpleform_view );

    }else{

        char* err_msg = PQresultErrorMessage(action_rs);
        xmlNodePtr update_error = xmlNewTextChild(divqz, NULL, "pre", err_msg);
        append_class(update_error, "err_msg");

        // Re-do the edit

        simpleform_view(h, form_name, divqz);
    }

    PQclear(action_rs);

    return;
}

void simpleform(struct handler_args* h){

    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    char* action = get_uri_part(h, QZ_URI_ACTION);
    struct table_action* this_ta = open_table(h, form_name, action);

    if (h->conf->log_simpleform_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d simpleform %s action %s \n",
            gettime(), h->request_id, __func__, __LINE__,
            form_name, action);
        pthread_mutex_unlock(&log_mutex);
    }

    if (this_ta == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail table_action (%s,%s) not found\n",
            gettime(), h->request_id, __func__, __LINE__,
            form_name, action);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_BAD_REQUEST, "no action");
        return;
    }

    doc_from_file(h, this_ta->xml_template);
    if (h->error_exists) return;

    content_type(h, "text/html");

    xmlNodePtr divqz;
    if ((divqz = qzGetElementByID(h, this_ta->target_div)) == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d Target div ID '%s' was not found.\n",
            gettime(), h->request_id, __func__, __LINE__,
            this_ta->target_div);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_NOT_FOUND,
            "Div id element specified by form was not found");
        return;
    }

    // branch to the named action
    if (strcmp("view", action)==0){
        simpleform_view(h, form_name, divqz);

    }else if (strcmp("action", action)==0){
        simpleform_action(h, form_name, divqz);

    }else{
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d unknown action (%s) \n",
            gettime(), h->request_id, __func__, __LINE__, action);
        pthread_mutex_unlock(&log_mutex);

        error_page(h,400, "unknown action");
    }

    if (! h->error_exists){
        add_all_menus(h);
        doc_adder(h);
    }

    return;
}
