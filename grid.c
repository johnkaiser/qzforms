
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
 *  grid_update_row
 *
 */
PGresult* grid_update_row(struct handler_args* h, char* form_name, int row){

    struct table_action* grid_update_row_ta;
    grid_update_row_ta = open_table(h, form_name, "update_row");

    if (grid_update_row_ta != NULL){
       return perform_post_row_action(h, grid_update_row_ta, row);

    }
    return NULL;
}


/*
 *  grid_insert_row
 *
 */
PGresult* grid_insert_row(struct handler_args* h, char* form_name, int row){

    struct table_action* grid_insert_row_ta;
    grid_insert_row_ta = open_table(h, form_name, "insert_row");

    if (grid_insert_row_ta != NULL){
        return  perform_post_row_action(h, grid_insert_row_ta, row);
    }
    return NULL;

}


/*
 *  grid_delete_row
 *
 */
PGresult* grid_delete_row(struct handler_args* h, char* form_name, int row){

    struct table_action* grid_delete_row_ta;
    grid_delete_row_ta = open_table(h, form_name, "delete_row");

    if (grid_delete_row_ta != NULL){
        return perform_post_row_action(h, grid_delete_row_ta, row);
    }
    return NULL;

}

/*
 *  table_constant_adder
 *
 *  When grid edit is called with post data passed to it,
 *  add that data to the form so updates have it too.
 *  This is called by xmlHashScan.
 */
struct table_constant_adder_data {
    char* form_name;
    struct handler_args* h;
    struct table_action* t_action;
    xmlNodePtr child_of;
};

void table_constant_adder(void* val, void* data, xmlChar* name){

    struct table_constant_adder_data* d = data;
    char* fieldname = name;
    char* fieldvalue = val;

    // form_tag does not get carried forward.
    if (strcmp(fieldname, "form_tag") == 0) return;

    struct prompt_rule p_rule = {
        .form_name = d->form_name,
        .fieldname = fieldname,
        .prompt_type = "input_hidden",
        .el_class = fieldname,
        .readonly = true
    };

    add_prompt(d->h, d->t_action, &p_rule, NULL, NO_OPTIONS, NO_ROW_INDEX,
        d->child_of, fieldname, fieldvalue);

    return;
}

/*
 *  add_row_form_data
 *
 *  Add Row button support data as a hidden input field.
 */

void add_row_form_data(xmlNodePtr add_row_form,
    struct table_action* grid_insert_row_ta,
    struct prompt_rule* p_rule,
    char** option_ar,
    char* fieldname){

    // Even if the insert_row action is not called right now,
    // its existance or non-existance determines if the call
    // should be possible in the future.
    if (grid_insert_row_ta == NULL) return;

    // ZZZZZZ Give it a default rule and continue
    if (p_rule == NULL){
        struct prompt_rule default_prompt_rule = (struct prompt_rule){
            .form_name = "default",
            .fieldname = fieldname,
            .prompt_type = "input_text",
            .expand_percent_n = true,
            .el_class = "",
            .options = "",
            .onblur = "",
            .onchange = "change_status(%n, 'U')",
            .onclick = "",
            .ondblclick = "",
            .onfocus = "",
            .onkeypress = "",
            .onkeyup = "",
            .onkeydown = "",
            .onmousedown = "",
            .onmouseup = "",
            .onmouseout = "",
            .onmouseover = "",
            .onselect = "",

        };

        p_rule = &default_prompt_rule;
    }


    // For each field in grid_edit_ta, I need the prompt_rule
    // for it in json so the input elements can be made by js.
    // Note that %n expansion is defered to js at execution.

    // Note that insert_row and update_row must have the same
    // attribute names.

    xmlNodePtr input_el;
    input_el = xmlNewChild(add_row_form, NULL, "input", NULL);

    char* el_name;
    asprintf(&el_name, "%s[%%n]", fieldname);
    xmlNewProp(input_el, "name", el_name);
    xmlNewProp(input_el, "id", el_name);
    xmlNewProp(input_el, "type", "hidden");

    char* add_el_args;
    add_el_args = json_add_element_args(NULL, p_rule, option_ar);

    char* add_el_args_esc;
    add_el_args_esc = xmlURIEscapeStr(add_el_args, NULL);

    xmlNewProp(input_el, "prompt_rule", add_el_args_esc);

    free(add_el_args);
    free(add_el_args_esc);

    return;
}
/*
 *  grid_edit
 *
 *  Add an html table where each attribute in the result set
 *  is a cell in the table.
 *
 */
void grid_edit(struct handler_args* h, char* form_name, xmlNodePtr root_el){

    struct table_action* grid_edit_ta = h->page_ta;

    xmlNodePtr divqz;
    if ((divqz = qzGetElementByID(h, root_el, grid_edit_ta->target_div)) == NULL){

        fprintf(h->log, "%f %d %s:%d Element with id %s not found\n",
            gettime(), h->request_id, __func__, __LINE__,
            grid_edit_ta->target_div);

        error_page(h, SC_EXPECTATION_FAILED,  "id element not found");
        return;
    }

    add_helpful_text(h, grid_edit_ta, root_el);

    fprintf(h->log, "%f %d %s:%d perform_action with table_action(%s,%s)\n",
        gettime(), h->request_id, __func__, __LINE__,
        form_name, "edit");

    PGresult* grid_edit_rs = perform_post_action(h, grid_edit_ta);

    if (grid_edit_rs == NULL) {
        fprintf(h->log, "%f %d %s:%d perform action from %s list produced NULL\n",
            gettime(), h->request_id, __func__, __LINE__, form_name);

        error_page(h, SC_EXPECTATION_FAILED, "Null result"); // not expected.
        return;
    }
    int nfields = PQnfields(grid_edit_rs);
    if (nfields < 1){
        fprintf(h->log, "%f %d %s:%d fail grid edit called with nfields=%d\n",
            gettime(), h->request_id, __func__, __LINE__,
            nfields);

        error_page(h, SC_EXPECTATION_FAILED, "Null result"); // not expected.
        return;
    }

    int col;

    xmlNewTextChild(divqz, NULL, "h2", form_name);

    // Collect prompt meta data for easy access later.
    struct pgtype_datum* pgtypes[nfields];
    struct prompt_rule* p_rules[nfields];
    char** option_ar[nfields];

    char* table_name;
    if ( grid_edit_ta->schema_name != NULL ){
        asprintf(&table_name, "%s.%s", grid_edit_ta->schema_name,
            grid_edit_ta->table_name);
    }else if( grid_edit_ta->table_name != NULL ){
        asprintf(&table_name, "%s", grid_edit_ta->table_name);
    }else{
        asprintf(&table_name, "%s", form_name);
    }

    for (col=0; col<nfields; col++){
        char* fname = PQfname(grid_edit_rs, col);

        pgtypes[col] = get_pgtype_datum(h, table_name, fname);
        p_rules[col] = fetch_prompt_rule(h, form_name, fname);
        option_ar[col] = fetch_options(h, pgtypes[col], p_rules[col], fname);
    }

    free(table_name);
    table_name = NULL;

    // The presence of the insert action determines if
    // new rows are allowed to be inserted.
    struct table_action* grid_insert_row_ta;
    grid_insert_row_ta = open_table(h, form_name, "insert_row");


    // Add Rows
    // The add rows button and data go here.
    // See add_row_form_data
    //
    if (grid_insert_row_ta != NULL){
        xmlNodePtr add_row_form = xmlNewChild(divqz, NULL, "form", NULL);
        xmlNewProp(add_row_form, "name", "add_row_form");
        xmlNewProp(add_row_form, "id", "add_row_form");

        xmlNodePtr add_row_button;
        add_row_button = xmlNewTextChild(add_row_form, NULL, "button", "Add Row");

        char* add_row_name;
        asprintf(&add_row_name, "%s_add_row", form_name);

        xmlNewProp(add_row_button, "name", add_row_name);
        xmlNewProp(add_row_button, "id", add_row_name);
        xmlNewProp(add_row_button, "type", "button");

        char* click_event;
        asprintf(&click_event, "grid_add_row()" );
        xmlNewProp(add_row_button, "onclick", click_event);

        // Add a hidden input field to describe each prompt.
        for (col=0; col<nfields; col++){

            add_row_form_data(add_row_form, grid_insert_row_ta, p_rules[col],
                option_ar[col], PQfname(grid_edit_rs, col));
        }


        free(add_row_name);
        free(click_event);
    }


    // Referenced for each row to determine if a delete button is called for.
    struct table_action* grid_delete_row_ta;
    grid_delete_row_ta = open_table(h, form_name, "delete_row");

    // Add form element
    char* action_target;
    asprintf(&action_target, "/%s/%s/save",
        get_uri_part(h, QZ_URI_BASE_SEGMENT),
        form_name);

    xmlNodePtr form = xmlNewChild(divqz, NULL, "form", NULL);
    xmlNewProp(form, "method", "post");
    xmlNewProp(form, "action", action_target);
    xmlNewProp(form, "name", "insert"); // XXXXXX ??? where did this come from?
    xmlNewProp(form, "enctype", "application/x-www-form-urlencoded");

    // XXXXXX SUBMIT_MULTIPLE or SUBMIT_ONLY_ONCE should come from
    // XXXXXX pg qz.form via the table action.
    struct form_record* form_rec;
    form_rec = register_form(h, form, SUBMIT_MULTIPLE, action_target);
    // XXXXXXX The zero here is problematic as a grid has many rows.
    save_context_parameters(h, form_rec, grid_edit_rs, 0);
    free(action_target);
    action_target = NULL;
    if (h->error_exists) return;

    // Add any fields submitted to edit before the table.
    // XXXXXXX This is all fields, should this be limited to fields
    // XXXXXXX used and listed in table_action->fieldnames????

    //Z if parameters exist add a dl container
    if (grid_edit_ta->nbr_params > 0){
        xmlNodePtr dl = xmlNewChild(form, NULL, "dl", NULL);
        xmlNewProp(dl, "class", "parameters");
        int p;
        //Z for each parameter
        for (p=0; p<grid_edit_ta->nbr_params; p++){

             //Z add prompt name in a dt
             char* fname = grid_edit_ta->fieldnames[p];
             if (fname == NULL) break;  // should not happen

             xmlNewTextChild(dl, NULL, "dt", fname);

             //Z add value in a dd
             char* value = xmlHashLookup(h->postdata, fname);
             if (value != NULL){
                 xmlNewTextChild(dl, NULL, "dd", value);
             }
        }
    }

    struct table_constant_adder_data data = {
        .form_name = form_name,
        .h = h,
        .t_action = grid_edit_ta,
        .child_of = form
    };

    xmlHashScan(h->postdata, table_constant_adder, &data);

    // Build an html table with a cell for each attribute.

    xmlNodePtr table = xmlNewChild(form, NULL, "table", NULL);
    xmlNewProp(table, "class", "tablesorter");
    xmlNewProp(table, "id", "grid_edit_table");

    xmlNodePtr tr;
    xmlNodePtr th;
    xmlNodePtr td;

    // Header Row

    xmlNodePtr thead = xmlNewChild(table, NULL, "thead", NULL);
    tr = xmlNewChild(thead, NULL, "tr", NULL);

    th = xmlNewTextChild(tr, NULL, "th", "Change Status");
    append_class(th, "change_status");

    for(col=0; col<PQnfields(grid_edit_rs); col++){
        th = xmlNewTextChild(tr, NULL, "th", PQfname(grid_edit_rs, col));
        xmlNewProp(th, "class", "column_header");
        append_class(th, PQfname(grid_edit_rs, col));

        if ( (p_rules[col] != NULL) &&
            (strcmp(p_rules[col]->prompt_type, "input_hidden") == 0)){

            append_class(th, "input_hidden");
        }

        if ((pgtypes[col] != NULL) &&
            (pgtypes[col]->description[0] != '\0') &&
            (grid_edit_ta->add_description)){

            xmlNewProp(th, "title", pgtypes[col]->description);
        }
    }

    // Add a row for each tuple

    xmlNodePtr tbody = xmlNewChild(table, NULL, "tbody", NULL);
    xmlNewProp(tbody, "id", "grid_edit_tbody");

    int row;
    for(row=0; row<PQntuples(grid_edit_rs); row++){

        tr = xmlNewChild(tbody, NULL, "tr", NULL);

        // Add a td for the change status

        char* change_status;
        asprintf(&change_status, "change_status[%d]", row);

        static char* change_status_str = "change_status";
        static char* input_text = "input_text";

        struct prompt_rule chg_status_rule =
        {
            .form_name = form_name,
            .fieldname = change_status,
            .prompt_type = input_text,
            .el_class = change_status_str,
            .readonly = true,
            .publish_pgtype = false,
            .size = 1
        };
        td = xmlNewChild(tr, NULL, "td", NULL);

        add_prompt(h, grid_edit_ta, &chg_status_rule, NULL, NO_OPTIONS,
            row, td, change_status, "E");

        free(change_status);
        change_status = NULL;

        // Add a delete button
        if (grid_delete_row_ta != NULL){
            char* delete_button_name;
            asprintf(&delete_button_name, "delete_row[%d]", row);
            char* delete_event;
            asprintf(&delete_event, "grid_delete_row(%d,'D')", row);

            xmlNodePtr delete_button = xmlNewTextChild(td, NULL, "button", "delete");
            xmlNewProp(delete_button, "name", delete_button_name);
            xmlNewProp(delete_button, "id", delete_button_name);
            xmlNewProp(delete_button, "type", "button");

            xmlNewProp(delete_button, "onclick", delete_event);

            free(delete_button_name);
            free(delete_event);
        }


        // Add a td for each cell
        int col;
        for(col=0; col<nfields; col++){
            td = xmlNewChild(tr, NULL, "td", NULL);

            // Add an input prompt for each

            // create a name like base[0], base[1], etc.
            char* fname;
            asprintf(&fname, "%s[%d]", PQfname(grid_edit_rs, col), row);

            char* fvalue = PQgetvalue(grid_edit_rs, row, col);

            if (p_rules[col] == NULL){
                fprintf(h->log, "%f %d %s:%d grid p_rule %s is null\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    fname);
            }

            add_prompt(h, grid_edit_ta, p_rules[col], pgtypes[col], option_ar[col],
                row, td, fname, fvalue);

            append_class(td, PQfname(grid_edit_rs, col));

            if ( (p_rules[col] != NULL) &&
                (strcmp(p_rules[col]->prompt_type, "input_hidden") == 0)){

                append_class(td, "input_hidden");
            }

            free(fname);
            fname=NULL;
        }

    }

    // Add a submit button
    xmlNodePtr input = xmlNewChild(form, NULL, "input", NULL);
    xmlNewProp(input, "type", "submit");
    xmlNewProp(input, "value", "Save");


   for(col=0; col<nfields; col++){
       free_prompt_rule(h, p_rules[col]);
       free(option_ar[col]);
   }
}

/*
 *  grid_save
 *
 */
void grid_save(struct handler_args* h, char* form_name, xmlNodePtr root_el){

    // Need the table action not for the action but for the
    // template name.
    struct table_action* grid_save_ta = h->page_ta;
    content_type(h, "text/html");

    xmlNodePtr divqz;
    if ((divqz = qzGetElementByID(h, root_el, grid_save_ta->target_div)) == NULL){
        fprintf(h->log, "%f %d %s:%d Element with id %s not found\n",
            gettime(), h->request_id, __func__, __LINE__,
            grid_save_ta->target_div);

        error_page(h, SC_EXPECTATION_FAILED,  "id element not found");
        return;
    }

    fprintf(h->log, "%f %d %s:%d perform_action with table_action(%s,%s)\n",
        gettime(), h->request_id, __func__, __LINE__,
        form_name, "edit");

    PGresult* grid_save_rs = perform_post_action(h, grid_save_ta);

    if (grid_save_rs == NULL) {
        fprintf(h->log, "%f %d %s:%d perform action from %s list produced NULL\n",
            gettime(), h->request_id, __func__, __LINE__, form_name);

        error_page(h, SC_EXPECTATION_FAILED, "Null result"); // not expected.
        return;
    }

    xmlNewTextChild(divqz, NULL, "h2", form_name);
    save_context_parameters(h, NULL, grid_save_rs, 0);

    // BEGIN
    PQexec(h->session->conn, "BEGIN");
    xmlNodePtr dl = xmlNewChild(divqz, NULL, "dl", NULL);

    int k = 0;
    char* value;
    PGresult* grid_row_rs;
    bool error_exists = false;

    do{
        // There has to be a postdata field "change_status[%n]"  where %n=row.
        char* name;
        asprintf(&name,"change_status[%d]", k);

        value = xmlHashLookup(h->postdata, name);

        if (value != NULL){
            xmlNodePtr dt = xmlNewTextChild(dl, NULL, "dt", value);
            xmlNodeAddContent(dt, " ");

            int pcnt;
            char* key_name;
            for (pcnt=0; pcnt<grid_save_ta->nbr_pkeys; pcnt++){
                asprintf(&key_name, "%s[%d]", grid_save_ta->pkeys[pcnt], k);
                xmlNodeAddContent(dt, xmlHashLookup(h->postdata, key_name));
                xmlNodeAddContent(dt, ", ");
                free(key_name);
            }
            switch(value[0]){
                case 'U':
                    grid_row_rs = grid_update_row(h, form_name, k);
                    break;

                case 'I':
                    grid_row_rs = grid_insert_row(h, form_name, k);
                    break;

                case 'D':
                    grid_row_rs = grid_delete_row(h, form_name, k);
                    break;

                // 'E' and 'X' are no ops.
                case 'E':
                case 'X':
                    grid_row_rs = NULL;
                    xmlNewTextChild(dl, NULL, "dd", "No Action");
                    break;
            }

            if (grid_row_rs != NULL){
                if (PQresultStatus(grid_row_rs) == PGRES_COMMAND_OK){
                    xmlNewTextChild(dl, NULL, "dd", "OK");
                    PQclear(grid_row_rs);
                    grid_row_rs = NULL;
                }else{
                    // Oh No
                    char* err_msg = nlfree_error_msg(grid_row_rs);
                    xmlNewTextChild(dl, NULL, "dd", err_msg);
                    free(err_msg);
                    xmlNewTextChild(dl, NULL, "dd", "ROLLBACK");
                    error_exists = true;
                    break;
                }
            }else if ((value[0] != 'E') && (value[0] != 'X')){// E X not errors.
                error_exists = true;
                fprintf(h->log, "%f %d %s:%d grid_row_rs unexpectedly null\n",
                    gettime(), h->request_id, __func__, __LINE__);

                break;
            }
        }
        free(name);
        k++;
    }while (value != NULL);

    //  ROLLBACK or COMMIT
    PGresult* final_rs;
    if (error_exists){
        final_rs = PQexec(h->session->conn, "ROLLBACK");
    }else{
        final_rs = PQexec(h->session->conn, "COMMIT");
        if (PQresultStatus(final_rs) == PGRES_COMMAND_OK){
            xmlNewTextChild(divqz, NULL, "p", "COMMIT OK");
        }else{
            char* err_msg = nlfree_error_msg(final_rs);
            xmlNewTextChild(divqz, NULL, "p", err_msg);
            free(err_msg);
        }
    }

}


void grid(struct handler_args* h){
 // useful stuff here

    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    char* action = get_uri_part(h, QZ_URI_ACTION);

    if (h->page_ta->xml_template != NULL){
        h->doc = doc_from_file(h, h->page_ta->xml_template);
    }
    if (h->doc == NULL){
        // logged by doc_from_file
        error_page(h, 500,  "xml conversion failure");
        return;
    }
    content_type(h, "text/html");


    xmlNodePtr root_el;
    if ((root_el = xmlDocGetRootElement(h->doc)) == NULL){

        fprintf(h->log, "%f %d %s:%d fail xml root element not found\n",
            gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_EXPECTATION_FAILED,  "xml document open failure");
        return;
    }


    if (strcmp("edit", action) == 0){
        grid_edit(h, form_name, root_el);

    }else if (strcmp("save", action) == 0){
        grid_save(h, form_name, root_el);

    }else{

        fprintf(h->log, "%f %d %s:%d unknown action (%s) \n",
            gettime(), h->request_id, __func__, __LINE__, action);

        error_page(h,400, "unknown action");
    }
    if (! h->error_exists){
        add_all_menus(h, root_el);
    }
}


