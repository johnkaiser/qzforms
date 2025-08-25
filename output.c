
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
 *  serve_output
 *
 *  If the error flag is flipped then do nothing.
 *  If an xml document exists then serve that,
 *  otherwise look for an xmlBuffer in handler_args data.
 */

void serve_output( struct handler_args* hargs ){

    if (hargs==NULL) return;
    if (hargs->error_exists) return;

    pthread_mutex_lock(&log_mutex);
    fprintf(hargs->log, "%f %d %s:%d %s\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        "function serve_output");
    pthread_mutex_unlock(&log_mutex);

    if (hargs->headers != NULL){
        FCGX_FPrintF(hargs->out, "%s\r\n", xmlBufferContent(hargs->headers));
    }

    if (hargs->doc != NULL){

        xmlBufferPtr xbuf = xmlBufferCreate();
        if (xbuf == NULL){
            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, "%f %d %s:%d %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                "xbuf from xmlBufferCreate is null\n");
            pthread_mutex_unlock(&log_mutex);

            error_page(hargs, SC_INTERNAL_SERVER_ERROR, "xmlBufferCreate failed");
            return;
        }

        xmlSaveCtxtPtr ctxt = xmlSaveToBuffer(xbuf, "UTF-8",
             XML_SAVE_FORMAT|XML_SAVE_NO_DECL|XML_SAVE_AS_HTML);

        if ((ctxt == NULL) && (hargs->log != NULL)){
            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, "%f %d %s:%d %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                "ctxt is null");
            pthread_mutex_unlock(&log_mutex);
        }

        xmlSaveDoc(ctxt, hargs->doc);
        xmlSaveClose(ctxt);

        FCGX_FPrintF(hargs->out, "%s", xbuf->content);

        xmlBufferFree(xbuf);

    } else if (hargs->data != NULL){

        FCGX_FPrintF(hargs->out, "%s", xmlBufferContent(hargs->data));

    } else {
        error_page(hargs, SC_BAD_REQUEST, "No content generated" );
    }

    return;
}

/*
 *  content_type
 *
 *  Add a content type header with the given mime type.
 */

void content_type(struct handler_args* h, char* mime_type){

    add_header(h, "Content-type", mime_type);
}

/*
 *  expires
 *
 *  Add an expires header to expire at the time given.
 */

void expires(struct handler_args* h, time_t expires_t){
    // I need about 38 chars, 128 is gratuitous
    char expbuf[128];
    struct tm* expires_tm = malloc(sizeof(struct tm));

    gmtime_r(&expires_t, expires_tm);
    strftime(expbuf, 127, "%a %b %d %T %Z %Y", expires_tm);
    add_header(h, "Expires", expbuf);
    free(expires_tm);
}

/*
 *  error_page
 *
 *  Declare an error condition by setting error_exists, and
 *  immediately sending out an error page.
 *  Setting error_exists will stop the normal output of docs
 *  and buffers, but they will be cleaned up by free_handler.
 */

void error_page( struct handler_args* h, int status_code, const char* msg ){

    h->error_exists = true;

#ifdef IS_TEST
    fprintf(stderr, "XXXXXXXXXXXX ERROR PAGE XXXXXXXXXXXXXX\n");
    fprintf(stderr, "Status: %d\n", status_code);

    if (msg != NULL){
        fprintf(stderr, "Content-type: text/plain\r\n");
        fprintf(stderr, "\r\n");
        fprintf(stderr, "ERROR MSG:\r\n");
        fprintf(stderr, "%s\r\n", msg);
        fprintf(stderr, "code=%d\r\n", status_code);
        fprintf(stderr, "request_id=%d\r\n", h->request_id);
        h->error_exists = true;
    }

#else

    FCGX_SetExitStatus(status_code, h->out);
    FCGX_FPrintF(h->out, "Status: %d\r\n", status_code);

    if (msg != NULL){
        FCGX_FPrintF(h->out, "Content-type: text/html\r\n");
        FCGX_FPrintF(h->out, "\r\n");
        FCGX_FPrintF(h->out, "<!DOCTYPE html>\r\n");
        FCGX_FPrintF(h->out, "<html><body>\r\n");
        FCGX_FPrintF(h->out, "<p><b>ERROR</b></p>\r\n");
        FCGX_FPrintF(h->out, "<p>%s</p>\r\n", msg);
        FCGX_FPrintF(h->out, "<p>code=%d</p>\r\n", status_code);
        FCGX_FPrintF(h->out, "<p>request_id=%d</p>\r\n", h->request_id);
        FCGX_FPrintF(h->out, "<p><a href='/%s/login'>Login</a></p>\r\n",
            get_uri_part(h,QZ_URI_BASE_SEGMENT));
        FCGX_FPrintF(h->out, "</body></html>\r\n");
        h->error_exists = true;
    }else{
        FCGX_FPrintF(h->out, "\r\n");
    }
#endif

    return;
}

/*
 *  catch_notifies
 *
 *  Polls pg for notifications.
 */

void catch_notifies(struct handler_args* h){

    PGnotify   *notify;

    while ((notify = PQnotifies(h->session->conn)) != NULL){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log,"%f %d %s:%d notify relname=%s be_pid=%d extra=%s\n",
            gettime(), h->request_id, __func__, __LINE__,
            notify->relname, notify->be_pid, notify->extra);
        pthread_mutex_unlock(&log_mutex);

        if (strcmp("pg_db_change",  notify->relname) == 0){
                // Just notified the database definition has been updated.
                // Drop the pgtype_datum's and reload them on each new
                // request.
                close_all_pgtype_datums(h->session);

                pthread_mutex_lock(&log_mutex);
                fprintf(h->log,"%f %d %s:%d datum hash emptied\n",
                    gettime(), h->request_id, __func__, __LINE__);
                pthread_mutex_unlock(&log_mutex);

        }
    }
}


/*
 *  form_tag_required
 */

bool form_tag_required(char* handler_name){

    static char* form_tag_not_required[] = {"login", "logout", "refresh",
        "fs", NULL};

    int n;

    for(n=0; form_tag_not_required[n] != NULL; n++){
        if (strcmp(handler_name, form_tag_not_required[n]) == 0){
            return false;
        }
    }
    return true;
}
/*
 *  do_page
 *
 */
void do_page( struct handler_args* hargs ){

    static char view_txt[] = "view";
    static char builtin_txt[] = "builtin";
    static char* builtins[] = {"login","logout","refresh","status", NULL};

    // This can happen if non-utf8 data is posted,
    // or if content length is not bytes read.
    if (hargs->error_exists){
        return;
    }

    bool has_valid_form_tag = false;
    bool is_builtin = false;
    char* form_name = NULL;
    char* action = NULL;

    form_name = get_uri_part(hargs, QZ_URI_FORM_NAME);
    action = get_uri_part(hargs, QZ_URI_ACTION);

    if (form_name == NULL){
            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, "%f %d %s:%d fail form_name not present\n",
                gettime(), hargs->request_id, __func__, __LINE__);
            pthread_mutex_unlock(&log_mutex);

            error_page(hargs, SC_NOT_FOUND, "not found");
            return;
    }

    // The handler name will come from qz.form unless it is a builtin.
    is_builtin = item_in_list(form_name, builtins);

    // Check the post data for being legal
    // It must contain a name 'form_tag' with value
    // that passes the test for being a valid etag.
    // Requests without postdata such as qzfs requests
    // do not post anything, so skip this.

    if (hargs->postdata != NULL){
        if ( ! post_contains_valid_form_tag(hargs)){

            pthread_mutex_lock(&log_mutex);
            fprintf(hargs->log, "%f %d %s:%d fail post contains invalid form_tag\n",
                gettime(), hargs->request_id, __func__, __LINE__);
            pthread_mutex_unlock(&log_mutex);

            // This was a hard error, but it happens too frequently for that.
            error_page(hargs, 404, "invalid request");
            return;
        }else{
            has_valid_form_tag = true;
        }
    }

    if ((has_valid_form_tag) &&
        (strncmp("refresh", form_name, MAX_SEGMENT_LENGTH) == 0)){

        // A refresh request to extend the expire time on a form
        // does not need to talk to postgres, so does not need the
        // mutex.

        refresh_form_tag(hargs);

    }else{

        // Get that session mutex

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d request session mutex\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        // mutex lock and wait for session
        pthread_mutex_lock(&(hargs->session->session_lock));
        /*************** Inside Mutex ******************************/

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d session mutex acquired\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        fflush(hargs->log);
        pthread_mutex_unlock(&log_mutex);

        // It is possible that a logout happened while this
        // was waiting, check again.
        if (hargs->session->is_logged_in){

            hargs->session->last_activity_time = time(NULL);

            if ((action == NULL) && (form_name_is_menu(hargs))){
                action = view_txt;
            }
            // Identify the handler

            struct table_action* page_ta = NULL;

            if (is_builtin){
                hargs->handler_name = get_uri_part(hargs, QZ_URI_FORM_NAME);
                action = builtin_txt;
            }else{

                page_ta = open_table(hargs, form_name, action);
                if (page_ta != NULL){
                    hargs->handler_name = page_ta->handler_name;
                    hargs->page_ta = page_ta;
                }else{

                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d open table fail "
                        "on [%s] [%s] \n",
                        gettime(), hargs->request_id, __func__, __LINE__,
                        form_name, action);
                    pthread_mutex_unlock(&log_mutex);

                    error_page(hargs, SC_NOT_FOUND, "not found");
                }
            }
            if ( ! hargs->error_exists){
                if (hargs->handler_name == NULL){
                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d fail handler not found\n",
                        gettime(), hargs->request_id, __func__, __LINE__);
                    pthread_mutex_unlock(&log_mutex);

                    error_page(hargs, SC_NOT_FOUND, "not found");

                }else{ // have a handler name

                    pthread_mutex_lock(&log_mutex);
                    fprintf(hargs->log, "%f %d %s:%d do_page handler %s, %s\n",
                        gettime(), hargs->request_id, __func__, __LINE__,
                        hargs->handler_name, action);
                    pthread_mutex_unlock(&log_mutex);

                    struct handler* handler;
                    handler = xmlHashLookup(handler_hash, hargs->handler_name);

                    if (form_tag_required(hargs->handler_name) &&
                        ! has_valid_form_tag ){

                        pthread_mutex_lock(&log_mutex);
                        fprintf(hargs->log, "%f %d %s:%d fail form tag not found\n",
                            gettime(), hargs->request_id, __func__, __LINE__);
                        pthread_mutex_unlock(&log_mutex);

                        error_page(hargs, SC_BAD_REQUEST, "validation failed");
                    }

                    if ( ! hargs->error_exists && (handler != NULL) ){
                        catch_notifies(hargs);
                        handler->count++;

                        hargs->handler_name = handler->name;

                        if ( check_postdata(hargs) ){
                           // Execute the selected handler
                            handler->handler( hargs );
                        }
                    }
                }
            } // not error_exists
        }else{
             error_page(hargs, SC_UNAUTHORIZED, "session logged out");
        }

        // Release session mutex
        pthread_mutex_unlock(&(hargs->session->session_lock));
        /*************** Outside Mutex *****************************/
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d session mutex released\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        fflush(hargs->log);
        pthread_mutex_unlock(&log_mutex);
    }
    return;
}

void rs_to_table(xmlNodePtr add_here, PGresult* rs, char* id){

    xmlNodePtr table = xmlNewChild(add_here, NULL, "table", NULL);
    xmlNodePtr thead;
    xmlNodePtr tbody;
    xmlNodePtr tr;
    int t, f;

    xmlNewProp(table, "class", "qztable tablesorter");
    if (id!=NULL) xmlNewProp(table, "id", id);

    thead = xmlNewChild(table,NULL,"thead",NULL);
    tr = xmlNewChild(thead, NULL, "tr", NULL);
    for(t=0; t<PQnfields(rs); t++){
        xmlNewTextChild(tr, NULL, "th",
            PQfname(rs,t) );
    }
    tbody = xmlNewChild(table,NULL,"tbody",NULL);
    for(t=0; t<PQntuples(rs); t++ ){
        tr = xmlNewChild(tbody, NULL, "tr", NULL);

        for(f=0; f<PQnfields(rs); f++ ){
            xmlNewTextChild(tr,NULL, "td",
                PQgetvalue(rs, t, f) );
        }
    }
}

void rs_to_sideways_table(xmlNodePtr add_here, PGresult* rs, char* id){

    xmlNodePtr table = xmlNewChild(add_here, NULL, "table", NULL);
    xmlNodePtr tbody;
    xmlNodePtr tr;
    xmlNodePtr td;
    int t, f;

    xmlNewProp(table, "class", "qztable tablesorter");
    if (id!=NULL) xmlNewProp(table, "id", id);

    tbody = xmlNewChild(table,NULL,"tbody",NULL);

    for (f=0; f<PQnfields(rs); f++ ){

        tr = xmlNewChild(tbody, NULL, "tr", NULL);
        td = xmlNewTextChild(tr, NULL, "td", PQfname(rs,f) );
        append_class(td, "viewtitle");

        for (t=0; t<PQntuples(rs); t++){
            xmlNewTextChild(tr,NULL, "td", PQgetvalue(rs, t, f) );
        }
    }
}

