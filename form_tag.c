
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
 *  register_form
 *
 *  Add an entry to the form_tag hash and add an
 *  html hidden input of the tag.
 *  The returned record pointer is useful with the 
 *  duplicate_registration function, but not otherwise.
 */

struct form_record* register_form(struct handler_args* h, 
    xmlNodePtr form_node,
    bool submit_only_once,
    char* form_action
    ){

    // build the record
    int action_url_len = strlen(form_action);
    int form_rec_record_size = sizeof(struct form_record) + action_url_len + 2;

    struct form_record* form_rec = calloc(1, form_rec_record_size);
    
    uint64_t form_id = qzrandom64ch(form_rec->form_id);

    time_t valid_duration = h->conf->form_duration;

    form_rec->is_valid = true;
    form_rec->created = time(NULL);
    form_rec->duration = valid_duration;
    form_rec->expires = form_rec->created + valid_duration;
    form_rec->submit_only_once = submit_only_once;
    form_rec->session_integrity_token = h->session->integrity_token;
  
    snprintf(form_rec->form_action, action_url_len+1, "%s", form_action);

    // save the record.
    xmlHashAddEntry(h->session->form_tags, form_rec->form_id, form_rec);

    // add the hidden input field.

    xmlNodePtr tag_node = xmlNewChild(form_node, NULL, "input", NULL);
    xmlNewProp(tag_node, "type", "hidden");
    xmlNewProp(tag_node, "name", "form_tag");
    xmlNewProp(tag_node, "refresh", "1");
    xmlChar tagbuf[ETAG_MAX_LENGTH];
    make_etag(tagbuf, h->session->tagger_socket_path, form_id);
    xmlNewProp(tag_node, "value", tagbuf);

    // add an expires attribute

    char* expires_buf;
    int arc;
    arc = asprintf(&expires_buf, "%lld", form_rec->expires);
    xmlNewProp(form_node, "expires", expires_buf);
    free(expires_buf);

    // 
    form_rec->form_set = h->current_form_set;

    return form_rec;
}

/*
 *  duplicate_registration
 *
 *  In the case where a large html table generates an edit form
 *  for every row, and all the edit buttons have the same form action,
 *  the html form_tag can reference the same struct form_record.
 *  This function uses the previously created form_record for a new
 *  html form tag.
 */
void duplicate_registration(struct handler_args* h, 
    struct form_record* previous_tag, xmlNodePtr new_node){

    uint64_t form_id;
    memcpy(&form_id, previous_tag->form_id, 8);

    // add the hidden input field.

    xmlNodePtr tag_node = xmlNewChild(new_node, NULL, "input", NULL);
    xmlNewProp(tag_node, "type", "hidden");
    xmlNewProp(tag_node, "name", "form_tag");
    xmlNewProp(tag_node, "refresh", "0");
    xmlChar tagbuf[ETAG_MAX_LENGTH];
    make_etag(tagbuf, h->session->tagger_socket_path, form_id);
    xmlNewProp(tag_node, "value", tagbuf);

    // add an expires attribute

    char* expires_buf;
    int arc;
    arc = asprintf(&expires_buf, "%lld", previous_tag->expires);
    xmlNewProp(new_node, "expires", expires_buf);
    free(expires_buf);

    return;
}

/*
 *  valid_context_parameter_scanner
 *
 *  If any elements in the post data are also in the context
 *  parameter hash table then they must have the same value.
 *  The context parameter is given in name and payload, the
 *  posted value comes from h->postdata
 */
void valid_context_parameter_scanner(void* payload, void* data, xmlChar* name){
    char* saved_value = payload;
    struct handler_args* h = data;

    char* submitted_value = xmlHashLookup(h->postdata, name);

    if (submitted_value != NULL){
        if (strcmp(saved_value, submitted_value) != 0){
           h->posted_form->form_set->is_valid = false;
           error_page(h, SC_BAD_REQUEST, "Invalid Data");

           fprintf(h->log, "%f %d %s:%d form set context parameter \"%s\" "
               "changed\n",
               gettime(), h->request_id, __func__, __LINE__,
               name);
        }
    }
}

/*
 *  post_contains_valid_form_tag
 *
 *  Check that the form_tag is present and valid.
 *  If the form is marked use only once then set
 *  the form as invalid but treat it as valid.
 *  
 */

bool post_contains_valid_form_tag(struct handler_args* h){

    struct form_record* this_form = get_posted_form_record(h);
    if (this_form == NULL) return false;

    if (!this_form->is_valid){
        fprintf(h->log, "%f %d %s:%d fail form record flagged as invalid "
           "form_action=%s\n", 
           gettime(), h->request_id, __func__, __LINE__,
           this_form->form_action); 

        return false;
    }    

    // Compare the url saved in the form record to the url used in 
    // the request.  Note url used is allowed to have extra text, just
    // the part saved has to match the begining of what is used.
    // This allows the url to pass additional information as needed.
    // For /qz/refresh requests, skip url match and submit only once flag.

    int form_action_length = strlen(this_form->form_action);
    char* request_url = FCGX_GetParam("REQUEST_URI",h->envpfcgi); 

    if ( strncmp( get_uri_part(h, QZ_URI_FORM_NAME), "refresh", 
        MAX_SEGMENT_LENGTH) != 0){

        if (strncmp(this_form->form_action, request_url,
            form_action_length) != 0){
        
            fprintf(h->log, 
               "%f %d %s:%d %s %s %s %s %s %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                "fail saved url", 
                this_form->form_action, 
                "does not match request", 
                request_url,
                "and is not refresh",
                 get_uri_part(h, QZ_URI_FORM_NAME)
                ); 

            return false;
        }   

        if (this_form->submit_only_once){
            this_form->is_valid = false;
        }
    }

    if (this_form->expires < time(NULL)){
        fprintf(h->log, "%f %d %s:%d fail form expired at %lld form_action=%s\n", 
           gettime(), h->request_id, __func__, __LINE__,
           this_form->expires, this_form->form_action);

        return false;   
    }

    // save the form for later, later includes the next step.
    h->posted_form = this_form;
    if (h->posted_form->form_set != NULL){
        struct form_set* fs = h->posted_form->form_set;
        fprintf(h->log, "%f %d %s:%d form_set->name=%s sec_token_ok=%c\n",
           gettime(), h->request_id, __func__, __LINE__,
           fs->name,  (fs->integrity_token == h->session->integrity_token)? 't':'f' );
    }

    // Verify the posted values match the form set for any context parameters.
    if (this_form->form_set != NULL){
        xmlHashScan(h->posted_form->form_set->context_parameters,
            valid_context_parameter_scanner, h);
    }

    fprintf(h->log, "%f %d %s:%d form tag OK form_action=%s\n", 
        gettime(), h->request_id, __func__, __LINE__,
        this_form->form_action); 
        
    return true;
}

/*
 *  refresh_form_tag
 *
 *  Find the form tag referenced
 *  if it has not yet expired,
 *  extend the expiration time by duration
 *  and return the new expire time 
 *  via http as text/plain
 */
void refresh_form_tag(struct handler_args* h){

    content_type(h, "text/plain");

    // function post_contains_valid_form_tag has already
    // been run and passed, error checking already done.  

    char* form_tag = xmlHashLookup(h->postdata, "form_tag");
    
    uint64_t payload_int;
    xmlChar payload_str[9];
    bzero(payload_str,9);

    payload_int = validate_etag(h->session->tagger_socket_path, form_tag);
    if (payload_int == 0) return;

    memcpy(payload_str, &payload_int, 8);
    struct form_record* this_form;
    this_form = (struct form_record*) xmlHashLookup(h->session->form_tags, 
        payload_str);

    if (this_form == NULL) return;

    if ((this_form->is_valid) && (this_form->expires >= time(NULL))){
        // This is the only write to the form tag, and it is an
        // atomic assignment of a single int.  Housekeeper would 
        // not be touching it now because it is still valid.  
        // Still, this could be a race condition if refresh requests
        // come in faster than they can be processed and get handled
        // out of order.  In which case the form expires sooner rather
        // than later.
        time_t new_expires = time(NULL) + this_form->duration;
        this_form->expires = new_expires;

        // longest 64 bit number is 20 chars, using 32
        struct strbuf* sb = new_strbuf(NULL, 32); 
        snprintf(sb->str, 32, "%lld", new_expires);
        h->data = sb;
    }else{
        //  not valid, no change, return zero
        h->data = new_strbuf("0", 0); 
    }
    fprintf(h->log, "%f %d %s:%d refresh for %s set to %s\n", 
        gettime(), h->request_id, __func__, __LINE__,
        FCGX_GetParam("REQUEST_URI",h->envpfcgi), h->data->str);
}

struct form_tag_housekeeping_data {
    // xmlHashTablePtr form_tags;
    struct session* this_session;  // the session being cleaned
    struct handler_args* hargs;    // the housekeeper's not the session's
};

/*
 *  form_tag_scanner
 * 
 *  Called for each form tag record.
 *  Look for old stuff and toss it.
 */
void form_tag_housekeeping_scanner(void* payload, void* data, xmlChar* name){

    struct form_record* form_rec = payload;
    struct form_tag_housekeeping_data * ft_hk_data = data;

    // Allow twice the duration before removing 
    // This makes it possible to display expired messages instead
    // of not found messages for recently expired forms.
    if ( time(NULL) >  (form_rec->expires + 2*form_rec->duration)){

        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d removing form tag %s\n", 
           gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
           form_rec->form_action); 

        decrement_form_set(form_rec);

        xmlHashRemoveEntry(ft_hk_data->this_session->form_tags, name, NULL);
        free(form_rec);
    }else if (time(NULL) > form_rec->expires){
        form_rec->is_valid = false;
    }    
}

/* 
 *  form_tag_housekeeping
 *
 *  Run through the form records looking for things to throw out.
 */
void form_tag_housekeeping(struct handler_args* hargs, 
    struct session* this_session){

    struct form_tag_housekeeping_data ft_hk_data = 
        (struct form_tag_housekeeping_data) {
            //.form_tags = this_session->form_tags,
            .this_session = this_session,
            .hargs = hargs
    };

    // hash scan args: xmlHashTablePtr, scanner function, blind data 
    xmlHashScan(this_session->form_tags, form_tag_housekeeping_scanner, 
        &ft_hk_data);

    xmlHashScan(this_session->form_sets, form_set_housekeeping_scanner, 
       ft_hk_data.this_session);
}

/*
 *  get_posted_form_record
 *
 *  Obtain the form record indicated by the form tag 
 *  in the post data.
 */
struct form_record* get_posted_form_record(struct handler_args* h){
 
    if (h->postdata == NULL) return NULL;

    char* form_tag = xmlHashLookup(h->postdata, "form_tag");
    if (form_tag == NULL){
        fprintf(h->log, "%f %d %s:%d fail form_tag not found in post data\n", 
           gettime(), h->request_id, __func__, __LINE__); 
 
        return NULL;
    }    
 
    // I need a null after the payload so the integer can be
    // interpeted as a string for xml hash use.
    uint64_t payload_int;
    xmlChar payload_str[9];
    bzero(payload_str,9);

    payload_int = validate_etag(h->session->tagger_socket_path, form_tag);

    if (payload_int == 0){
        fprintf(h->log, "%f %d %s:%d form_tag validation failed\n", 
           gettime(), h->request_id, __func__, __LINE__); 

        return NULL;
    }
    memcpy(payload_str, &payload_int, 8);
    struct form_record* this_form;
    this_form = xmlHashLookup(h->session->form_tags, payload_str);

    if (this_form == NULL){ 
        fprintf(h->log, "%f %d %s:%d fail form record not found in hash table\n", 
           gettime(), h->request_id, __func__, __LINE__); 
         
        return NULL;
    } 

    // If the token fails to match then throw a hard error
    // and kill the session.
    if (this_form->session_integrity_token != h->session->integrity_token){
        fprintf(h->log, 
            "%f %d %s:%d fail form record integrity token invalid\n",
            gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_INTERNAL_SERVER_ERROR, "Bad Token");
        h->session->is_logged_in = false;

        return NULL;
    }

    if ((this_form->form_set != NULL) &&
        (this_form->form_set->integrity_token != 
            h->session->integrity_token)){

        fprintf(h->log, 
            "%f %d %s:%d fail form set integrity token check invalid\n",
            gettime(), h->request_id, __func__, __LINE__);
        
        error_page(h, SC_INTERNAL_SERVER_ERROR, "Bad Token");
        h->session->is_logged_in = false;
        return NULL;
    }        

    return this_form;
}


