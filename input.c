
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
 *  doc_from_file
 *
 *  Turn a file name into an xml node tree,
 */  
xmlDocPtr doc_from_file( struct handler_args* h, char* requested_docname ){

    fprintf(h->log, "%f %d %s:%d start doc_from_file %s\n", 
        gettime(), h->request_id, __func__, __LINE__,
        requested_docname); 

    if (h->doc != NULL){
        fprintf(h->log, "%f %d %s:%d danger %s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            "doc_from_file called with h->doc undefined");
    }

    if (h->conf == NULL){
        fprintf(h->log, "%f %d %s:%d fail h->conf is null\n",
            gettime(), h->request_id, __func__, __LINE__);

        return NULL;
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

/*
 *  validate_regex
 *
 *  Check that postdata matches its regex pattern if it
 *  is defined in the prompt rule.
 */
void validate_regex(void* val, void* data, xmlChar* key){
    struct handler_args* h = data;

    // is there data there to check?
    int subject_length = strlen(val);
    if (subject_length == 0) return;

    // fetch the prompt rule
    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    struct prompt_rule* rule = fetch_prompt_rule(h, form_name, key);
    if (rule == NULL) return;

    // does it have a pattern?
    if (rule->comp_regex == NULL){
        free_prompt_rule(h, rule);
        return;
    }

    // does the value fit the pattern?
    const int   ovectcount = 30;
    int ovector[ovectcount];
    int rc;

    rc = pcre_exec(rule->comp_regex, NULL, val, subject_length, 0, 0,
        ovector, ovectcount);

    if (rc < 0){ // match failed

        if (h->data == NULL){
            // Then this is the first one, start with an
            // explanatory note about the failure.
            static char* regex_failure_hdr = 
                "One or more fields submitted failed validation.\n"
                "This error should have been caught by the client\n"
                "before the data was submitted.\n"
                "You may be able to recover by using your back\n"
                "button and correcting your data (or may not).\n\n";

            h->data = new_strbuf(regex_failure_hdr, 0);
            content_type(h, "text/plain");
            h->regex_check = failed;
        }

        char* error_msg;
        asprintf(&error_msg, "attribute %s failed regex_pattern %s rc=%d\n\n",
             rule->fieldname, rule->regex_pattern, rc);

        strbuf_append(h->data, new_strbuf(error_msg,0));

        fprintf(h->log, "%f %d %s:%d "
            "fail attribute \"%s\" val [%s] regex_pattern %s rc=%d\n\n",
            gettime(), h->request_id, __func__, __LINE__,
            rule->fieldname, (char*)val, rule->regex_pattern, rc);
        
        free(error_msg);
    }
    free_prompt_rule(h, rule);
}

/*
 *  valid_pkey_value_scanner
 *
 *  For each value+primary key record saved in the form record,
 *  check that the value posted, if any, matches.
 */
void valid_pkey_value_scanner(void* payload, void* data, xmlChar* name){
    char* saved_value = payload;
    struct handler_args* hargs = data;

    char* posted_value = xmlHashLookup(hargs->postdata, name);

    if (has_data(posted_value) && (strcmp(saved_value, posted_value) != 0)){
        hargs->pkey_check = failed;

        fprintf(hargs->log, "%f %d %s:%d fail primary key validation key=%s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            name);
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

    xmlHashScan(h->postdata, validate_regex, h);

    if (h->regex_check == failed){
        error_page(h, SC_BAD_REQUEST, "fail");
        return false;
    }else{
        h->regex_check = passed;
    }

    struct form_record* form_rec = get_posted_form_record(h);
    if ((form_rec != NULL) && (form_rec->pkey_values != NULL)){
        xmlHashScan(form_rec->pkey_values, valid_pkey_value_scanner, h);

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

