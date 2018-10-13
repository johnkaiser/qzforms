
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

pthread_mutex_t login_tracker_mutex;
xmlHashTablePtr failed_login_tracker;
/*
 *  init_login_tracker
 *
 *  setup mutex for tracking failed logins
 */
void init_login_tracker(void){
    pthread_mutex_init(&login_tracker_mutex,NULL);
    failed_login_tracker = xmlHashCreate(101);
}

/*
 *  record_login_attempt
 *
 *  Record the count of failed and successfull logins for a
 *  period of time. Used as a basis for blocking dictionary
 *  attacks.
 */
void record_login_attempt(struct handler_args* hargs, char* remote,
    bool success_or_fail){

    if ((remote == NULL) || (remote[0] == '\0')) return;
    if (hargs->conf->max_failed_logins == 0) return;

    // lock access  here
    pthread_mutex_lock(&login_tracker_mutex);
    double previous_attempt = 0;
    struct login_tracker* lt = xmlHashLookup(failed_login_tracker, remote);

    if (lt == NULL){
        lt = calloc(1, sizeof(struct login_tracker));
        snprintf(lt->remote_host, (INET6_ADDRSTRLEN+1), "%s", remote);
        xmlHashAddEntry(failed_login_tracker, lt->remote_host, lt);
    }else{
        previous_attempt = lt->last_attempt;
        if ((gettime() - lt->last_attempt) >
            hargs->conf->failed_login_block_timeout){

            lt->success = 0;
            lt->failed = 0;
        }
    }

    lt->last_attempt = gettime();

    if (success_or_fail == true){
        lt->success++;
    }else{
        lt->failed++;
    }

    if (hargs->conf->log_login_tracker_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d host %s %s last attempt %f "
            "failed count %d success count %d\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            remote, (success_or_fail) ? "success":"fail",
            previous_attempt, lt->failed, lt->success);
        pthread_mutex_unlock(&log_mutex);
    }
    pthread_mutex_unlock(&login_tracker_mutex);
}


/*
 *  check_login_tracker
 *
 *  Check the failed login record and indicate if the request
 *  should be honored.
 */
bool check_login_tracker(struct handler_args* h){

    if (h->conf->max_failed_logins == 0) return true;

    char* remote = FCGX_GetParam("REMOTE_ADDR", h->envpfcgi);

    pthread_mutex_lock(&login_tracker_mutex);

    struct login_tracker* lt = xmlHashLookup(failed_login_tracker, remote);

    if (lt == NULL){
        // This means there have been no failed logins

        if (h->conf->log_login_tracker_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d host %s no tracker record\n",
            gettime(), h->request_id, __func__, __LINE__, remote);
            pthread_mutex_unlock(&log_mutex);
        }
        pthread_mutex_unlock(&login_tracker_mutex);
        return true;
    }

    if (lt->success > 0){
        // Someone has logged in from this address, so OK

        if (h->conf->log_login_tracker_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d host %s previous login OK\n",
                gettime(), h->request_id, __func__, __LINE__, remote);
            pthread_mutex_unlock(&log_mutex);
        }
        pthread_mutex_unlock(&login_tracker_mutex);
        return true;
    }

    if ((gettime() - lt->last_attempt) >
            h->conf->failed_login_block_timeout){
       // The record is old, ignore it.

        if (h->conf->log_login_tracker_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d host %s ignoring old record\n",
                gettime(), h->request_id, __func__, __LINE__, remote);
            pthread_mutex_unlock(&log_mutex);
        }
        pthread_mutex_unlock(&login_tracker_mutex);
        return true;
   }

   if (lt->failed > h->conf->max_failed_logins){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d denying login attempt from host %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            remote);
        pthread_mutex_unlock(&log_mutex);

        error_page(h, SC_TOO_MANY_REQUESTS,
            "Too many failed login attempts.\nTry again later.");

        pthread_mutex_unlock(&login_tracker_mutex);
        return false;
    }else{
        pthread_mutex_unlock(&login_tracker_mutex);
        return true;
    }

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d fail unexpected result\n",
        gettime(), h->request_id, __func__, __LINE__);
    pthread_mutex_unlock(&log_mutex);

    pthread_mutex_unlock(&login_tracker_mutex);
    return false; // should never happen.
}

/*
 *  login_tracking_housekeeping
 *  login_tracking_scanner
 *
 *  Remove expired login tracking records.
 */
void login_tracking_scanner(void* payload, void* data, const xmlChar* name){
    struct login_tracker* lt = payload;
    struct handler_args* hargs = data;

    if (hargs->conf->log_login_tracker_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d checking host %s "
            "last_attempt %f failed %d success %d\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            lt->remote_host, lt->last_attempt, lt->failed, lt->success);
        pthread_mutex_unlock(&log_mutex);
    }

    if ((gettime() - lt->last_attempt) >
            hargs->conf->failed_login_block_timeout){

        if (hargs->conf->log_login_tracker_details){
            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, "%f %d %s:%d removing host %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                lt->remote_host);
            pthread_mutex_unlock(&log_mutex);
        }
       // The record is old, clear it out.
       xmlHashRemoveEntry(failed_login_tracker, name,
           (xmlHashDeallocator)xmlFree);
    }
}
void login_tracking_housekeeping(struct handler_args* hargs){

    double start_time = gettime();
    if (hargs->conf->log_login_tracker_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d beginning housekeeping\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);
    }

    pthread_mutex_lock(&login_tracker_mutex);

    xmlHashScan(failed_login_tracker, (void*) login_tracking_scanner, hargs);

    pthread_mutex_unlock(&login_tracker_mutex);

    pthread_mutex_lock(&log_mutex);
    fprintf(hargs->log, "%f %d %s:%d completed in %f\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        gettime() - start_time);
    pthread_mutex_unlock(&log_mutex);

}


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

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d\n", 
        gettime(), h->request_id, __func__, __LINE__);
    pthread_mutex_unlock(&log_mutex);

    // logged by function
    if (check_login_tracker(h) == false) return;

    doc_from_file(h, "login.xml");
    if (h->error_exists) return;

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d login request from host %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        FCGX_GetParam("REMOTE_ADDR", h->envpfcgi));
    pthread_mutex_unlock(&log_mutex);

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
     
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d Element with id qz not found\n", 
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);
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
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d Element with id qz not found\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);
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

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d start login\n",
            gettime(), h->request_id, __func__, __LINE__);
    pthread_mutex_unlock(&log_mutex);

    const char* kw[] = { "user", "password", NULL };
    char* vals[] = { "", "", NULL };
    char* user;

    vals[0] = user = xmlHashLookup(h->postdata, "user");
    vals[1] = xmlHashLookup(h->postdata, "password"); 

    // If a user name and password are not present then it is 
    // not a real login attempt and validate should not be called.
    if ((vals[0] == NULL) || (vals[0][0] == '\0') || 
        (vals[1] == NULL) || (vals[1][0] == '\0')  ){
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
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d user name too long %zu\n",
            gettime(), h->request_id, __func__, __LINE__, strlen(user));
        pthread_mutex_unlock(&log_mutex);

        char* uri_parts[] = {h->uri_parts[0], "login", NULL};
        char* login_target = build_path(uri_parts);
        location(h, login_target);
        free(login_target);
        return;
    }
 
    h->session->conn = PQconnectdbParams(kw, (const char* const*)vals, 0);

    char* remote_address =  FCGX_GetParam("REMOTE_ADDR", h->envpfcgi);

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
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d LISTEN command failed\n", 
                gettime(), h->request_id, __func__, __LINE__);
            pthread_mutex_unlock(&log_mutex);

        }
        PQclear(listen_rs);

        listen_rs = PQexec(h->session->conn, "LISTEN qz_notify");
        if (PQresultStatus(listen_rs) != PGRES_COMMAND_OK){
            // This is a most unexpected, and unfortunate error
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d LISTEN command failed\n", 
                gettime(), h->request_id, __func__, __LINE__);
            pthread_mutex_unlock(&log_mutex);
        }
        PQclear(listen_rs);

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d user %s login success from host %s\n",
            gettime(), h->request_id, __func__, __LINE__, user, remote_address);
        pthread_mutex_unlock(&log_mutex);

        record_login_attempt(h, remote_address, true);

        h->page_ta = open_table(h, "menu", "view");
        redirect_to_menu(h);
    }else{

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d user %s login failed from host %s\n",
            gettime(), h->request_id, __func__, __LINE__, user, remote_address);
        pthread_mutex_unlock(&log_mutex);

        record_login_attempt(h, remote_address, false);

        char* uri_parts[] = {h->uri_parts[0], "login", NULL};
        char* login_target = build_path(uri_parts);
        location(h, login_target);
        free(login_target);
        PQfinish(h->session->conn);
        h->session->conn = NULL;
    }

    return;
}

void logout(struct handler_args* hargs){

    if (hargs==NULL) return;
 
    if (hargs->session != NULL){

        if (*hargs->session->user != '\0'){
            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, 
                "%f %d %s:%d logout user %s from host %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                hargs->session->user,
                FCGX_GetParam("REMOTE_ADDR", hargs->envpfcgi));
            pthread_mutex_unlock(&log_mutex);
        }else{
            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, 
                "%f %d %s:%d logout user with null name from host %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                FCGX_GetParam("REMOTE_ADDR", hargs->envpfcgi));
            pthread_mutex_unlock(&log_mutex);
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
    
    pthread_mutex_lock(&log_mutex);
    fprintf(hargs->log, "%f %d %s:%d logout\n", 
        gettime(), hargs->request_id, __func__, __LINE__);
    pthread_mutex_unlock(&log_mutex);

    return;
}
