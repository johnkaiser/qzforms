
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
 *  menupage
 *
 *  A menu as a handler, a page without a form.
 */

void menupage( struct handler_args* h ){

    xmlNodePtr divqz;

    doc_from_file(h, "base.xml");
    if (h->error_exists) return;

    content_type(h, "text/html");

    if ((divqz = qzGetElementByID(h, "qz")) == NULL){
        fprintf(h->log, "%f %d %s:%d Element with id qz not found\n",
            gettime(), h->request_id, __func__, __LINE__);

        error_page(h, SC_EXPECTATION_FAILED,  "Element with id qz not found");
        return;
    }

    add_helpful_text(h, h->page_ta);
    add_all_menus(h);

    return;
}

/*
 *  form_name_is_menu
 *
 *  Is the form being executed a menupage?
 */

bool form_name_is_menu(struct handler_args* h){

    bool does_exist = false;

    char* params[2];
    params[0] = get_uri_part(h, QZ_URI_FORM_NAME);
    params[1] = NULL;

    PGresult* menu_exists_rs = PQexecPrepared(h->session->conn,
        "menu_exists_check", 1, (const char * const*) params, NULL, NULL, 0);

    char* error_msg = nlfree_error_msg(menu_exists_rs);

    if (PQresultStatus(menu_exists_rs) == PGRES_TUPLES_OK){
        char* m_ex = PQgetvalue(menu_exists_rs, 0, 0);
        does_exist = ( 't' == m_ex[0] );
    }

    fprintf(h->log, "%f %d %s:%d menu_name_exists_rs is %s, %s tuples %d "
        "fields %d (0,0) %s %d\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(menu_exists_rs)),
        error_msg, PQntuples(menu_exists_rs), PQnfields(menu_exists_rs),
        PQgetvalue(menu_exists_rs, 0, 0), does_exist);

    free(error_msg);
    PQclear(menu_exists_rs);
    return does_exist;

}
/*
 *  init_menu
 *
 *  Create prepared statement for menu retrieval.
 */

// XXXXXXXX add error checks to a hard 500 and stop.
void init_menu(struct handler_args* hargs){

    PGresult* rs;

    char menu_items[] =
        "SELECT mi.menu_name, mi.menu_item_sequence, mi.target_form_name, "
        "mi.action, mi.menu_text, m.form_set_name, "
        "  (SELECT count(fp.parameter_key) "
        "   FROM qz.fixed_parameter fp "
        "   WHERE fp.menu_name = mi.menu_name "
        "   AND fp.menu_item_sequence = mi.menu_item_sequence) "
        "   fixed_parameter_count "
        "FROM qz.menu_item mi "
        "JOIN qz.menu m ON (m.menu_name = mi.menu_name) "
        "WHERE mi.menu_name = $1 "
        "ORDER BY mi.menu_item_sequence ";

   rs = PQprepare(hargs->session->conn, "fetch_menu_items", menu_items,
       0, NULL);

   char* error_msg = nlfree_error_msg(rs);
   fprintf(hargs->log, "%f %d %s:%d %s %s %s\n",
       gettime(), hargs->request_id, __func__, __LINE__,
       "fetch_menu_items", PQresStatus(PQresultStatus(rs)), error_msg );

   free(error_msg);
   error_msg = NULL;
   PQclear(rs);
   rs = NULL;

   char menu_set[] =
       "SELECT CASE "
       "WHEN s.menu_name = 'main' THEN "
       "  COALESCE( "
            "(SELECT u.main_menu FROM qz.user u "
            "WHERE u.user_name = current_user), "
       "  'main') "
       "  ELSE s.menu_name "
       "END menu_name, "
       "m.target_div, m.description "
       "FROM qz.menu_set s "
       "JOIN qz.menu m ON ( m.menu_name = s.menu_name ) "
       "WHERE s.host_form_name = $1 "
       "AND (s.action = 'any' OR s.action = $2) "
       "ORDER BY s.menu_name ";

   rs = PQprepare(hargs->session->conn, "fetch_menu_set", menu_set,
       0, NULL);

   error_msg = nlfree_error_msg(rs);
   fprintf(hargs->log, "%f %d %s:%d %s %s %s\n",
       gettime(), hargs->request_id, __func__, __LINE__,
       "fetch_menu_set", PQresStatus(PQresultStatus(rs)), error_msg );

   free(error_msg);
   error_msg = NULL;
   PQclear(rs);
   rs = NULL;

   char fixed_parameter[] =
        "SELECT parameter_key, parameter_value "
        "FROM qz.fixed_parameter "
        "WHERE menu_name = $1 "
        "AND menu_item_sequence = $2 ";

   rs = PQprepare(hargs->session->conn, "fetch_fixed_parameter",
       fixed_parameter, 0, NULL);

   error_msg = nlfree_error_msg(rs);

   fprintf(hargs->log, "%f %d %s:%d %s %s %s\n",
       gettime(), hargs->request_id, __func__, __LINE__,
       "fetch_fixed_parameter", PQresStatus(PQresultStatus(rs)),
       error_msg );

   free(error_msg);
   error_msg = NULL;
   PQclear(rs);
   rs = NULL;

   //
   char menu_exists_check[] =
        "SELECT EXISTS ("
        "SELECT form_name "
        "FROM qz.form "
        "WHERE form_name = $1 "
        "AND handler_name = 'menupage') ";

   rs = PQprepare(hargs->session->conn, "menu_exists_check",
       menu_exists_check, 0, NULL);

   error_msg = nlfree_error_msg(rs);

   fprintf(hargs->log, "%f %d %s:%d %s %s %s\n",
       gettime(), hargs->request_id, __func__, __LINE__,
       "menu_exists_check", PQresStatus(PQresultStatus(rs)), error_msg );

   free(error_msg);
   error_msg = NULL;
   PQclear(rs);
   rs = NULL;

}

struct node_scanner_args {
    struct handler_args* hargs;
    xmlNodePtr form_node;
};

void context_parameter_input_node_scanner(void* payload, void* data,
    xmlChar* name){

    struct node_scanner_args* scan_args = data;
    //struct handler_args* hargs = scan_args->hargs;
    xmlNodePtr form_node = scan_args->form_node;

    char* value = payload;

    xmlNodePtr input_el = xmlNewChild(form_node, NULL, "input", NULL);

    xmlNewProp(input_el, "type", "hidden");
    xmlNewProp(input_el, "name", name);
    xmlNewProp(input_el, "value", value);
}

/*  
 *  add_menu
 *
 *  Add one menu to the indicated node.
 *
 *  A menu is a set of one or more button choices.
 *  Each menu item contains one html form with data from
 *  the form set in html hidden fields for each
 *  pg menu_item parameters array element.
 */

void add_menu(struct handler_args* hargs,
    PGresult* menu_rs,
    xmlNodePtr child_of){

    append_class(child_of, "menu");
    append_class(child_of, get_value(menu_rs, 0, "menu_name"));

    int row;
    xmlNodePtr form;
    for (row=0; row<PQntuples(menu_rs); row++){

        // For each row in _rs add a form.
        form = xmlNewChild(child_of, NULL, "form", NULL);
        char* action_target;

        char* action = get_value(menu_rs, row, "action");
        char* target_form_name = get_value(menu_rs, row, "target_form_name");

        if ((action != NULL) && (action[0] != '\0')){

            asprintf(&action_target, "/%s/%s/%s",
                get_uri_part(hargs, QZ_URI_BASE_SEGMENT),
                target_form_name, action );

        }else{

            asprintf(&action_target, "/%s/%s",
                get_uri_part(hargs, QZ_URI_BASE_SEGMENT),
                target_form_name);

        }
        xmlNewProp(form, "method", "post");
        xmlNewProp(form, "action", action_target);

        char* item_id;
        asprintf(&item_id, "%s[%d]", get_value(menu_rs, row, "menu_name"), row);
        xmlNewProp(form, "name", item_id);
        xmlNewProp(form, "id", item_id);
        free(item_id);

        xmlNewProp(form, "enctype", "application/x-www-form-urlencoded");
        append_class(form, "menu");
        append_class(form, get_value(menu_rs, row, "menu_name"));

        // Add a hidden form field for each context parameter in the form set

        if (hargs->current_form_set != NULL){
   
            // The menu form gets data from the current context. 
            // ZZZZZZZZZ get from page_ta char** context_parameters;
            // ZZZZZZZZZ if page_ta bool clear_context_parameters 
            // ZZZZZZZZZ is set then skip. but first...

            if ( ! form_set_is_valid(hargs, hargs->current_form_set)){ 
                 fprintf(hargs->log, "%f %d %s:%d fail current form set "
                     "is invalid\n",
                     gettime(), hargs->request_id, "add_all_menus", __LINE__);

                 error_page(hargs, SC_INTERNAL_SERVER_ERROR, "bad token");
                 return;
            }     

            char** params = hargs->page_ta->context_parameters;
            
            if (params != NULL){
                int p;
                for (p=0; params[p]!=NULL; p++){
   
                    char* pvalue = xmlHashLookup(
                      hargs->current_form_set->context_parameters, params[p]);
                    
                    if (pvalue != NULL){
   
                      xmlNodePtr input_el = xmlNewChild(form, NULL, "input", NULL);
                      xmlNewProp(input_el, "type", "hidden");
                      xmlNewProp(input_el, "name", params[p]);
                      xmlNewProp(input_el, "value", pvalue);
   
                    }else{
   
                      // This is probably bad.
                      fprintf(hargs->log, "%f %d %s:%d warning menu %s param %s "
                          "not found in input\n",
                          gettime(), hargs->request_id, __func__, __LINE__,
                          get_value(menu_rs, row, "menu_name"), params[p]);
                    }
                }
            }
        }

        // Add fixed parameters, if any, as hidden input elements.
        int fixed_parameter_count = atoi(
            get_value(menu_rs, row, "fixed_parameter_count")
        );

        if (fixed_parameter_count > 0){
            char* fixed_params[3];
            fixed_params[0] = get_value(menu_rs, row, "menu_name");
            fixed_params[1] = get_value(menu_rs, row, "menu_item_sequence");
            fixed_params[2] = NULL;
    
            PGresult* fixed_params_rs = PQexecPrepared(hargs->session->conn, 
                "fetch_fixed_parameter", 2, 
                (const char * const *) fixed_params, 
                NULL, NULL, 0);
    
            if ((PQresultStatus(fixed_params_rs) == PGRES_TUPLES_OK) &&
                (PQntuples(fixed_params_rs) > 0)){
    
               int fpn;
               for(fpn=0; fpn < PQntuples(fixed_params_rs); fpn++){
    
                   xmlNodePtr fp_node = xmlNewChild(form, NULL, "input", NULL);
                   xmlNewProp(fp_node, "type", "hidden");
                   xmlNewProp(fp_node, "name", 
                       get_value(fixed_params_rs, fpn, "parameter_key"));
    
                   xmlNewProp(fp_node, "value", 
                       get_value(fixed_params_rs, fpn, "parameter_value"));
               }
            }
        }    

        // The menu form has a button.
        xmlNodePtr submit_button = xmlNewChild(form, NULL, "input", NULL);
        xmlNewProp(submit_button, "type", "submit");
        xmlNewProp(submit_button, "value", get_value(menu_rs,row,"menu_text"));
        append_class(submit_button, "menu_button");

        struct form_record* form_rec;
        form_rec = register_form(hargs, form, SUBMIT_MULTIPLE, action_target);

        free(action_target);
    } // row

}

/*
 *  log_context_variables
 *
 *  Run through the context variables and log them.
 */

void context_variable_logging_scanner(void* payload, void* data, xmlChar* name){
 
     struct handler_args* hargs = data;
     char* value = payload;

     fprintf(hargs->log, "%f %d %s:%d context_variable=%s value=%s\n",
         gettime(), hargs->request_id, "add_all_menus", __LINE__,
         name, value);
}
void log_context_variables(struct handler_args* hargs){
 
     if ((hargs->current_form_set != NULL) && 
         (hargs->current_form_set->context_parameters != NULL)){

         fprintf(hargs->log, "%f %d %s:%d context_paramters has %d items\n",
             gettime(), hargs->request_id, __func__, __LINE__,
             xmlHashSize(hargs->current_form_set->context_parameters));
         
         xmlHashScan(hargs->current_form_set->context_parameters, 
             context_variable_logging_scanner,
             hargs);
     }else{
         fprintf(hargs->log, "%f %d %s:%d form_set or form_set->"
             "context_parameters is null\n",
             gettime(), hargs->request_id, __func__, __LINE__);
     }
}
 

/*
 *  add_all_menus
 *
 *  Add all the menus identified in pg menu_set
 *  to the existing doc. If the target div identified
 *  in pg menu is not found then don't change anything
 *  but return as though everything is fine.
 *
 */
void add_all_menus(struct handler_args* hargs){

    double start_time = gettime();

    if ((hargs->current_form_set != NULL) && 
        ( ! form_set_is_valid(hargs, hargs->current_form_set) )){

        error_page(hargs, SC_INTERNAL_SERVER_ERROR, "form set invalid");
        return;
    }    
    log_context_variables(hargs);

    // Use menu_set in pg to feed add_menu.
    char* params[3];
    params[0] = get_uri_part(hargs, QZ_URI_FORM_NAME);
    params[1] = get_uri_part(hargs, QZ_URI_ACTION);
    params[2] = NULL;

    PGresult* menu_set_rs;

    menu_set_rs = PQexecPrepared(hargs->session->conn, "fetch_menu_set", 2,
        (const char * const *) params, NULL, NULL, 0);

    if ((menu_set_rs == NULL) ||
        (PQresultStatus(menu_set_rs) != PGRES_TUPLES_OK)){

        char* error_msg = nlfree_error_msg(menu_set_rs);
        fprintf(hargs->log, "%f %d %s:%d fail fetch_menu_set %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            error_msg);

        free(error_msg);
        PQclear(menu_set_rs);
        return;
    }


    // Each row returned is the menu name and target div for one menu.
    int row;
    for (row=0; row < PQntuples(menu_set_rs); row++){
        char* menu_name = get_value(menu_set_rs, row, "menu_name");
        char* target_div = get_value(menu_set_rs, row, "target_div");

        xmlNodePtr add_here = qzGetElementByID(hargs, target_div);

        if (add_here == NULL){

            fprintf(hargs->log, "%f %d %s:%d fail - target_div %s not found\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                target_div);

            return;
        }

        char* menu_params[2];
        menu_params[0] = menu_name;
        menu_params[1] = NULL;

        PGresult* menu_item_rs;
        menu_item_rs = PQexecPrepared(hargs->session->conn, "fetch_menu_items",
            1, (const char* const* ) menu_params, NULL, NULL, 0);

        if (PQntuples(menu_item_rs) < 1){

            char* error_msg = nlfree_error_msg(menu_item_rs);

            fprintf(hargs->log, "%f %d %s:%d fail menu_item %s not found %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                menu_name, error_msg);

            free(error_msg);

        }else{

            add_menu(hargs, menu_item_rs, add_here);

            if (hargs->error_exists) return;

            fprintf(hargs->log, "%f %d %s:%d menu %s added\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                menu_name);
        }

        PQclear(menu_item_rs);
    }
    PQclear(menu_set_rs);
    fprintf(hargs->log, "%f %d %s:%d completed in %f\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        gettime() - start_time);
}


