
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
    // XXXXXX Another size to add to config
    //form_rec->pkey_values = xmlHashCreate(23);
    form_rec->pkey_values = NULL;
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
    if (form_set_is_valid(h, h->current_form_set)){
        form_rec->form_set = h->current_form_set;
        h->current_form_set->ref_count++;
    }else{
        form_rec->form_set = NULL;
    }    

    fprintf(h->log, "%f %d %s:%d form_id = %llx\n",
        gettime(), h->request_id, __func__, __LINE__,
        form_id);

    return form_rec;
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

    if (has_data(saved_value) && has_data(submitted_value)){
        if (strcmp(saved_value, submitted_value) != 0){
           h->posted_form->form_set->is_valid = false;
           error_page(h, SC_BAD_REQUEST, "Invalid Data");

           fprintf(h->log, "%f %d %s:%d fail form set context parameter \"%s\" "
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

    // refresh returns many tags using form_tag[n] notation.
    // Just check for the first one. There is no need to validate
    // the tag matches the called handler.
    if (uri_part_n_is(h, QZ_URI_FORM_NAME, "refresh")){

        char* form_tag_zero;
        uint64_t tag_value;

        form_tag_zero = xmlHashLookup(h->postdata, "form_tag[0]" );
        if (form_tag_zero != NULL){
            tag_value = validate_etag(h->session->tagger_socket_path,
                form_tag_zero);

            if (tag_value > 0) return true;
        }
        return false;
    }

    // Not a refresh

    struct form_record* this_form = get_posted_form_record(h);
    if (this_form == NULL) return false;

    if ( ! this_form->is_valid){
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
 *  refresh returns an JSON thing
 *  {[ {form_id: expires},...]}
 *
 *  {[ {menu[0]:1486745337}, {menu[1]:1486745337}, {edit[0]:1486745337} ]}
 */
void refresh_one_tag(struct handler_args* h, char* form_id, char* form_tag){

    uint64_t payload_int;
    xmlChar payload_str[9];
    struct form_record* this_form;
    char* result = NULL;
    char comma = ',';

    // No comma before the first element
    if (h->data == NULL){
        h->data = new_strbuf("{[ ",0);
        comma = ' ';
    }

    payload_int = validate_etag(h->session->tagger_socket_path, form_tag);

    // An invalid tag is never OK.
    if (payload_int == 0){
        fprintf(h->log, "%f %d %s:%d fail invalid form tag\n",
            gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_BAD_REQUEST, "Invalid form tag submitted to refresh");
        return;
    }

    memcpy(payload_str, &payload_int, 8);
    payload_str[8] = '\0';

    this_form = (struct form_record*) xmlHashLookup(h->session->form_tags,
        payload_str);

    if (this_form == NULL){
       asprintf(&result, "%c {%s:%d}", comma, form_id, 0);

    }else if ((this_form->is_valid) && (this_form->expires >= time(NULL))){

        time_t new_expires = time(NULL) + this_form->duration;
        this_form->expires = new_expires;

        asprintf(&result, "%c {%s:%lld}", comma, form_id, new_expires);

    }else{
        //  not valid, no change, return zero
        asprintf(&result, "%c {%s,%d}", comma, form_id, 0);
    }
    strbuf_append(h->data, new_strbuf(result,0));
    free(result);
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

    char* form_id_key;
    char* form_id_value;
    char* form_tag_key;
    char* form_tag_value;
    int k;

    content_type(h, "application/json");

    for(k=0; ;k++){
        asprintf(&form_id_key, "form_id[%d]", k);
        form_id_value = xmlHashLookup(h->postdata, form_id_key);

        asprintf(&form_tag_key, "form_tag[%d]", k);
        form_tag_value = xmlHashLookup(h->postdata, form_tag_key);

        if (has_data(form_id_value) && has_data(form_tag_value)){
            refresh_one_tag(h, form_id_value, form_tag_value);

            if (h->error_exists) break;
        }else{
            break;
        }
    }
    if (h->data != NULL){
        strbuf_append(h->data, new_strbuf(" ]}",0));
    }
}

void pkey_values_deallocator(void* pkey_value, xmlChar* pkey){
    free(pkey_value);
}

/*
 *  delete_form_record
 *
 *  Remove the given form record and clean it up.
 */
void delete_form_record(void* payload, void* data, xmlChar* name){

    struct form_record* form_rec = payload;
    struct form_tag_housekeeping_data * ft_hk_data = data;

    if (form_rec == NULL){
        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d form_rec is null\n", 
            gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__);

        return;
    }
    uint64_t form_id;
    memcpy(&form_id, form_rec->form_id, 8);

    fprintf(ft_hk_data->hargs->log, "%f %d %s:%d removing form tag %llx\n", 
        gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
        form_id);

    decrement_form_set(form_rec);

    // A hash table scanner calling another hash table scanner.
    // This one will free primary key value records.
    if (form_rec->pkey_values != NULL){
        //xmlHashFree(form_rec->pkey_values, pkey_values_deallocator);
        xmlHashFree(form_rec->pkey_values, (xmlHashDeallocator)xmlFree);
    }
    xmlHashRemoveEntry(ft_hk_data->this_session->form_tags, name, NULL);
    free(form_rec);
}
/*
 *  form_tag_scanner
 * 
 *  Called for each form tag record.
 *  Look for old stuff and toss it.
 */
void form_tag_housekeeping_scanner(void* payload, void* data, xmlChar* name){

    struct form_record* form_rec = payload;
    struct form_tag_housekeeping_data * ft_hk_data = data;

    if (form_rec == NULL){
        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d unexpected null form_tag name=%s\n",
            gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
            name);
    }else{
        uint64_t form_id;
        memcpy(&form_id, form_rec->form_id, 8);
        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d checking form_id=%llx %s %d\n",
            gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
            form_id, "expires in", (time(NULL) - form_rec->expires) );
    }

    // Allow twice the duration before removing 
    // This makes it possible to display expired messages instead
    // of not found messages for recently expired forms.
    if ( time(NULL) >  (form_rec->expires + 2*form_rec->duration)){

        delete_form_record(form_rec, ft_hk_data, name);

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
       &ft_hk_data);
}

/*
 *  close_all_form_tags
 *
 *  What it says on the tin.
 */
void close_all_form_tags(struct handler_args* hargs, struct session* this_session){

    struct form_tag_housekeeping_data ft_hk_data = 
        (struct form_tag_housekeeping_data) {
            .this_session = this_session,
            .hargs = hargs
     };

    xmlHashScan(this_session->form_tags, delete_form_record, &ft_hk_data);
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

    if ( (this_form->form_set != NULL) && 
        (! form_set_is_valid(h, this_form->form_set))){

        fprintf(h->log, 
            "%f %d %s:%d fail form set integrity token check invalid\n",
            gettime(), h->request_id, __func__, __LINE__);
        
        error_page(h, SC_INTERNAL_SERVER_ERROR, "Bad Token");
        h->session->is_logged_in = false;
        return NULL;
    }        

    return this_form;
}


