
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
 *  build_index
 *
 *  Build a hash table of all the id's plus head and body
 *  as __HEAD__ and __BODY__
 */
struct stack_node {
    xmlNodePtr node;
    struct stack_node* next;
};


void build_index(struct handler_args* h){

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d start build_index\n",
        gettime(), h->request_id, __func__, __LINE__);
    pthread_mutex_unlock(&log_mutex);

    if (h->doc == NULL) return;

    if (h->id_index != NULL){
        error_page(h, 500, "Attempt to build index when index already exists");
        return;
    }
    xmlNodePtr cur;
    cur = xmlDocGetRootElement(h->doc);
    if (cur == NULL){
         error_page(h, 500, "xmlDocGetRootElement not found");
         return;
    }

    struct stack_node* stack = calloc(1, sizeof(struct stack_node));
    struct stack_node* last = NULL;
    struct stack_node* this_node;
    xmlNodePtr child;
    xmlNodePtr check;
    xmlChar* this_id;

    bool head_found = false;
    bool body_found = false;

    h->id_index = xmlHashCreate(197);

    while (cur != NULL){
        this_id = xmlGetProp(cur, "id");

        if (h->conf->log_id_index_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log,  "%f %d %s:%d on node %s id %s\n",
                 gettime(), h->request_id, __func__, __LINE__,
                 cur->name, this_id);
            pthread_mutex_unlock(&log_mutex);
        }
        if (this_id != NULL){
            check = xmlHashLookup(h->id_index, this_id);
            if (check != NULL){
                error_page(h, 500, "Duplicate ID");
            }

            struct id_node* new_id = calloc(1, sizeof(struct id_node));
            new_id->node = cur;
            snprintf(new_id->id, QZ_MAX_ID_LENGTH, "%s", this_id);

            xmlHashAddEntry(h->id_index, new_id->id, new_id);
        }

        // Check for head, body nodes

        if ( ! head_found && (xmlStrcasecmp(cur->name, "head") == 0)){

            struct id_node* new_id = calloc(1, sizeof(struct id_node));
            new_id->node = cur;
            snprintf(new_id->id, QZ_MAX_ID_LENGTH, "%s", "__HEAD__");

            xmlHashAddEntry(h->id_index, new_id->id, new_id);
            head_found = true;
        }

        if ( ! body_found && (xmlStrcasecmp(cur->name, "body") == 0)){

            struct id_node* new_id = calloc(1, sizeof(struct id_node));
            new_id->node = cur;
            snprintf(new_id->id, QZ_MAX_ID_LENGTH, "%s", "__BODY__");

            xmlHashAddEntry(h->id_index, new_id->id, new_id);
            body_found = true;
        }

        child = cur->xmlChildrenNode;
        while (child != NULL){

            this_node = malloc(sizeof(struct stack_node));
            this_node->node = child;
            this_node->next = stack;
            stack = this_node;
            child = child->next;
        }
        xmlFree(this_id);

        last = stack;
        stack = stack->next;
        cur = last->node;
        free(last);
    }

}

/*
 *  add_index
 *
 *  Add the given node id and node ptr to the id index.
 */
void add_to_id_index(struct handler_args* h, xmlNodePtr the_node){

    char* node_id = xmlGetProp(the_node, "id");

    if (node_id != NULL){
        if (xmlHashLookup(h->id_index, node_id) != NULL){
            error_page(h, 500, "Duplicate ID");
            return;
        }

        struct id_node* new_id = calloc(1, sizeof(struct id_node));
        new_id->node = the_node;
        snprintf(new_id->id, QZ_MAX_ID_LENGTH, "%s", node_id);

        xmlHashAddEntry(h->id_index, new_id->id, new_id);
        xmlFree(node_id);
    }
}

/*
 *  doc_from_file
 *
 *  Turn a file name into an xml node tree,
 */  
void doc_from_file( struct handler_args* h, char* requested_docname ){

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d start doc_from_file %s\n", 
        gettime(), h->request_id, __func__, __LINE__,
        requested_docname); 
    pthread_mutex_unlock(&log_mutex);

    if (h->doc != NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d danger %s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            "doc_from_file called with h->doc defined");
        pthread_mutex_unlock(&log_mutex);
    }

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

    h->doc = xmlParseFile(full_path);

    if (h->doc == NULL){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "xmlParseFile failed");
        free(full_path);
        full_path = NULL;
        return;
    }

    free(full_path);
    full_path = NULL;

    build_index(h);

    add_jscss_links(h);

    char* form_refresh;
    // The form refresh rate should be a bit less than one third
    // of the form duration so that 3 refresh requests have a chance
    // before the inactivity timer kills things. d*95/300 for a
    // 60 second duration would refresh at 19,38, and 57 seconds.

    if ( !
        (uri_part_n_is(h, QZ_URI_FORM_NAME, "login") ||
         uri_part_n_is(h, QZ_URI_FORM_NAME, "logout")) ){

        asprintf(&form_refresh, "form_refresh_init(%d*1000)",
            h->conf->form_duration*95/300);

        add_listener(h, NULL, "onLoad", form_refresh);
        free(form_refresh);
    }
    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d doc_from_file complete\n", 
        gettime(), h->request_id, __func__, __LINE__); 
    pthread_mutex_unlock(&log_mutex);
}


/*
 *  validate_rule
 *
 *  Check that postdata matches its regex pattern if it
 *  is defined in the prompt rule and the it fits in maxlength.
 */
void validate_rule(void* val, void* data, const xmlChar* key){
    struct handler_args* h = data;
    char* base = NULL;

    // is there data there to check?
    size_t subject_length = strlen(val);
    size_t key_length = strlen(key);

    if (h->conf->log_validate_rule_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d key %s val length %"PRId64"\n",
            gettime(), h->request_id, __func__, __LINE__,
            key, (int64_t) subject_length);
        pthread_mutex_unlock(&log_mutex);
    }
    if (subject_length == 0) return;
    if (key_length == 0) return;

    if (key[key_length-1] == ']'){
        base = array_base(key);
    }else{
        asprintf(&base, "%s", key);
    }

    // fetch the prompt rule
    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    struct prompt_rule* rule = fetch_prompt_rule(h, form_name, base);

    if (h->conf->log_validate_rule_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d base:%s %s regex:%s\n",
            gettime(), h->request_id, __func__, __LINE__,
            base,
            (rule == NULL) ? "no rule":"rule found",
            (rule == NULL || rule->regex_pattern == NULL) ? "none":rule->regex_pattern);
        pthread_mutex_unlock(&log_mutex);
    }

    if (rule == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d prompt rule is null\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        free(base);
        return;
    }

    // does it have a pattern?
    if (rule->comp_regex == NULL){
        free_prompt_rule(h, rule);
        free(base);
        return;
    }
    // does the value fit the pattern?
    const int   ovectcount = 30;
    int ovector[ovectcount];
    int rc;

    rc = pcre_exec(rule->comp_regex, NULL, val, subject_length, 0, 0,
        ovector, ovectcount);

    static char* regex_failure_hdr =
        "One or more fields submitted failed validation.\n"
        "This error should have been caught by the client\n"
        "before the data was submitted.\n"
        "You may be able to recover by using your back\n"
        "button and correcting your data (or may not).\n\n";

    if (rc < 0){ // match failed

        if (h->data == NULL){
            // Then this is the first one, start with an
            // explanatory note about the failure.

            h->data = new_strbuf(regex_failure_hdr, 0);
            content_type(h, "text/plain");
            h->regex_check = failed;
        }

        char* error_msg;
        asprintf(&error_msg, "attribute %s failed regex_pattern %s rc=%d\n\n",
             key, rule->regex_pattern, rc);

        strbuf_append(h->data, new_strbuf(error_msg,0));

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d "
            "fail attribute \"%s\" regex_pattern %s rc=%d\n\n",
            gettime(), h->request_id, __func__, __LINE__,
            rule->fieldname, rule->regex_pattern, rc);
        pthread_mutex_unlock(&log_mutex);

        free(error_msg);

    }else{ // match passed

        if (h->conf->log_validate_rule_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d regex_pattern passed for %s\n",
                gettime(), h->request_id, __func__, __LINE__, key);
            pthread_mutex_unlock(&log_mutex);
        }
    }

    // Check the length of val against the rule maxlength

    if (rule->maxlength > 0){
        size_t val_length = strlen(val);
        if (val_length > rule->maxlength){

            h->regex_check = failed;

            if (h->data == NULL){
                // Then this is the first one, start with an
                // explanatory note about the failure.

                h->data = new_strbuf(regex_failure_hdr, 0);
                content_type(h, "text/plain");
                h->regex_check = failed;
            }

            char* length_error;
            asprintf(&length_error,
                "fail attribute \"%s\"\n"
                "length of %"PRIu64" exceeds maxlength %d\n\n",
                key, (uint64_t)val_length, rule->maxlength);

            strbuf_append(h->data, new_strbuf(length_error,0));

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d key %s value length %"PRIu64
                " exceeds maxlength %d\n",
                gettime(), h->request_id, __func__, __LINE__,
                key, (uint64_t)val_length, rule->maxlength);
            pthread_mutex_unlock(&log_mutex);

            free(length_error);
        }
    }
    free_prompt_rule(h, rule);
    free(base);
}

/*
 *  valid_pkey_value_scanner
 *
 *  For each value+primary key record saved in the form record,
 *  check that the value posted, if any, matches.
 */
void valid_pkey_value_scanner(void* payload, void* data, const xmlChar* name){
    char* saved_value = payload;
    struct handler_args* hargs = data;

    char* posted_value = xmlHashLookup(hargs->postdata, name);

    if (has_data(posted_value) && (strcmp(saved_value, posted_value) != 0)){
        hargs->pkey_check = failed;

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d fail primary key validation key=%s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            name);
        pthread_mutex_unlock(&log_mutex);
    }
}

/*
 *  check_postdata
 *
 *  Scan the postdata, set *_check flags in the handler.
 *  Test that each supplied attribute is appropriate for its type.
 *  More checks required, add them here.
 */
bool check_postdata(struct handler_args* h){

    /* utf8 checking is done in parse_key_eq_val */

    xmlHashScan(h->postdata, (void*) validate_rule, h);

    if (h->regex_check == failed){
        return false;
    }else{
        h->regex_check = passed;
    }

    struct form_record* form_rec = get_posted_form_record(h);
    if ((form_rec != NULL) && (form_rec->pkey_values != NULL)){
        xmlHashScan(form_rec->pkey_values, (void*) valid_pkey_value_scanner, h);

        if (h->pkey_check == failed){
            error_page(h, SC_BAD_REQUEST, "Primary key validation failed");
            return false;
        }else{
            h->pkey_check = passed;
        }
    }
    return true;
}

/*
 *  save_pkey_values
 *
 *  Save any value whose key is in the table action's pkey list
 *  and also in PGresult into the form records pkey_values hash table.
 */
void save_pkey_values(struct handler_args* h,
    struct form_record* form_rec,
    struct table_action* ta,
    PGresult* rs,
    int row){

    if (ta == NULL) return;
    if (rs == NULL) return;
    if (form_rec == NULL) return;

    int n;
    char* value;
    char* pkey_value_record;
    char* this_key;

    if (form_rec->pkey_values == NULL){
        // XXXXXX Another size to add to config
        form_rec->pkey_values = xmlHashCreate(23);
    }
    for(n=0; n < ta->nbr_pkeys; n++){
        value = get_value(rs, row, ta->pkeys[n]);
        if (has_data(value)){

            // This is "value\0key\0" so lookup on key returns value.
            asprintf(&pkey_value_record, "%s%c%s%c",
                value, '\0', ta->pkeys[n], '\0');

            this_key =  pkey_value_record + strlen(pkey_value_record) + 1;

            xmlHashAddEntry(form_rec->pkey_values, this_key, pkey_value_record);

        }
    }
}

#ifdef ID_INDEX_TEST

pthread_mutex_t log_mutex;

int main(void){

    // setup a fake environment
    pthread_mutex_init(&log_mutex,NULL);
    qzrandom64_init();
    struct qz_config* conf = init_config();

    struct FCGX_Request freq = (struct FCGX_Request){
       .requestId = 1,
    };
    struct handler_args hargs = (struct handler_args){
       .request_id = 1,
       .request = &freq,
       .conf = conf,
       .log = stdout
    };
    struct handler_args* h = &hargs;
    char* id_list[] = {"stuff_zero", "qz", "helpful_text", "qzsubmenu", "qzmenu", "borkelsnot", NULL};

    int loop;
    for(loop=0; loop<100000; loop++){

        doc_from_file(h, "base.xml");

        struct id_node* id_item;
        int item;
        for(item=0; id_list[item] != NULL; item++){

            id_item = xmlHashLookup(h->id_index, id_list[item]);

            if (id_item != NULL){

                fprintf(h->log, "%f %d %s:%d id=%s name=%s\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    id_item->id, id_item->node->name);
            }else{
                fprintf(h->log, "%f %d %s:%d id=%s not found\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    id_list[item]);

            }
        }

        xmlFreeDoc(h->doc);
        h->doc = NULL;
        xmlHashFree(h->id_index, xmlFree);
        h->id_index = NULL;
        h->request_id++;
    }
    return 0;
}


#endif
