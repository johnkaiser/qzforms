
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


void menupage( struct handler_args* h ){

    xmlNodePtr cur;
    xmlNodePtr divqz;

    h->doc = doc_from_file(h, "base.xml");
    content_type(h, "text/html");

    if ((cur = xmlDocGetRootElement(h->doc)) == NULL){
        fprintf(h->log, "%f %d %s:%d xml root element not found\n", 
            gettime(), h->request_id, __func__, __LINE__); 

        error_page(h, SC_EXPECTATION_FAILED,  "xml root element not found");
        return;
    }

    if ((divqz = qzGetElementByID(h, cur, "qz")) == NULL){
        fprintf(h->log, "%f %d %s:%d Element with id qz not found\n", 
            gettime(), h->request_id, __func__, __LINE__); 

        error_page(h, SC_EXPECTATION_FAILED,  "Element with id qz not found");
        return;
    }
    xmlNewTextChild(divqz, NULL, "h1", "Menu");

    //add_helpful_text(h, h->page_ta, cur);
    add_all_menus(h, cur);

    return;
}

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
        "SELECT menu_name, menu_item_sequence, target_form_name, "
        "action, menu_text, context_parameters "
        "FROM qz.menu_item "
        "WHERE menu_name = $1 "
        "ORDER BY menu_item_sequence ";

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
        "SELECT DISTINCT "
        "s.menu_name, m.target_div, m.description "
        "FROM qz.menu_set s "
        "JOIN qz.menu m ON ( m.menu_name = s.menu_name ) "
        "WHERE s.host_form_name = $1 "
        "AND (s.action = 'any' OR s.action = $2)";

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

   char menu_item_parameters[] = 
        "SELECT parameter_key, parameter_value "
        "FROM qz.menu_item_parameter "
        "WHERE menu_name = $1 "
        "AND menu_item_sequence = $2 ";

   rs = PQprepare(hargs->session->conn, "fetch_menu_item_parameters", 
       menu_item_parameters, 0, NULL);

   error_msg = nlfree_error_msg(rs);

   fprintf(hargs->log, "%f %d %s:%d %s %s %s\n",
       gettime(), hargs->request_id, __func__, __LINE__,
       "fetch_menu_item_parameters", PQresStatus(PQresultStatus(rs)), error_msg );
   
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
   
   rs = PQprepare(hargs->session->conn, "menu_exists_check", menu_exists_check, 0, NULL);
   error_msg = nlfree_error_msg(rs);

   fprintf(hargs->log, "%f %d %s:%d %s %s %s\n",
       gettime(), hargs->request_id, __func__, __LINE__,
       "menu_exists_check", PQresStatus(PQresultStatus(rs)), error_msg );

   free(error_msg);
   error_msg = NULL;
   PQclear(rs);
   rs = NULL;

}

/*
 *  add_menu
 *
 *  Add one menu to the indicated node.
 *
 *  A menu is a set of one or more button choices. 
 *  Each menu item contains one html form with data from
 *  current postdata in html hidden fields for each 
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

        // For each row in _rs add a form ...
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

        // ... that has the data it needs ...
        // ... from the current context ...
        char** params = parse_pg_array(get_value(menu_rs, row, "context_parameters"));
        if (params != NULL){
            int p;
            for (p=0; params[p]!=NULL; p++){
               // ... including data from the current request ... 

               char* fvalue = xmlHashLookup(hargs->postdata, params[p]);
               if (fvalue != NULL){

                   xmlNodePtr input_el = xmlNewChild(form, NULL, "input", NULL);
                   xmlNewProp(input_el, "type", "hidden");
                   xmlNewProp(input_el, "name", params[p]);
                   xmlNewProp(input_el, "value", fvalue);

               }else{

                   // This is probably bad.
                   // If I had more time I might make this a popup menu,
                   // but I need it to work first. XXXXXXX
                   fprintf(hargs->log, "%f %d %s:%d fail param %s "
                       "on not found in input\n",
                       gettime(), hargs->request_id, __func__, __LINE__,
                       params[p]); 

               }
            }
        }
        // ... and from the parameter values stored in pg ...
        PGresult* item_param_rs;
        char* query_parameters[3];
        query_parameters[0] = get_value(menu_rs, row, "menu_name");
        query_parameters[1] = get_value(menu_rs, row, "menu_item_sequence");
        query_parameters[2] = NULL;

        item_param_rs = PQexecPrepared(hargs->session->conn, 
          "fetch_menu_item_parameters", 2, (const char * const *)query_parameters,
          NULL, NULL, 0);

        int p_row;
        for(p_row=0; p_row < PQntuples(item_param_rs); p_row++){
            xmlNodePtr p_input = xmlNewChild(form, NULL, "input", NULL);
            xmlNewProp(p_input, "type", "hidden");
            char* key =  get_value(item_param_rs, p_row, "parameter_key");
            xmlNewProp(p_input, "name", key);
            char* p_value = get_value(item_param_rs, p_row, "parameter_value");
            xmlNewProp(p_input, "value", p_value); 
        }
        PQclear(item_param_rs);

        // ... that is the menu button.
        xmlNodePtr submit_button = xmlNewChild(form, NULL, "input", NULL);
        xmlNewProp(submit_button, "type", "submit");
        xmlNewProp(submit_button, "value", get_value(menu_rs, row, "menu_text"));
        append_class(submit_button, "menu_button");

        register_form(hargs, form, SUBMIT_MULTIPLE, action_target);

        free(action_target);
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
void add_all_menus(struct handler_args* hargs, xmlNodePtr root_node){

    double start_time = gettime();

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
        return;
    }    


    // Each row returned is the menu name and target div for one menu.
    int row;
    for (row=0; row < PQntuples(menu_set_rs); row++){
        char* menu_name = get_value(menu_set_rs, row, "menu_name");
        char* target_div = get_value(menu_set_rs, row, "target_div");
        
        xmlNodePtr add_here = qzGetElementByID(hargs, root_node, target_div);

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
        menu_item_rs = PQexecPrepared(hargs->session->conn, "fetch_menu_items", 1,
            (const char* const* ) menu_params, NULL, NULL, 0);

        if (PQntuples(menu_item_rs) < 1){

            char* error_msg = nlfree_error_msg(menu_item_rs);

            fprintf(hargs->log, "%f %d %s:%d fail - menu_item %s not found %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                menu_name, error_msg);

            free(error_msg);

        }else{
         
            add_menu(hargs, menu_item_rs, add_here); 
            fprintf(hargs->log, "%f %d %s:%d menu %s added\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                menu_name);

        }
        
        PQclear(menu_item_rs);
    }

    fprintf(hargs->log, "%f %d %s:%d completed in %f\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        gettime() - start_time);
}

    
