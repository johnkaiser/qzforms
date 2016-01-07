
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
 *  add_delete
 *
 *  Adds a delete button if there is a table action delete.
 *  No confirmation, that should be done in Javascript.
 */

void add_delete(struct handler_args* h, xmlNodePtr here, PGresult* rs){
    
    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    struct table_action* delete_ta = open_table(h, form_name, "delete");

    // delete is optional, missing action is not an error.
    if (delete_ta == NULL) return;

    // Can't delete without a primary key
    if (delete_ta->nbr_pkeys < 1) return;

    char* action_target;
    char* uri_parts[] = {
        h->uri_parts[0], // qz
        form_name,
        "delete",
        NULL
    };
    action_target = build_path(uri_parts);

    xmlNodePtr del_form = xmlNewChild(here, NULL, "form", NULL);
    xmlNewProp(del_form, "method", "post");
    xmlNewProp(del_form, "action", action_target);
    xmlNewProp(del_form, "name", "delete_it");
    xmlNewProp(del_form, "id", "delete_it");
    xmlNewProp(del_form, "enctype", "application/x-www-form-urlencoded");
   
    // register_form adds a record about the form in a table and adds 
    // a hidden input field named "form_tag" that must be returned and 
    // validated before any http post data is accepted.
    register_form(h, del_form, SUBMIT_MULTIPLE, action_target);

    // Add a hidden field for each primary key.
    int pcnt;
    for (pcnt=0; pcnt<delete_ta->nbr_pkeys; pcnt++){
        char* fname = delete_ta->pkeys[pcnt];
        char* fvalue = PQgetvalue(rs, 0, PQfnumber(rs, fname));
        xmlNodePtr pk_input = xmlNewChild(del_form, NULL, "input", NULL);
        xmlNewProp(pk_input, "type", "hidden");
        xmlNewProp(pk_input, "name", fname);
        xmlNewProp(pk_input, "id", fname);
        xmlNewProp(pk_input, "value", fvalue);
    }

    xmlNodePtr button = xmlNewChild(del_form, NULL, "input", NULL);
    xmlNewProp(button, "type", "submit");
    xmlNewProp(button, "value", "Delete");
    
    free(action_target);
    return;
}

/*
 *  edit_form
 * 
 *  Turn a result set with a single row into an update form.
 *  Called by both insert and update actions.
 *  Result must be created and freed by calling function.
 */
void edit_form(struct handler_args* h, char* next_action, 
    struct table_action* edit_ta, PGresult* edit_rs, char* form_name, 
    xmlNodePtr divqz){

    xmlNodePtr form; 
    xmlNodePtr input;

    xmlNewTextChild(divqz, NULL, "h2", form_name);

    xmlNodePtr root_el = xmlDocGetRootElement(h->doc);
    add_helpful_text(h, edit_ta, root_el);

    add_delete(h, divqz, edit_rs);

    char* form_target;

    asprintf(&form_target, "/%s/%s/%s", 
        get_uri_part(h, QZ_URI_BASE_SEGMENT),
        get_uri_part(h, QZ_URI_FORM_NAME),
        next_action);

    form = xmlNewChild(divqz, NULL, "form", NULL);
    xmlNewProp(form, "action", form_target);
    xmlNewProp(form, "method", "post");
    char* action_name;
    asprintf(&action_name, "%s_it", next_action);
    xmlNewProp(form, "name", action_name);
    xmlNewProp(form, "id", action_name);
    free(action_name);
    action_name = NULL;

    xmlNewProp(form, "enctype", "application/x-www-form-urlencoded");

    register_form(h, form, SUBMIT_MULTIPLE, form_target);

    int col;

    for(col=0; col<PQnfields(edit_rs); col++){

        char* fname = PQfname(edit_rs,col);
        char* fvalue = PQgetvalue(edit_rs,0,col);

        struct prompt_rule* rule;
        rule = fetch_prompt_rule(h, form_name, fname);
        
        struct pgtype_datum * pgtype;

        // table  = schema + table 
        char* table_name;
        if ( edit_ta->schema_name != NULL ){
            asprintf(&table_name, "%s.%s", edit_ta->schema_name, 
                edit_ta->table_name);
        }else if( edit_ta->table_name != NULL ){
            asprintf(&table_name, "%s", edit_ta->table_name); 
        }else{    
            asprintf(&table_name, "%s", form_name);
        }    

        pgtype = get_pgtype_datum(h, table_name, fname);
        free(table_name);
        char** options = fetch_options(h, pgtype, rule, fname);
        add_prompt(h, edit_ta, rule, pgtype, options, NO_ROW_INDEX, form, 
            fname, fvalue);

        free(options);
        free_prompt_rule(h, rule);
    }

    if (next_action != NULL){
        // The submit button 
        input = xmlNewChild(form, NULL, "input", NULL);
        xmlNewProp(input, "type", "submit");
        xmlNewProp(input, "value", "Submit");
    }    
    // XXXXXX add a cancel button maybe.
    
    free(form_target);
}

/*
 *  add_insert_button
 *
 *  Adds an insert button if there is a table action
 *  to construct a create form.
 *  table action create serves an empty or mostly empty record
 *  function onetable_create turns it into a form.
 *  function onetable_insert catches the completed form
 *  table action insert updates the database.
 */
void add_insert_button(struct handler_args* h, xmlNodePtr here){

    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    struct table_action* create_ta = open_table(h,form_name,"create");

    // There is no requirement that adding new rows is enabled. Failure OK.
    if (create_ta == NULL) return;
 
    char* action_target;
    asprintf(&action_target, "/%s/%s/%s", 
         get_uri_part(h, QZ_URI_BASE_SEGMENT), form_name, "create");

    xmlNodePtr form = xmlNewChild(here, NULL, "form", NULL);

    // Add an insert prompt for each attribute of the primary key
    // Not needed if the create action automagically creates the key,
    // for example, a sequence.
    int pcnt;
    for (pcnt=0; pcnt<create_ta->nbr_pkeys; pcnt++){
        char* fname = create_ta->pkeys[pcnt];
        struct prompt_rule* rule;
        struct pgtype_datum* pgtype;
        char* table_name;

        rule = fetch_prompt_rule(h, form_name, fname);
        if ( create_ta->schema_name != NULL ){
            asprintf(&table_name, "%s.%s", create_ta->schema_name, 
                create_ta->table_name);
        }else if( create_ta->table_name != NULL ){
            asprintf(&table_name, "%s", create_ta->table_name); 
        }else{    
            asprintf(&table_name, "%s", form_name);
        }    

        pgtype = get_pgtype_datum(h, table_name, fname);
        free(table_name);

        char** options = fetch_options(h, pgtype, rule, fname);

        // If the fieldname is in postdata then fill in the value
        // and because it is a primary key it will be readonly.
        char* fvalue = xmlHashLookup(h->postdata, fname);

        add_prompt(h, create_ta, rule, pgtype, options, NO_ROW_INDEX, form,
            fname, fvalue);
    }

    xmlNewProp(form, "method", "post");
    xmlNewProp(form, "action", action_target);
    xmlNewProp(form, "name", "insert");
    xmlNewProp(form, "id", "insert");
    xmlNewProp(form, "enctype", "application/x-www-form-urlencoded");

    register_form(h, form, SUBMIT_MULTIPLE, action_target);

    xmlNodePtr button = xmlNewTextChild(form, NULL, "button", "Insert");
    xmlNewProp(button, "type", "submit");

    free(action_target);
}

/*
 *  onetable_getall
 * 
 *  Turn a table_action into a select list
 *  and turn that into an edit with an edit button
 *  for each line.
 */
void onetable_getall(struct handler_args* h, char* form_name, xmlNodePtr divqz){

    int col;
    int row;
    int k;

    content_type(h, "text/html");

    struct table_action* getall_ta = open_table(h, form_name, "getall");
    if (getall_ta==NULL){
        fprintf(h->log, "%f %d %s:%d fail table_action %s getall not found\n",
            gettime(), h->request_id, __func__, __LINE__, form_name); 

        error_page(h, SC_BAD_REQUEST, "Not Found");
        return;
    }

    fprintf(h->log, "%f %d %s:%d perform_action with table_action(%s,%s)\n",
        gettime(), h->request_id, __func__, __LINE__, 
        form_name, "getall");

    PGresult* getall_rs = perform_post_action(h, getall_ta);
    if (getall_rs == NULL) {
        fprintf(h->log, "%f %d %s:%d perform action from %s getall produced NULL\n",
            gettime(), h->request_id, __func__, __LINE__, form_name); 
            
        error_page(h, SC_EXPECTATION_FAILED, "Null result"); // not expected.
        return;
    }

    fprintf(h->log, "%f %d %s:%d perform_action  result %s returned %d cols %d rows\n", 
        gettime(), h->request_id, __func__, __LINE__, 
        PQresStatus(PQresultStatus(getall_rs)),
        PQnfields(getall_rs), PQntuples(getall_rs));

    xmlNewTextChild(divqz, NULL, "h2", form_name);

    // Add helpful_text 
    xmlNodePtr root_el = xmlDocGetRootElement(h->doc);
    add_helpful_text(h, getall_ta, root_el);

    add_insert_button(h, divqz);

    // Show passed in parameters.
    if (getall_ta->nbr_params > 0){
        xmlNodePtr dl = xmlNewChild(divqz, NULL, "dl", NULL);
        xmlNewProp(dl, "class", "parameters");
        int p;
        // for each parameter
        for (p=0; p<getall_ta->nbr_params; p++){

             // add prompt name in a dt
             char* fname = getall_ta->fieldnames[p];
             if (fname == NULL) break;  // should not happen

             xmlNodePtr dt = xmlNewTextChild(dl, NULL, "dt", fname);

             // add value in a dd
             char* value = xmlHashLookup(h->postdata, fname);
             if (value != NULL){
                 xmlNodePtr dd = xmlNewTextChild(dl, NULL, "dd", value);
             }    
        }
    }

    // In order to have an edit button, there must be a
    // table action for edit.
    bool has_edit_button = false;
    struct table_action* getone_ta = open_table(h, form_name, "edit");

    if (getone_ta != NULL) has_edit_button = true;

    xmlNodePtr table = xmlNewChild(divqz, NULL, "table", NULL);
    xmlNewProp(table, "class", "tablesorter");
    xmlNewProp(table, "named", "getall");
    xmlNewProp(table, "id", "getall");
    xmlNodePtr tr;

    // Header Row
    xmlNodePtr thead = xmlNewChild(table, NULL, "thead", NULL);
    tr = xmlNewChild(thead, NULL, "tr", NULL);

    if (has_edit_button){
        // first column is the doit button.
        xmlNewTextChild(tr, NULL, "th", "Edit");
    }    

    for(col=0; col<PQnfields(getall_rs); col++){
        xmlNewTextChild(tr, NULL, "th", PQfname(getall_rs,col));
    }

    // Table Body 
    xmlNodePtr form;
    xmlNodePtr input;
    xmlNodePtr td;
    xmlNodePtr tbody;
    int form_nbr = 0;
    char* form_prop_name;

    char* action_target;
    char* uri_parts[4];
    uri_parts[0] = get_uri_part(h, QZ_URI_BASE_SEGMENT);
    uri_parts[1] = form_name;
    uri_parts[2] = "edit";
    uri_parts[3] = NULL;
    
    action_target = build_path(uri_parts);
    struct form_record* form_tag = NULL;

    tbody = xmlNewChild(table, NULL, "tbody", NULL);
    for(row=0; row<PQntuples(getall_rs); row++){
        tr = xmlNewChild(tbody, NULL, "tr", NULL);

        // The edit button
        if (has_edit_button){
            td = xmlNewChild(tr, NULL, "td", NULL);
            form = xmlNewChild(td, NULL, "form", NULL);
            xmlNewProp(form, "action", action_target);
            xmlNewProp(form, "method", "post");
            xmlNewProp(form,"enctype","application/x-www-form-urlencoded");

            asprintf(&form_prop_name, "edit%d",  form_nbr++);
            xmlNewProp(form, "name", form_prop_name);
            xmlNewProp(form, "id", form_prop_name);
            free(form_prop_name);

            // Make a new form_tag for the 1st edit row,
            // and clone it for all the other edit rows.
            if (form_tag == NULL){
                //  XXXXXX get timeout and submit_only_once flag from pg
                form_tag = register_form(h, form, SUBMIT_MULTIPLE, 
                    action_target);

            }else{
                duplicate_registration(h, form_tag, form);
            }
    
            // add a hidden field for each part of the primary key
            for(k=0; k<getall_ta->nbr_pkeys; k++){
                input = xmlNewChild(form, NULL, "input", NULL);
                xmlNewProp(input, "type", "hidden");
                int f_nbr = PQfnumber(getall_rs, getall_ta->pkeys[k]);
                if (PQfname(getall_rs, f_nbr) != NULL){
                    xmlNewProp(input, "name", getall_ta->pkeys[k]);

                    char* key_name_nbr;
                    asprintf(&key_name_nbr, "%s[%d]",getall_ta->pkeys[k], row); 
                    xmlNewProp(input, "id", key_name_nbr);
                    free(key_name_nbr);

                    xmlNewProp(input, "value", 
                        PQgetvalue(getall_rs,row, f_nbr));

                }else{
                    // The primary key was not in the returned data, but
                    // it was requested.  Perhaps it's in the passed in data.
                    char* passed_in = xmlHashLookup(h->postdata, 
                        getall_ta->pkeys[k]);

                    if (passed_in != NULL){
                        xmlNewProp(input, "name", getall_ta->pkeys[k]);

                        char* key_name_nbr;
                        asprintf(&key_name_nbr, "%s[%d]",
                            getall_ta->pkeys[k], row); 

                        xmlNewProp(input, "id", key_name_nbr);
                        free(key_name_nbr);

                        xmlNewProp(input, "value", passed_in);
                    } 
                 }   
             }    

            // The button itself
            input = xmlNewChild(form, NULL, "input", NULL);
            xmlNewProp(input, "type", "submit");
            xmlNewProp(input, "value", "Edit");
        }

        // The rest of the columns
        for(col=0; col<PQnfields(getall_rs); col++){
            char* col_val = PQgetvalue(getall_rs,row,col);
            char* fname = PQfname(getall_rs,col);

            if (col_val != NULL){
                td = xmlNewTextChild(tr, NULL, "td", col_val);
            }else{
                td = xmlNewChild(tr, NULL, "td", NULL);
            }

            int pk;
            for (pk=0; pk<getall_ta->nbr_pkeys; pk++){
                if (strncmp(fname, getall_ta->pkeys[pk], PG_NAMEDATALEN) == 0){
                    append_class(td, "pkey");
                }
            }
            append_class(td, fname);
        } // col
    } // row

    free(action_target);
    PQclear(getall_rs);
    return;
}

/*
 *  onetable_edit
 *
 *  Send a form, loaded with one row of data.
 *  URI is like /qz/form_name/edit
 *  Record key is in post data. 
 */
void onetable_edit(struct handler_args* h, char* form_name, xmlNodePtr divqz){
    
    struct table_action* edit_ta = open_table(h, form_name, "edit");
    if (edit_ta == NULL){
        fprintf(h->log,"%f %d open_table %s, edit) failed\n", 
            gettime(), h->request_id, form_name);

        error_page(h, SC_BAD_REQUEST, "form_name not found");
        return;
    }
   
    PGresult* edit_rs = perform_post_action(h, edit_ta); 

    if (edit_rs == NULL){
        fprintf(h->log, "%f %d %s %d fail action returned null\n", 
        gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_EXPECTATION_FAILED, "action returned null");
        return;
    }

    fprintf(h->log, "%f %d %s %d perform action returned %d rows\n", 
        gettime(), h->request_id, __func__, __LINE__, PQntuples(edit_rs));

    // not found.
    if (PQntuples(edit_rs) == 0){
        
        fprintf(h->log, "%f %d %s %d Record not found.\n",
            gettime(), h->request_id, __func__, __LINE__ );

        error_page(h, SC_EXPECTATION_FAILED, "Record not found.");
        PQclear(edit_rs);
        return;
    }
    // Exactly 1. PQgetvalue can be called for row 0.
    if (PQntuples(edit_rs) != 1){
        
        fprintf(h->log, "%f %d %s %d Wrong data count %d should be 1.\n",
            gettime(), h->request_id, __func__, __LINE__,
            PQntuples(edit_rs));

        error_page(h, SC_EXPECTATION_FAILED, "Wrong data count");
        PQclear(edit_rs);
        return;
    }

    // Now turn that into a form.
    edit_form(h, "update", edit_ta, edit_rs, form_name, divqz);

    PQclear(edit_rs);
}

/*
 *  onetable_create
 * 
 *  Create a new record to serve as an insert dialog.
 */  
void onetable_create(struct handler_args* h, char* form_name, xmlNodePtr divqz){

    struct table_action* create_ta = open_table(h, form_name, "create");
   
    if (create_ta == NULL){

        fprintf(h->log, "%f %d %s:%d create_ta is null from %s, %s\n", 
            gettime(), h->request_id,  __func__, __LINE__, 
            form_name, "create");

        error_page(h,SC_EXPECTATION_FAILED, "Not Found");
        return; 
    }

    PGresult* create_rs = perform_post_action(h, create_ta);

    if (create_rs == NULL){

        fprintf(h->log, "%f %d %s %d fail action returned null\n", 
            gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_EXPECTATION_FAILED, "action returned null");
        return;
    }

    // Exactly 1. PQgetvalue can be called for row 0.
    if (PQntuples(create_rs) != 1){
        
        fprintf(h->log, "%f %d %s %d Wrong data count %d should be 1.\n",
            gettime(), h->request_id, __func__, __LINE__,
            PQntuples(create_rs));

        error_page(h, SC_EXPECTATION_FAILED, "Wrong data count");
        PQclear(create_rs);
        return;
    }

    edit_form(h, "insert", create_ta, create_rs, form_name, divqz);

    PQclear(create_rs);
}

/*
 *  onetable_insert
 *
 *  Call the insert table action to add a new row to a table.
 *  Called after onetable_create and user submit.
 */
void onetable_insert(struct handler_args* h, char* form_name, xmlNodePtr divqz){

    struct table_action* insert_ta = open_table(h, form_name, "insert");
   
    if (insert_ta == NULL){
        fprintf(h->log, "%f %d %s:%d insert_ta is null from %s, %s\n", 
            gettime(), h->request_id,  __func__, __LINE__, 
            form_name, "insert");

        error_page(h,SC_EXPECTATION_FAILED, "Not Found");
        return;
    }

    // This is it right here.
    PGresult* insert_rs = perform_post_action(h, insert_ta);

    if (insert_rs == NULL){
        fprintf(h->log, "%f %d %s %d fail action returned null\n", 
        gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_EXPECTATION_FAILED, "action returned null");
        return;
    }

    if ( (PQresultStatus(insert_rs) == PGRES_TUPLES_OK) 
         ||
         (PQresultStatus(insert_rs) == PGRES_COMMAND_OK) ){
        // Yeah, it worked.

        // Redisplay the first screen.
        onetable_getall(h, form_name, divqz);

    }else{

        char* err_msg = PQresultErrorMessage(insert_rs);
        xmlNodePtr insert_error = xmlNewTextChild(divqz, NULL, "pre", err_msg);
        append_class(insert_error, "err_msg");

        // Re-do the insert 
        // XXXXXX How to return the form with the given data?
        onetable_create(h, form_name, divqz);
    }

    PQclear(insert_rs);
    return;
}
/*
 *  onetable_update
 *
 *  Update one row from the post data.
 */
void onetable_update(struct handler_args* h, char* form_name, xmlNodePtr divqz){

    struct table_action* update_ta = open_table(h, form_name, "update");
   
    if (update_ta == NULL){
        fprintf(h->log, "%f %d %s:%d update_ta is null from %s, %s\n", 
            gettime(), h->request_id,  __func__, __LINE__, 
            form_name, "update");

        error_page(h,SC_EXPECTATION_FAILED, "Not Found");
        return;
    }

    // This is it right here.
    PGresult* update_rs = perform_post_action(h, update_ta);

    fprintf(h->log, "%f %d %s:%d table action returned %s %s\n", 
            gettime(), h->request_id,  __func__, __LINE__, 
            PQresStatus(PQresultStatus(update_rs)),
            PQcmdStatus(update_rs));

    if ( (PQresultStatus(update_rs) == PGRES_TUPLES_OK) 
         ||
         (PQresultStatus(update_rs) == PGRES_COMMAND_OK) ){
        // Yeah, it worked.

        // Redisplay the first screen.
        onetable_getall(h, form_name, divqz);

    }else{

        char* err_msg = PQresultErrorMessage(update_rs);
        xmlNodePtr update_error = xmlNewTextChild(divqz, NULL, "pre", err_msg);
        append_class(update_error, "err_msg");

        // Re-do the edit
        // XXXXXXXXX include the provided data somehow.
        onetable_edit(h, form_name, divqz);
    }
    
    PQclear(update_rs);
}

/*
 *  onetable_delete
 *
 *  Call the delete table action.
 */
void onetable_delete(struct handler_args* h, char* form_name, xmlNodePtr divqz){

   struct table_action* delete_ta = open_table(h, form_name, "delete");
   
   if (delete_ta == NULL){
        fprintf(h->log, "%f %d %s:%d delete_ta is null from %s, %s\n", 
            gettime(), h->request_id,  __func__, __LINE__, 
            form_name, "delete");
        error_page(h,SC_EXPECTATION_FAILED, "Not Found");
        return;
    }
  
    PGresult* delete_rs = perform_post_action(h, delete_ta);

    if (delete_rs == NULL){
        fprintf(h->log, "%f %d %s %d fail action returned null\n", 
        gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_EXPECTATION_FAILED, "action returned null");
        return;
    }

    if ( (PQresultStatus(delete_rs) == PGRES_TUPLES_OK) 
         ||
         (PQresultStatus(delete_rs) == PGRES_COMMAND_OK) ){
        // Yeah, it worked.

    }else{
        char* err_msg = PQresultErrorMessage(delete_rs);
        xmlNodePtr delete_error = xmlNewTextChild(divqz, NULL, "pre", err_msg);
        append_class(delete_error, "err_msg");
    }
    // Redisplay the first screen.
    onetable_getall(h, form_name, divqz);

    PQclear(delete_rs);
}

/*
 *  onetable
 *
 *  Manage a single Postgres table.
 *  Dispatch to a specific action handler
 */
void onetable(struct handler_args* h){

    // Lookup name is a parameter so that header detail can
    // promiscuously call onetable functions.
    // This turned out to be unnecessary.

    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    char* action = get_uri_part(h, QZ_URI_ACTION);
    struct table_action* this_ta = open_table(h, form_name, action);

    if (this_ta == NULL){
        fprintf(h->log, "%f %d %s:%d fail table_action (%s,%s) not found\n", 
            gettime(), h->request_id, __func__, __LINE__,
            form_name, action);

        error_page(h, SC_BAD_REQUEST, "no action");
        return;
    }    

    h->doc = doc_from_file(h, this_ta->xml_template);
    if (h->doc == NULL){
        // logged by doc_from_file
        error_page(h, 500,  "xml conversion failure");
        return;
    }
    xmlNodePtr root_el;
    if ((root_el = xmlDocGetRootElement(h->doc)) == NULL){
        fprintf(h->log, "%f %d %s:%d fail xml root element not found\n", 
            gettime(), h->request_id, __func__, __LINE__); 

        error_page(h, SC_EXPECTATION_FAILED,  "xml document open failure");
        return;
    }
    content_type(h, "text/html");

    add_all_menus(h, root_el);

    xmlNodePtr divqz;
    if ((divqz = qzGetElementByID(h, root_el, this_ta->target_div)) == NULL){
        fprintf(h->log, "%f %d %s:%d Element with id %s not found\n", 
            gettime(), h->request_id, __func__, __LINE__, 
            this_ta->target_div); 

        error_page(h, SC_EXPECTATION_FAILED,  "id element not found");
        return;
    }

    // branch to the named action
    if (strcmp("getall", action)==0){
        onetable_getall(h, form_name, divqz);

    }else if (strcmp("edit", action)==0){
        onetable_edit(h, form_name, divqz);

    }else if (strcmp("create", action)==0){
        onetable_create(h, form_name, divqz);

    }else if (strcmp("insert", action)==0){
        onetable_insert(h, form_name, divqz);

    }else if (strcmp("update", action)==0){
        onetable_update(h, form_name, divqz);

    }else if (strcmp("delete", action)==0){
        onetable_delete(h, form_name, divqz);

    }else{
        fprintf(h->log, "%f %d %s:%d unknown action (%s) \n", 
            gettime(), h->request_id, __func__, __LINE__, action);

        error_page(h,400, "unknown action");
    }
}
