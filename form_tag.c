
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
#include "hex_to_uchar.h"


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
    
    qzrandomch(form_rec->form_id, 16, last_is_null);

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

    // Set the form set name if appropriate
    char* posted_form_set_name = NULL;
    char* new_form_set_name = NULL;

    if ((h != NULL) && (h->posted_form != NULL) &&
        (h->posted_form->form_set != NULL)){

            posted_form_set_name = h->posted_form->form_set->name;
    }

    if ((h != NULL) && (h->page_ta != NULL) &&
        (h->page_ta->form_set_name[0] != '\0')){

            new_form_set_name = h->page_ta->form_set_name;
    }

    if ((posted_form_set_name != NULL) &&
        (new_form_set_name != NULL)){

        if (strcmp(posted_form_set_name, new_form_set_name) == 0){
            // It's the one, save it.
            h->current_form_set = h->posted_form->form_set;
        }
    }
    // save the record.
    xmlHashAddEntry(h->session->form_tags, form_rec->form_id, form_rec);

    // add the hidden input field.

    xmlNodePtr tag_node = xmlNewChild(form_node, NULL, "input", NULL);
    xmlNewProp(tag_node, "type", "hidden");
    xmlNewProp(tag_node, "name", "form_tag");
    xmlNewProp(tag_node, "refresh", "1");
    xmlChar tagbuf[ETAG_MAX_LENGTH];

    make_etag(tagbuf, h->conf->tagger_socket_path, h->session->form_tag_token,
        form_rec->form_id);

    xmlNewProp(tag_node, "value", tagbuf);

    // add an expires attribute
    char* expires_buf;
    asprintf(&expires_buf, "%"PRId64, (int64_t)form_rec->expires);
    xmlNewProp(form_node, "expires", expires_buf);
    free(expires_buf);

    //
    if (form_set_is_valid(h, h->current_form_set)){
        form_rec->form_set = h->current_form_set;
        h->current_form_set->ref_count++;
    }else{
        form_rec->form_set = NULL;
    }    

    if (h->conf->log_form_tag_details){
        char* hex_form_id = uchar_to_hex(form_rec->form_id, 16);

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d form_id = %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            hex_form_id);
        pthread_mutex_unlock(&log_mutex);

        free(hex_form_id);
    }
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
void valid_context_parameter_scanner(void* payload, void* data,
    const xmlChar* name){

    char* saved_value = payload;
    struct handler_args* h = data;

    char* submitted_value = xmlHashLookup(h->postdata, name);

    if (has_data(saved_value) && has_data(submitted_value)){
        if (strcmp(saved_value, submitted_value) != 0){
            h->posted_form->form_set->is_valid = false;
            error_page(h, SC_BAD_REQUEST, "Invalid Data");

            if (h->conf->log_form_set_details){
                pthread_mutex_lock(&log_mutex);
                fprintf(h->log, "%f %d %s:%d fail form set context parameter "
                    "\"%s\" changed from %s to %s\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    name, saved_value, submitted_value);
                pthread_mutex_unlock(&log_mutex);

            }else{
                pthread_mutex_lock(&log_mutex);
                fprintf(h->log, "%f %d %s:%d fail form set context parameter "
                    "\"%s\" changed\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    name);
                pthread_mutex_unlock(&log_mutex);
          }
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
        char payload[16];

        form_tag_zero = xmlHashLookup(h->postdata, "form_tag[0]" );
        if (form_tag_zero != NULL){

            validate_etag(payload, h->conf->tagger_socket_path,
                h->session->form_tag_token, form_tag_zero);

            if (strlen(payload) == 15) return true;
        }
        return false;
    }

    // Not a refresh

    struct form_record* this_form = get_posted_form_record(h);
    if (this_form == NULL) return false;

    if ( ! this_form->is_valid){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail form record flagged as invalid "
           "form_action=%s\n", 
           gettime(), h->request_id, __func__, __LINE__,
           this_form->form_action); 
        pthread_mutex_unlock(&log_mutex);

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
        
            pthread_mutex_lock(&log_mutex);
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
            pthread_mutex_unlock(&log_mutex);

            return false;
        }   

        if (this_form->submit_only_once){
            this_form->is_valid = false;
        }
    }

    if (this_form->expires < time(NULL)){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail form expired at %"PRId64" "
           "form_action=%s\n",
           gettime(), h->request_id, __func__, __LINE__,
           (int64_t)this_form->expires, this_form->form_action);
        pthread_mutex_unlock(&log_mutex);

        return false;   
    }

    // save the form for later, later includes the next step.
    h->posted_form = this_form;
    if (h->posted_form->form_set != NULL){
        struct form_set* fs = h->posted_form->form_set;
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d form_set->name=%s sec_token_ok=%c\n",
           gettime(), h->request_id, __func__, __LINE__,
           fs->name,  (fs->integrity_token == h->session->integrity_token) ?
               't':'f' );
        pthread_mutex_unlock(&log_mutex);
    }

    // Verify the posted values match the form set for any context parameters.
    if (this_form->form_set != NULL){
        xmlHashScan(h->posted_form->form_set->context_parameters,
            (void*) valid_context_parameter_scanner, h);
    }

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d form tag OK form_action=%s\n", 
        gettime(), h->request_id, __func__, __LINE__,
        this_form->form_action); 
    pthread_mutex_unlock(&log_mutex);
        
    return true;
}

/*
 *  refresh returns an JSON thing
 *  {[ {form_id: expires},...]}
 *
 *  {[ {menu[0]:1486745337}, {menu[1]:1486745337}, {edit[0]:1486745337} ]}
 */
void refresh_one_tag(struct handler_args* h, char* form_id, char* form_tag){

    xmlChar payload[16];
    struct form_record* this_form;
    char* result = NULL;
    char comma = ',';

    // No comma before the first element
    if (h->data == NULL){
        h->data = new_strbuf("{[ ",0);
        comma = ' ';
    }

    validate_etag(payload, h->conf->tagger_socket_path,
        h->session->form_tag_token, form_tag);

    // An invalid tag is never OK.
    if (strlen(payload)  != 15){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail invalid form tag\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_BAD_REQUEST, "Invalid form tag submitted to refresh");
        return;
    }

    this_form = (struct form_record*) xmlHashLookup(h->session->form_tags,
        payload);

    if (this_form == NULL){
       asprintf(&result, "%c {%s:%d}", comma, form_id, 0);

    }else if ((this_form->is_valid) && (this_form->expires >= time(NULL))){

        time_t new_expires = time(NULL) + this_form->duration;
        this_form->expires = new_expires;

        asprintf(&result, "%c {%s:%"PRId64"}", comma, form_id, (int64_t)new_expires);

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

            free(form_id_key);
            free(form_tag_key);

            if (h->error_exists) break;
        }else{
            free(form_id_key);
            free(form_tag_key);
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
void delete_form_record(void* payload, void* data, const xmlChar* name){

    struct form_record* form_rec = payload;
    struct form_tag_housekeeping_data * ft_hk_data = data;

    if (form_rec == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d form_rec is null\n", 
            gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        return;
    }
    uint64_t form_id;
    memcpy(&form_id, form_rec->form_id, 8);

    if (ft_hk_data->hargs->conf->log_form_tag_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d removing form tag %"PRIx64"\n",
            gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
            form_id);
        pthread_mutex_unlock(&log_mutex);
    }
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
void form_tag_housekeeping_scanner(void* payload, void* data, const xmlChar* name){

    struct form_record* form_rec = payload;
    struct form_tag_housekeeping_data * ft_hk_data = data;

    if (form_rec == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(ft_hk_data->hargs->log, "%f %d %s:%d "
            "unexpected null form_tag name=%s\n",
            gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
            name);
        pthread_mutex_unlock(&log_mutex);
    }else{
        uint64_t form_id;
        memcpy(&form_id, form_rec->form_id, 8);

        if (ft_hk_data->hargs->conf->log_form_tag_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(ft_hk_data->hargs->log, "%f %d %s:%d "
                "checking form_id=%"PRIx64" %s %ld\n",
                gettime(), ft_hk_data->hargs->request_id, __func__, __LINE__,
                form_id, "expires in", (long) (time(NULL) - form_rec->expires) );
            pthread_mutex_unlock(&log_mutex);
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
    xmlHashScan(this_session->form_tags, (void*) form_tag_housekeeping_scanner,
        &ft_hk_data);

    xmlHashScan(this_session->form_sets, (void*) form_set_housekeeping_scanner,
       &ft_hk_data);
}

/*
 *  close_all_form_tags
 *
 *  What it says on the tin.
 */
void close_all_form_tags(struct handler_args* hargs,
    struct session* this_session){

    struct form_tag_housekeeping_data ft_hk_data = 
        (struct form_tag_housekeeping_data) {
            .this_session = this_session,
            .hargs = hargs
     };

    xmlHashScan(this_session->form_tags, (void*) delete_form_record,
        &ft_hk_data);
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
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail form_tag not found in post data\n", 
           gettime(), h->request_id, __func__, __LINE__); 
        pthread_mutex_unlock(&log_mutex);
 
        return NULL;
    }    
 
    xmlChar payload[16];
    validate_etag(payload, h->conf->tagger_socket_path,
        h->session->form_tag_token, form_tag);

    if (strlen(payload) != 15){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d form_tag validation failed\n", 
           gettime(), h->request_id, __func__, __LINE__); 
        pthread_mutex_unlock(&log_mutex);

        return NULL;
    }
    struct form_record* this_form;
    this_form = xmlHashLookup(h->session->form_tags, payload);

    if (this_form == NULL){ 
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail form record not found "
           "in hash table\n",
           gettime(), h->request_id, __func__, __LINE__); 
        pthread_mutex_unlock(&log_mutex);
         
        return NULL;
    } 

    // If the token fails to match then throw a hard error
    // and kill the session.
    if (this_form->session_integrity_token != h->session->integrity_token){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log,
            "%f %d %s:%d fail form record integrity token invalid\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_INTERNAL_SERVER_ERROR, "Bad Token");
        h->session->is_logged_in = false;

        return NULL;
    }

    if ( (this_form->form_set != NULL) &&
        (! form_set_is_valid(h, this_form->form_set))){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log,
            "%f %d %s:%d fail form set integrity token check invalid\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_INTERNAL_SERVER_ERROR, "Bad Token");
        h->session->is_logged_in = false;
        return NULL;
    }

    return this_form;
}


