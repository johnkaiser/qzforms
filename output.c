
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
 *  otherwise look for a strbuf list.
 */

void serve_output( struct handler_args* hargs ){ 

    if (hargs==NULL) return;
    if (hargs->error_exists) return;

    fprintf(hargs->log, "%f %d %s:%d %s\n", 
        gettime(), hargs->request_id, __func__, __LINE__,
        "function serve_output");

    struct strbuf* a_header = hargs->headers;

    while(a_header != NULL){
        FCGX_FPrintF(hargs->out, "%s\r\n", a_header->str); 
        a_header = a_header->next;
    }
    if (hargs->headers != NULL) FCGX_FPrintF(hargs->out, "\r\n");

    if (hargs->doc != NULL){
       
        xmlBufferPtr xbuf = xmlBufferCreate();
        if (xbuf == NULL){ 
            fprintf(hargs->log, "%f %d %s:%d %s\n", 
                gettime(), hargs->request_id, __func__, __LINE__,
                "xbuf from xmlBufferCreate is null\n");

            error_page(hargs, SC_INTERNAL_SERVER_ERROR, "xmlBufferCreate failed");
            return;
        }
    
        xmlSaveCtxtPtr ctxt = xmlSaveToBuffer(xbuf, "UTF-8", 
             XML_SAVE_FORMAT|XML_SAVE_NO_DECL|XML_SAVE_AS_HTML);

        if ((ctxt == NULL) && (hargs->log != NULL)){ 
            fprintf(hargs->log, "%f %d %s:%d %s\n", 
                gettime(), hargs->request_id, __func__, __LINE__,
                "ctxt is null");
        }

        xmlSaveDoc(ctxt, hargs->doc);
        xmlSaveClose(ctxt);

        FCGX_FPrintF(hargs->out, "%s", xbuf->content);
        
        xmlBufferFree(xbuf);

    } else if (hargs->data != NULL){

        struct strbuf* a_datum;
        for(a_datum=hargs->data; a_datum!=NULL; a_datum=a_datum->next){
            FCGX_FPrintF(hargs->out, "%s", a_datum->str);

        }
    } else {
        error_page(hargs, SC_EXPECTATION_FAILED, "No content generated" );
    }

    return;
}

/*
 *  content_type
 *
 *  Add a content type header with the given mime type.
 */

void content_type(struct handler_args* h, char* mime_type){

    char *content_type = "Content-type: ";
    struct strbuf * ct = new_strbuf(NULL, QZ_MAX_CONTENT_TYPE_LENGTH);

    strncpy(ct->str, content_type, QZ_MAX_CONTENT_TYPE_LENGTH);
    strncat(ct->str, mime_type, 
        QZ_MAX_CONTENT_TYPE_LENGTH - strlen(content_type) );
     
    ct->next = h->headers;
    h->headers = ct;
}

/*
 *  expires
 *
 *  Add an expires header to expire at the time given.
 */

void expires(struct handler_args* h, time_t expires_t){
    // I need about 38 chars, 128 is gratuitous
    struct strbuf* expires_buf = new_strbuf(NULL, 128);
 
    struct tm* expires_tm = malloc(sizeof(struct tm));

    gmtime_r(&expires_t, expires_tm);
    strftime(expires_buf->str, 127, "Expires: %a %b %d %T %Z %Y", expires_tm);

    free(expires_tm);

    expires_buf->next = h->headers;
    h->headers = expires_buf;
}


/*
 *  etag_header
 *
 *  Add an etag header with the given value of payload
 *  encoded in the etag string.
 */

void etag_header(struct handler_args* h, uint64_t payload){
    
    if (payload == 0) return;

    // the line will be like:
    // etag: "626d480193b5c9d6.8b4aeb4b01d2df4b663167b3a184db84"
    // 4 + 1 + 1 + 1 + 49 + 1 + null = 58 chars, 64 is gratuitous

    struct strbuf* etag_header = new_strbuf(NULL, 64);

    char etag_buf[64];
    make_etag(etag_buf, h->session->tagger_socket_path, payload);
    if (strlen(etag_buf)==49){
       snprintf(etag_header->str, 64, "etag: \"%s\"", etag_buf);
       etag_header->next = h->headers;
       h->headers = etag_header;
    }
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
    
    FCGX_SetExitStatus(status_code, h->out);
    FCGX_FPrintF(h->out, "Status: %d\r\n", status_code);

    if (msg != NULL){
        FCGX_FPrintF(h->out, "Content-type: text/plain\r\n");
        FCGX_FPrintF(h->out, "\r\n");
        FCGX_FPrintF(h->out, "ERROR\r\n");
        FCGX_FPrintF(h->out, "%s\r\n", msg);
        FCGX_FPrintF(h->out, "code=%d\r\n", status_code);
        FCGX_FPrintF(h->out, "request_id=%d\r\n", h->request_id);
        h->error_exists = true;
    }else{
        FCGX_FPrintF(h->out, "\r\n");
    }    
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

        fprintf(h->log,"%f %d %s:%d notify relname=%s be_pid=%d extra=%s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            notify->relname, notify->be_pid, notify->extra);

        if (strcmp("pg_db_change",  notify->relname) == 0){ 
                // Just notified the database definition has been updated.
                // Drop the pgtype_datum's and reload them on each new 
                // request.
                close_all_pgtype_datums(h);

                fprintf(h->log,"%f %d %s:%d datum hash emptied\n", 
                    gettime(), h->request_id, __func__, __LINE__);

        }    
    }
}


/*
 *  do_page
 *
 *  
 */
void do_page( struct handler_args* hargs ){
    
    // This can happen if non-utf8 data is posted,
    // or if content length is not bytes read.
    if (hargs->error_exists){
        return;
    }    

    // Check the post data for being legal
    // It must contain a name 'form_tag' with value
    // that passes the test for being a valid etag.
    // Requests without postdata such as qzfs requests or getall requests
    // do not post anything, so skip this.

    if (hargs->postdata != NULL){
        if (!post_contains_valid_form_tag(hargs)){

            fprintf(hargs->log, "%f %d %s:%d fail post contains invalid form_tag\n", 
                gettime(), hargs->request_id, __func__, __LINE__);

            // This was a hard error, but it happens too frequently for that. 
            logout(hargs);
            return;
        }
    }

    // The handler name will come from qz.form unless it is a builtin.
    static char* builtins[] = {"login","logout","refresh","status",
        "timestamp", NULL};
    static char menu_txt[] = "menupage";
    
    char* form_name = get_uri_part(hargs, QZ_URI_FORM_NAME);

    if (form_name == NULL){
            fprintf(hargs->log, "%f %d %s:%d fail form_name not present\n", 
                gettime(), hargs->request_id, __func__, __LINE__);
        
            error_page(hargs, SC_NOT_FOUND, "not found");
            return;
    }
    
    if (strncmp("refresh", form_name, MAX_SEGMENT_LENGTH) == 0){

        // A refresh request to extend the expire time on a form
        // does not need to talk to postgres, so does not need the
        // mutex.

        refresh_form_tag(hargs);

    }else{

        // Get that mutex

        fprintf(hargs->log, "%f %d %s:%d request session mutex\n", 
            gettime(), hargs->request_id, __func__, __LINE__);
        
        // mutex lock and wait for session 
        pthread_mutex_lock(&(hargs->session->session_lock));
        /*************** Inside Mutex ******************************/
    
        fprintf(hargs->log, "%f %d %s:%d session mutex acquired\n", 
            gettime(), hargs->request_id, __func__, __LINE__);
        fflush(hargs->log);    

        // It is possible that a logout happened while this
        // was waiting, check again.
        if (hargs->session->is_logged_in){

            hargs->session->last_activity_time = time(NULL);

            // Identify the handler

            int n;
            char* handler_name = NULL;
            struct table_action* page_ta = NULL;

            for(n=0; builtins[n] != NULL; n++){
                if (strncmp(form_name, builtins[n], MAX_SEGMENT_LENGTH) == 0){
                    handler_name = builtins[n];         
                    break;
                }    
            }

            char* action = get_uri_part(hargs, QZ_URI_ACTION);

            // Note that builtins do not need an action but anything
            // from a table_action must have an action or it won't
            // be called.

            if ((handler_name == NULL) && (action != NULL)){ 
                // not in builtins from above

                page_ta = open_table(hargs, form_name, action);
                if (page_ta != NULL){
                    handler_name = page_ta->handler_name;
                    // page_ta can be used without checking for null
                    // because it is now known to exist unless it's a menu.
                    hargs->page_ta = page_ta;
                }    
            }
            if ((handler_name == NULL) && (action == NULL)){
               // like a menu page
                fprintf(hargs->log, "%f %d %s:%d form_name_is_menu=%s\n", 
                    gettime(), hargs->request_id, __func__, __LINE__,
                    form_name_is_menu(hargs) ? "t":"f");

               if (form_name_is_menu(hargs)){
                   handler_name = menu_txt;
                   hargs->page_ta = open_table(hargs, form_name, "view");
               }    
            }

            if (handler_name == NULL){
                fprintf(hargs->log, "%f %d %s:%d fail handler not found\n", 
                    gettime(), hargs->request_id, __func__, __LINE__);
        
                error_page(hargs, SC_NOT_FOUND, "not found");

            }else{ // have a handler name

                fprintf(hargs->log, "%f %d %s:%d do_page handler %s, %s\n", 
                    gettime(), hargs->request_id, __func__, __LINE__,
                    handler_name, action);
    
                struct handler* handler;
                handler = xmlHashLookup(handler_hash, handler_name);


                if (handler != NULL){
                    catch_notifies(hargs);
                    handler->count++;

                    if ( check_postdata(hargs) ){
                       // Execute the selected handler
                        handler->handler( hargs );
                    }    
                } 
            }    
        }else{
             error_page(hargs, SC_UNAUTHORIZED, "session logged out");
        }

        // Release session mutex 
        pthread_mutex_unlock(&(hargs->session->session_lock));
        /*************** Outside Mutex *****************************/
        fprintf(hargs->log, "%f %d %s:%d session mutex released\n", 
            gettime(), hargs->request_id, __func__, __LINE__);
        fflush(hargs->log);    
    }
    return;
}

void rs_to_table(xmlNodePtr add_here, PGresult* rs, char* id){

    xmlNodePtr table = xmlNewChild(add_here, NULL, "table", NULL);
    xmlNodePtr thead;
    xmlNodePtr tbody;
    xmlNodePtr tr;
    xmlNodePtr td;
    xmlNodePtr th;
    int t, f;

    xmlNewProp(table, "class", "qztable tablesorter");
    if (id!=NULL) xmlNewProp(table, "id", id);

    thead = xmlNewChild(table,NULL,"thead",NULL);
    tr = xmlNewChild(thead, NULL, "tr", NULL);
    for(t=0; t<PQnfields(rs); t++){
        th = xmlNewTextChild(tr, NULL, "th",
            PQfname(rs,t) );
    }
    tbody = xmlNewChild(table,NULL,"tbody",NULL);
    for(t=0; t<PQntuples(rs); t++ ){
        tr = xmlNewChild(tbody, NULL, "tr", NULL);

        for(f=0; f<PQnfields(rs); f++ ){
            td = xmlNewTextChild(tr,NULL, "td", 
                PQgetvalue(rs, t, f) );
        }
    }
}

