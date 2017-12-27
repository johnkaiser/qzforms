
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

pid_t tagger_pid = 0;

struct thread_launch_data {

    struct qz_config* conf;
    int thread_id;
    pthread_mutex_t accept_mutex;
    char** envpmain;
    xmlHashTablePtr sessions;
    int* next_id;

};

int next_id;

/*
 *  cleanup
 *
 *  Catch a sigterm signal and shut down the tagger process.
 */
void cleanup(int sig){

    // kill tagger
    kill(tagger_pid, SIGTERM);

    int status;
    waitpid(tagger_pid, &status, 0);
    exit(0);
}

/*
 *  init_handler
 *
 *  Initialize a request by collecting what is known about the
 *  request into a single structure to pass to the designated handler.
 */
struct handler_args* init_handler(FCGX_Request *request, char *envpmain[],
    int request_id, struct qz_config* conf){

    char* ck;
    struct handler_args* hargs;
    uint64_t session_payload = 0;

    if ( (hargs = calloc(1, sizeof(struct handler_args))) == NULL ){
       FCGX_FPrintF(request->err, "calloc failed in init_handler");
       // Just up and die then.
       exit(11);
    }
    hargs->starttime = gettime();

    hargs->log = fopen(conf->logfile_name, "a");

    if (hargs->log == NULL){
       free(hargs);
       FCGX_FPrintF(request->err, "open failed on log file %s\n",
           conf->logfile_name);

       return NULL;
    };


    if (request != NULL){
        fprintf(hargs->log, "%f %d %s:%d begin init_handler %s\n",
            hargs->starttime, request_id, __func__, __LINE__,
            FCGX_GetParam("REQUEST_URI",request->envp));

        // not the housekeeper
        hargs->request = request;
        hargs->in = request->in;
        hargs->out = request->out;
        hargs->err = request->err;
        hargs->envpfcgi = request->envp;
        hargs->uri_parts = str_to_array(
            FCGX_GetParam("REQUEST_URI",request->envp), '/');
    }else{
        fprintf(hargs->log, "%f %d %s:%d init_handler %s\n",
            hargs->starttime, request_id, __func__, __LINE__,
            "housekeeper");

        // is the housekeeper
        hargs->request = NULL;
        hargs->envpfcgi = NULL;
        hargs->uri_parts = NULL;
    }
    hargs->cookie_buf = NULL;
    hargs->envpmain = envpmain;
    hargs->request_id = request_id;
    hargs->conf = conf;
    hargs->page_ta = NULL;
    hargs->headers = NULL;
    hargs->posted_form = NULL;
    hargs->current_form_set = NULL;
    hargs->error_exists = false;
    hargs->regex_check = notchecked;
    hargs->pkey_check = notchecked;

    if (hargs->uri_parts != NULL){  // null for housekeeper
        for( hargs->nbr_uri_parts = 0;
            hargs->uri_parts[hargs->nbr_uri_parts] != NULL;
            hargs->nbr_uri_parts++)
            ;
    }

    if ( (ck = FCGX_GetParam("HTTP_COOKIE", hargs->envpfcgi)) != NULL ){
        parse_cookie(hargs, ck);

        char* session_key = xmlHashLookup(hargs->cookiesin, "session_key");
        if (session_key != NULL){
            // Just check that it could be valid.
            // Don't actually check that it is in sessions and is valid.
            // Content parsing is skipped if this is zero.
            session_payload = validate_etag(conf->tagger_socket_path,
                session_key);

            snprintf(hargs->session_key, SESSION_KEY_LENGTH, "%s", session_key);
        }

        fprintf(hargs->log, "%f %d %s:%d cookiesin OK\n",
            gettime(), request_id, __func__, __LINE__);
    }else{
        hargs->cookiesin = NULL;
        fprintf(hargs->log, "%f %d %s:%d cookiesin is null\n",
            gettime(), request_id, __func__, __LINE__);
    }

    char* content_length_str = FCGX_GetParam("CONTENT_LENGTH", hargs->envpfcgi);
    int content_length = 0;
    if (content_length_str != NULL){
        content_length = atoi( content_length_str );
    }

    if ((content_length > 0) && (session_payload > 0)) {
        char* buf;
        int bytes_read;
        buf = calloc(1, content_length+2);
        bytes_read = FCGX_GetStr(buf, content_length, hargs->in);
        hargs->postbuf = buf;

        if (bytes_read != content_length){
            fprintf(hargs->log, "%f %d %s:%d fail "
                "content_length=%d bytesread=%d\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                content_length, bytes_read);
            // According to the fcgi spec, this
            // must be an abort on this condition.
            error_page(hargs, SC_BAD_REQUEST, "content length read error");
        }

        // parse_post
        if ( ! hargs->error_exists ){ // possibly set by error_page() above
            fprintf(hargs->log, "%f %d %s:%d parse_post called with %zu %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            strlen(hargs->postbuf), "bytes");

            hargs->postdata = parse_key_eq_val(hargs, hargs->postbuf, '&',true);

            fprintf(hargs->log, "%f %d %s:%d parse_post complete %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                (hargs->postdata == NULL) ? hargs->postbuf : "success" );
        }
    }else{
        hargs->postdata = NULL;
        hargs->postbuf = NULL;
    }

    hargs->doc = NULL;
    hargs->data = NULL;
    hargs->session = NULL;
    return hargs;
}

void free_handler(struct handler_args* handler){
    if (handler != NULL){
        double start = handler->starttime;

        if (handler->doc != NULL) xmlFreeDoc(handler->doc);
        if (handler->id_index != NULL) xmlHashFree(handler->id_index,(xmlHashDeallocator) xmlFree);
        if (handler->uri_parts != NULL) free(handler->uri_parts);
        if (handler->headers   != NULL) strbuf_free_chain(handler->headers);

        if (handler->cookiesin != NULL){
            xmlHashFree(handler->cookiesin, NULL);
            free(handler->cookie_buf);
            handler->cookie_buf = NULL;
        }

        if (handler->postdata != NULL){
            xmlHashFree(handler->postdata, NULL);
            handler->postdata = NULL;
        }

        if (handler->postbuf != NULL){
            free(handler->postbuf);
            handler->postbuf = NULL;
        }

        if (handler->data != NULL){
            strbuf_free_chain(handler->data);
        }

        FCGX_Finish_r(handler->request);

        if (handler->log!=NULL) fprintf(handler->log, "%f %d %s:%d %s %f\n",
            gettime(), handler->request_id, __func__, __LINE__,
            "free_handler - request duration ",
            gettime() - start);

        fflush(handler->log);
        fclose(handler->log);
        free(handler);
    }
}


/*
 *  launch_connection_thread
 *
 *  Start 1 thread sharing a mutex and catching
 *  fastcgi connections.
 */
void launch_connection_thread(void* data){

    struct thread_launch_data* thread_dat = data;
    FCGX_Request request;
    if (FCGX_InitRequest(&request, 0, 0) != 0){
        FILE* log = fopen(thread_dat->conf->logfile_name, "a");
        fprintf(log,"%f %d %s:%d FCGX_InitRequest failed in thread %d\n",
            gettime(), 0, __func__, __LINE__, thread_dat->thread_id );

        fclose(log);
        return;
    }

    for(;;){

        pthread_mutex_lock(&(thread_dat->accept_mutex));
        int rc = FCGX_Accept_r(&request);
        next_id += 1;
        pthread_mutex_unlock(&(thread_dat->accept_mutex));

        if (rc < 0){

            int this_error = errno;
            char errbuf[BUFLEN];
            bzero(errbuf, BUFLEN);
            strerror_r(this_error, errbuf, BUFLEN)

            FILE* log = fopen(thread_dat->conf->logfile_name, "a");
            fprintf(log,"%f %d %s:%d FCGX_Accept failed on thread %d rc=%d "
                "error=%s\n",
                gettime(), next_id, __func__, __LINE__,
                thread_dat->thread_id, rc, errbuf);

            fclose(log);

        }else{
            struct handler_args* hargs;
            enum session_state this_session_state;

            if (request.out == NULL){

                FILE* log = fopen(thread_dat->conf->logfile_name, "a");
                fprintf(log, "%f %d %s:%d request with null output\n",
                    gettime(), next_id, __func__, __LINE__);

                fclose(log);
                continue;
            }

            hargs = init_handler(&request, thread_dat->envpmain, next_id,
                thread_dat->conf);

            if (hargs == NULL){
                FCGX_FPrintF(request.err, "qzfcgi init_handler returned NULL");
                continue;
            }

            hargs->session = session_from_hargs(hargs, thread_dat->sessions,
                thread_dat->conf);

            char login_uri[MAX_LOGIN_URI];
            snprintf(login_uri, MAX_LOGIN_URI, "/%s/login", hargs->uri_parts[0]);
            char logout_uri[MAX_LOGIN_URI];
            snprintf(logout_uri, MAX_LOGIN_URI,
                "/%s/logout", hargs->uri_parts[0]);

            this_session_state = get_session_state(hargs);

            // There is a document login_process.html that shows this
            // switch statement as a state table with explanations.

            switch (this_session_state){

                case bad_session:
                // logged_out is from postgres's perspective,
                case logged_out:

                close_session(hargs, hargs->session);
                hargs->session = NULL;
                // Fall through.
                // yeah really, no break.

                case no_session:
                    if (uri_part_n_is(hargs, 1, "logout")){
                        logout(hargs);
                    }else if (uri_part_n_is(hargs, 1, "login") &&
                        (hargs->nbr_uri_parts==2)) { // i.e. not validate

                        setup_session(hargs,thread_dat->sessions,
                            thread_dat->conf);

                        req_login(hargs);

                    }else{
                        location(hargs, logout_uri);
                    }
                    break;

                case session_no_login:

                // Could be:
                //   /qz/logout (from refresh failure) - park on logged out
                //   /qz/login - wants the login screen
                //   /qz/login/validate - try this password
                //   anything else - call logout

                    if (uri_part_n_is(hargs, 1, "logout")){
                        logout(hargs);

                    }else if (uri_part_n_is(hargs, 1, "login")){

                        if (uri_part_n_is(hargs, 2, "validate")){
                            if (post_contains_valid_form_tag(hargs)){

                                validate_login( hargs );
                            }else{

                                // Asking to validate but the form tag is bad.
                                close_session(hargs, hargs->session);

                                setup_session(hargs,thread_dat->sessions,
                                    thread_dat->conf);

                                location(hargs, login_uri);
                            }
                        }else{

                            setup_session(hargs,thread_dat->sessions,
                                    thread_dat->conf);

                            req_login(hargs);
                        }
                    }else{

                        setup_session(hargs,thread_dat->sessions,
                            thread_dat->conf);

                        req_login(hargs);
                    }
                    break;

                case logged_in:

                    if (uri_part_n_is(hargs, 1, "login")){

                        if (hargs->nbr_uri_parts==2) { // i.e. not validate

                            close_session(hargs, hargs->session);
                            hargs->session = NULL;

                            setup_session(hargs,thread_dat->sessions,
                                thread_dat->conf);

                            req_login(hargs);
                        }else{
                            close_session(hargs, hargs->session);
                            location(hargs, logout_uri);
                        }
                    }else{

                        do_page(hargs);

                    }
                    break;

            } // switch

            serve_output(hargs);
            free_handler(hargs);

        } // FCGX_Accept >= 0
    } // f(;;)
}

int main(int argc, char* argv[], char* envpmain[]){

    qzrandom64_init();
    struct qz_config* conf = init_config();
    init_login_tracker();

    next_id = 0;
    log_file_rotation(conf);
    FILE* log = fopen(conf->logfile_name,"a+");

    if (log == NULL) {
        fprintf(stderr, "unable to open log file %s exiting now\n",
            conf->logfile_name);

        exit(12);
    }

    // Log the startup condition
    fprintf(log, "%f %d %s:%d server startup version=%.3f\n",
        gettime(), next_id, __func__, __LINE__, QZVER);

    fprintf(log, "%f %d %s:%d expected schema version=%d\n",
        gettime(), next_id, __func__, __LINE__, SCHEMA_VER);

    fprintf(log, "%f %d %s:%d tagger_socket_path=%s\n",
        gettime(), next_id, __func__, __LINE__, conf->tagger_socket_path);

    fprintf(log, "%f %d %s:%d template_path=%s\n",
        gettime(), next_id, __func__, __LINE__, conf->template_path);

    fprintf(log, "%f %d %s:%d logfile_name=%s\n",
        gettime(), next_id, __func__, __LINE__, conf->logfile_name);

    fflush(log);
    tagger_pid = tagger_init(conf, argv);

    // In order to know the compiler did not optimize away
    // the wiping of the server token and key, I must test them.

    int tch;
    for(tch=0; tch<SERVER_TOKEN_HEX_LENGTH; tch++){
        if (conf->server_token[tch] != 0){
            fprintf(log, "%f %d %s:%d server token not wiped.\n",
                gettime(), next_id, __func__, __LINE__);

            exit(13);
       }
    }
    int kch;
    for(kch=0; kch<SERVER_KEY_HEX_LENGTH; kch++){
        if (conf->server_key[kch] != 0){
            fprintf(log, "%f %d %s:%d server key not wiped.\n",
                gettime(), next_id, __func__, __LINE__);

            exit(14);
       }
    }

    signal( SIGTERM, cleanup );

    // Do a tag test here and log it
    char tagbuf[50];
    uint64_t tagvalue_in, tagvalue_out;
    tagvalue_in = 0x42;

    make_etag(tagbuf, conf->tagger_socket_path, tagvalue_in);

    fprintf(log, "%f %d %s:%d tagger test %s\n",
        gettime(), next_id, __func__, __LINE__, tagbuf);

    tagvalue_out = validate_etag(conf->tagger_socket_path, tagbuf);

    fprintf(log, "%f %d %s:%d validate_etag %s\n",
        gettime(), next_id, __func__, __LINE__,
        (tagvalue_in == tagvalue_out) ? "OK":"fail" );
    if (tagvalue_in != tagvalue_out){
        // failed that test
        exit(15);
    }

    init_prompt_type_hash();
    init_handler_hash();

    FCGX_Init();
    fprintf(log, "%f %d %s:%d FCGX_Init complete\n",
        gettime(), next_id, __func__, __LINE__);

    FCGX_Request *request = calloc(1, sizeof(FCGX_Request));
    FCGX_InitRequest(request, 0, 0);
    fprintf(log, "%f %d %s:%d FCGX_InitRequest complete\n",
        gettime(), next_id, __func__, __LINE__);

    xmlHashTablePtr sessions = xmlHashCreate(conf->number_of_users);
    if (sessions == NULL){
        fprintf(stderr, "xmlHashCreate failed with an argument of %d\n",
            conf->number_of_users);
        exit(16);
    }
    fflush(log);

    struct thread_launch_data* thread_dat;
    pthread_mutex_t accept_mutex =  PTHREAD_MUTEX_INITIALIZER;
    pthread_t threads[conf->number_of_threads];

    int i;
    for(i = 0; i < conf->number_of_threads; i++)
    {
        thread_dat = calloc(1,sizeof(struct thread_launch_data));
        thread_dat->conf = conf;
        thread_dat->thread_id = i;
        thread_dat->accept_mutex = accept_mutex;
        thread_dat->next_id = &next_id;
        thread_dat->envpmain = envpmain;
        thread_dat->sessions = sessions;

        pthread_create(&threads[i], NULL, (void*) launch_connection_thread,
            (void*) thread_dat);
    }

    struct handler_args* housekeeper;

    for(;;){
        sleep(conf->housekeeper_nap_time);
        housekeeper = init_handler(NULL, envpmain, ++next_id, conf);
        do_housekeeping( housekeeper, sessions, conf );
        free_handler(housekeeper);
    }

    fprintf(log, "%f %d %s:%d Exited main while loop\n",
                 gettime(), next_id, __func__, __LINE__);
    exit(0);
}

