
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
 *  req_login
 *
 *  Generate a login request form.
 */
void req_login( struct handler_args* h ){

    xmlNodePtr divqz;
    xmlNodePtr form;
    xmlNodePtr list;
    xmlNodePtr list_item;

    fprintf(h->log, "%f %d %s:%d\n", 
        gettime(), h->request_id, __func__, __LINE__);

    // XXXXX login.xml should be documented somewhere 
    doc_from_file(h, "login.xml");
    if (h->error_exists) return;

    divqz = qzGetElementByID(h, "qz");
    if (divqz != NULL){
        char* action_target;
        char* uri_parts[] = {h->uri_parts[0], "login", "validate", NULL};
        action_target = build_path(uri_parts); 

        form = xmlNewChild(divqz, NULL, "form", NULL);
        xmlNewProp(form, "action", action_target);
        register_form(h, form, SUBMIT_ONLY_ONCE, action_target);
        free(action_target);

        xmlNewProp(form, "method", "post");
        xmlNewProp(form, "id", "login_form");
        xmlNewProp(form, "name", "login_form");
        xmlNewProp(form, "enctype", "application/x-www-form-urlencoded");

     
        list = xmlNewChild(form, NULL, "ul", NULL);
        append_class(list, "login");

        xmlNewTextChild(list, NULL, "li", "user");

        list_item = xmlNewChild(list, NULL, "li", NULL);

        xmlNodePtr user = xmlNewChild(list_item, NULL, "input", NULL);
        xmlNewProp(user, "type", "text");
        xmlNewProp(user, "id", "user");
        xmlNewProp(user, "name", "user");
         
        xmlNewTextChild(list, NULL, "li", "password");

        list_item = xmlNewChild(list, NULL, "li", NULL);

        xmlNodePtr passwd = xmlNewChild(list_item, NULL, "input", NULL);
        xmlNewProp(passwd, "type", "password");
        xmlNewProp(passwd, "id", "password");
        xmlNewProp(passwd, "name", "password");

        list_item = xmlNewChild(list, NULL, "li", NULL); 
        xmlNodePtr submit = xmlNewChild(list_item, NULL, "input", NULL);
        xmlNewProp(submit, "type", "submit");
        xmlNewProp(submit, "value", "Login");

        content_type(h, "text/html");
        
    }else{
     
        fprintf(h->log, "%f %d %s:%d Element with id qz not found\n", 
            gettime(), h->request_id, __func__, __LINE__);
    }

    return;
}

/*
 *  redirect_to_menu
 *
 *  Creat a page with one menu button to /qz/menu.
 *  Submit it with js.
 */

void redirect_to_menu(struct handler_args* h){

    xmlNodePtr divqz;
    xmlNodePtr form;

    doc_from_file(h, "login.xml");
    if (h->error_exists) return;

    divqz = qzGetElementByID(h, "qz");

    if (divqz !=  NULL){

        char* action;
        asprintf(&action, "/%s/menu", h->uri_parts[QZ_URI_BASE_SEGMENT]);

        form = xmlNewChild(divqz, NULL, "form", NULL);
        register_form(h, form, SUBMIT_ONLY_ONCE, action);
        xmlNewProp(form, "action", action);
        free(action);

        xmlNewProp(form, "method", "post");
        xmlNewProp(form, "id", "menu");
        xmlNewProp(form, "name", "menu");
        xmlNewProp(form, "enctype", "application/x-www-form_urlencoded");
        xmlNewProp(form, "class", "menu main");

        xmlNodePtr submit = xmlNewChild(form, NULL, "input", NULL);
        xmlNewProp(submit, "type", "submit");
        xmlNewProp(submit, "value", "menu");

        content_type(h, "text/html");

        char* js_func =
            "function(){ "
            "console.log('loaded'); "
            "document.getElementById('menu').submit(); "
            "}";

        add_listener(h, NULL, "DOMContentLoaded", js_func);
    }else{
        fprintf(h->log, "%f %d %s:%d Element with id qz not found\n",
            gettime(), h->request_id, __func__, __LINE__);
    }
}
/*
 *  validate_login
 *
 *  Attempt to login with the credentials supplied.
 */
void validate_login( struct handler_args* h  ){

    if (h==NULL) {
        error_page(h, 404,  "handler args is null ");
        return;
    }

    if (h->session==NULL){
        error_page(h, 404,  "session pointer is null ");
        return;
    }

    fprintf(h->log, "%f %d %s:%d start login\n",
            gettime(), h->request_id, __func__, __LINE__);

    const char* kw[] = { "user", "password", NULL };
    char* vals[] = { "", "", NULL };
    char* user;

    vals[0] = user = xmlHashLookup(h->postdata, "user");
    vals[1] = xmlHashLookup(h->postdata, "password"); 

    // If a user name and password are not present then it is 
    // not a real login attempt and validate should not be called.
    if ((vals[0] == NULL) || (vals[1] == NULL)){
        char* login_uri;
        asprintf(&login_uri, "/%s/login", h->uri_parts[0]);
        location(h, login_uri); 
        free(login_uri);
        login_uri = NULL;

        return;
    }
    // If the password is 42 then no login should be attempted.
    // I published some tests with a login of qz and a password 
    // of 42, so hack attempts should be expected.
    if ((vals[1] == NULL) || (strcmp(vals[1], "42") == 0)){
        error_page(h, SC_NOT_ACCEPTABLE, "The answer is not a valid password");
        return;
    }


    if (strlen(user) > MAX_USER_NAME_LENGTH){
        fprintf(h->log, "%f %d %s:%d user name too long %ld\n", 
            gettime(), h->request_id, __func__, __LINE__, strlen(user));

        char* uri_parts[] = {h->uri_parts[0], "login", NULL};
        char* login_target = build_path(uri_parts);
        location(h, login_target);
        free(login_target);
        return;
    }
 
    h->session->conn = PQconnectdbParams(kw, (const char* const*)vals, 0);

    if (PQstatus(h->session->conn) == CONNECTION_OK){    

        strncpy(h->session->user, user, MAX_USER_NAME_LENGTH); 
        h->session->is_logged_in = true;
        h->session->logged_in_time = time(NULL);
        init_open_table(h);

        init_menu(h);

        PGresult* listen_rs;

        listen_rs = PQexec(h->session->conn, "LISTEN pg_db_change");
        if (PQresultStatus(listen_rs) != PGRES_COMMAND_OK){
            // This is a most unexpected, and unfortunate error
            fprintf(h->log, "%f %d %s:%d LISTEN command failed\n", 
                gettime(), h->request_id, __func__, __LINE__);

        }
        PQclear(listen_rs);

        listen_rs = PQexec(h->session->conn, "LISTEN qz_notify");
        if (PQresultStatus(listen_rs) != PGRES_COMMAND_OK){
            // This is a most unexpected, and unfortunate error
            fprintf(h->log, "%f %d %s:%d LISTEN command failed\n", 
                gettime(), h->request_id, __func__, __LINE__);
        }
        PQclear(listen_rs);

        fprintf(h->log, "%f %d %s:%d user %s logged in\n", 
            gettime(), h->request_id, __func__, __LINE__, user);

        h->page_ta = open_table(h, "menu", "view");
        redirect_to_menu(h);
    }else{

        fprintf(h->log, "%f %d %s:%d user %s login failed\n", 
            gettime(), h->request_id, __func__, __LINE__, user);

        char* uri_parts[] = {h->uri_parts[0], "login", NULL};
        char* login_target = build_path(uri_parts);
        location(h, login_target);
        free(login_target);
    }

    return;
}

void logout(struct handler_args* hargs){

    if (hargs==NULL) return;
 
    if (hargs->session != NULL){

        if (*hargs->session->user == '\0'){
            fprintf(hargs->log, 
                "%f %d %s:%d logout user %s\n", 
                gettime(), hargs->request_id, __func__, __LINE__,
                hargs->session->user);
        }else{
            fprintf(hargs->log, 
                "%f %d %s:%d logout user with null name\n", 
                gettime(), hargs->request_id, __func__, __LINE__);
        }

        close_session(hargs, hargs->session);
    }    

    // Set an empty session key to destroy the session.
    char* path_parts[] = {hargs->uri_parts[0], "", NULL};
    char* path = build_path(path_parts);
    make_cookie(hargs, "session_key", "", path, NULL, 0, false, true);
    free(path);

    // Build the logout page with a link to login.
    xmlNodePtr divqz;

    doc_from_file(hargs, "login.xml");
    if (hargs->error_exists) return;

    divqz = qzGetElementByID(hargs, "qz");
    if (divqz != NULL){

        char* login_parts[] = {hargs->uri_parts[0], "login", NULL};
        char* login_path = build_path(login_parts);

        xmlNodePtr h2 = xmlNewChild(divqz, NULL, "h2", NULL);
        
        xmlNodePtr login_link;
        login_link = xmlNewTextChild(h2, NULL, "a", "login");
        xmlNewProp(login_link, "href", login_path);

        free(login_path);
    }    
    
    fprintf(hargs->log, "%f %d %s:%d logout\n", 
        gettime(), hargs->request_id, __func__, __LINE__);

    return;
}
