
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
 *  state_text
 *
 *  Convert the enumerated value of session_state into text.
 */
char*
state_text(enum session_state state){

    static char no_session_text[] = "no_session";
    static char bad_session_text[] = "bad_session";
    static char session_no_login_text[] = "session_no_login";
    static char logged_in_text[] = "logged_in";
    static char logged_out_text[] = "logged_out";
    static char unknown_text[] =  "unknown";

    switch( state ){
        case no_session:       return no_session_text;
        case bad_session:      return bad_session_text;
        case session_no_login: return session_no_login_text;
        case logged_in:        return logged_in_text;
        case logged_out:       return logged_out_text;
   }
   return unknown_text;
}


/*
 *  get_session_state
 *
 *  Examine the available data to determine the session state
 *  of the current request.
 */

enum session_state 
get_session_state(struct handler_args* h){ 
    
    if ( h == NULL ){ 
        // Should not happen but there is no way to log this.
        return no_session; 
    }

    if ( h->cookiesin == NULL ){
        fprintf(h->log, "%f %d %s:%d no_session-cookies is null\n", 
            gettime(), h->request_id, __func__, __LINE__);  

        return no_session;
    }

    if (strlen(h->session_key) == 0){
        fprintf(h->log, "%f %d %s:%d no_session-session key is null\n", 
            gettime(), h->request_id, __func__, __LINE__);

        return no_session;
    }

    if (h->session == NULL){
        fprintf(h->log, "%f %d %s:%d no_session-hashlookup returned null\n", 
            gettime(), h->request_id, __func__, __LINE__);

        return no_session;
    }
    
    if (!(h->session->is_logged_in)){
        fprintf(h->log, "%f %d %s:%d session no-login-is_logged_in is false\n", 
            gettime(), h->request_id, __func__, __LINE__);

        return session_no_login;
    }

    if (h->session->conn == NULL ){
        fprintf(h->log, "%f %d %s:%d logged_out-conn is null\n", 
            gettime(), h->request_id, __func__, __LINE__);

        return logged_out;
    }

    if ( PQstatus(h->session->conn) == CONNECTION_OK ){
        fprintf(h->log, "%f %d %s:%d logged_in-CONNECTION_OK user:%s\n", 
            gettime(), h->request_id, __func__, __LINE__, h->session->user);

        return logged_in;
    }

    if ( PQstatus(h->session->conn) == CONNECTION_BAD ){
        fprintf(h->log, "%f %d %s:%d logged_out-CONNECTION_BAD user:%s\n", 
            gettime(), h->request_id, __func__, __LINE__, h->session->user);

        return logged_out;
    }
    fprintf(h->log, "%f %d %s:%d logged_out\n", 
        gettime(), h->request_id, __func__, __LINE__);

    return logged_out; 
}

/*
 *  setup_session
 *
 *  Create a new session for the given handler
 */
void
setup_session(struct handler_args* hargs, 
    xmlHashTablePtr sessions, 
    struct qz_config* conf){ 

    struct session * this_session;

    // session_id is a number here, a string in the session struct,
    // and the encrypted contents of the session key, same value for each.
    uint64_t session_id;

    this_session = calloc(1, sizeof(struct session));

    // Assign a random number that has no zero octets 
    // as the session identifier, then test it.
    session_id = qzrandom64ch(this_session->session_id); 

    // There is a risk of session id collisions that
    // is larger than might be hoped from the 
    // birthday paradox.
    // With around 200 users, the chance of a hash key
    // collission is around 10^-15.  This is on the order
    // of random bit errors and cosmic ray bit flipping.
    // With around 6000 users, the chance goes up to
    // around 10^-6, or once in a million.
    // Or it could be the NSA is  messing with your PRNG, 
    // or it could be your  PRNG is setup wrong.
    // In any case, it is bad and wrong to continue.
    struct session* test_for_session;
    test_for_session = xmlHashLookup(sessions, this_session->session_id);

    if (test_for_session != NULL){
        fprintf(hargs->log, "%f %d %s:%d collision in session identifiers - %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            "terminating application now");
        
       FCGX_FPrintF(hargs->err, 
           "collision in session identifiers - ending program now\n");

       exit(49);
    }

    if (pthread_mutex_init( &(this_session->session_lock), NULL) != 0){
        fprintf(hargs->log, "%f %d %s:%d mutex_init failed\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        fflush(hargs->log);
        free(this_session);
        return;
    }

    make_etag(hargs->session_key, conf->tagger_socket_path, session_id);

    /*  Don't allow a replay attack to successfully validate to a random 64 bit
     *  number because the birthday paradox says it may happen.
     *  Save the whole string with 192 bits of data to check against.
     */
    snprintf(this_session->full_tag, SESSION_KEY_LENGTH, "%s",
        hargs->session_key);

    snprintf(this_session->tagger_socket_path, MAXPATHLEN, "%s", 
        conf->tagger_socket_path);  

    this_session->zero = 0;
    this_session->is_logged_in = false;
    this_session->logged_in_time = 0;
    this_session->logged_out_time = 0;
    this_session->last_activity_time = time(NULL);
    this_session->conn = NULL;
    
    // 197 is just an arbritrary value.
    // It is prime, hashes should be a prime size.
    // These should be in the config file. XXXXXXXXXXXXX
    this_session->opentables = xmlHashCreate(197);
    this_session->pgtype_datum = xmlHashCreate(197);
    this_session->form_tags = xmlHashCreate(197);
    this_session->form_sets = xmlHashCreate(197);

    this_session->integrity_token = conf->integrity_token;

    // index on session_id for crypto etag
    xmlHashAddEntry(sessions, this_session->session_id, this_session);

    // cookie path is /qz/ or whatever is used as the base path
    char* uri_parts[] = {hargs->uri_parts[0],"",NULL};
    char* path = build_path(uri_parts);

    make_cookie(hargs, "session_key", hargs->session_key, path,
        NULL, 0, true, true);
    free(path);

    hargs->session = this_session;

    fprintf(hargs->log, "%f %d %s:%d setup_session complete\n",
        gettime(), hargs->request_id, __func__, __LINE__);

    return;
}


/*
 *  session_from_hargs
 *
 *  Return a session from a handler struct session key
 */
struct session* 
session_from_hargs(struct handler_args* hargs, 
    xmlHashTablePtr sessions, struct qz_config* conf){ 

    uint64_t etag_payload;
    unsigned char session_id[9];
    
    struct session* this_session;

    etag_payload = validate_etag(hargs->conf->tagger_socket_path, 
        hargs->session_key);

    if (etag_payload == 0){

        fprintf(hargs->log, "%f %d %s:%d session key not validated\n", 
            gettime(), hargs->request_id, __func__, __LINE__);

        return NULL;
    }

    memcpy(session_id, &etag_payload, 8);
    session_id[8] = '\0';

    this_session = xmlHashLookup(sessions, session_id);

    if (this_session==NULL){

        fprintf(hargs->log, "%f %d %s:%d null session from hash lookup\n", 
            gettime(), hargs->request_id, __func__, __LINE__);
        
        return NULL;
    }
    
    if (this_session->integrity_token != conf->integrity_token){
        // The last 64 bits of the session struct have been 
        // altered.  The session record is corrupted.
        // Clean up the session

        close_session(hargs, this_session);

        fprintf(hargs->log,"%f %d %s:%d session closed for bad integrity token",
            gettime(), hargs->request_id, __func__, __LINE__);
        
        return NULL;
    }

    if (strncmp(this_session->full_tag, hargs->session_key,
        SESSION_KEY_LENGTH) != 0){
        // This is a double check that not only is the tag value
        // a match, but the whole tag as a string, which includes the IV.
        // Here the test failed,

        close_session(hargs, this_session);

        fprintf(hargs->log,"%f %d %s:%d session closed for bad session match",
            gettime(), hargs->request_id, __func__, __LINE__);

        return NULL;
    }

    fprintf(hargs->log, "%f %d %s:%d user=%s\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        this_session->user);

    return this_session;
}

/*
 *  close_session
 * 
 *  Cleanly close a possibly corrupted session struct
 *  Does not remove record from hash table,
 *  nor does it remove the mutex (i.e. a session can close itself).
 *  Can be called safely multiple times on a session.
 *
 *  Requires session mutex before calling.
 */

static void free_table_scanner(void* payload, void* data, xmlChar* name){
    free(payload);
}

void close_session(struct handler_args* hargs, struct session* this_session){

    if (this_session == NULL) return;

    this_session->is_logged_in = false;
    if (this_session->logged_out_time == 0){
        this_session->logged_out_time = time(NULL);
    }    

    fprintf(hargs->log, "%f %d %s:%d Closing session for %s\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        this_session->user);

    if (this_session->pgtype_datum != NULL){
        close_all_pgtype_datums(this_session);
        xmlHashFree(this_session->pgtype_datum, NULL);
        this_session->pgtype_datum = NULL;
    }
    
    if (this_session->form_tags != NULL){
        close_all_form_tags(hargs, this_session);
        xmlHashFree(this_session->form_tags, (xmlHashDeallocator)xmlFree);
        this_session->form_tags = NULL;
    }

    if (this_session->form_sets != NULL){
        struct form_tag_housekeeping_data ft_hk_data =
            (struct form_tag_housekeeping_data) {
                .this_session = this_session,
                .hargs = hargs
        };

        close_all_form_sets(&ft_hk_data);
        xmlHashFree(this_session->form_sets, NULL);
        this_session->form_sets = NULL;
    }    

    if (this_session->conn != NULL){
        // close_all_tables  clears ->opentables
        close_all_tables(hargs, this_session);

        PQfinish(this_session->conn);
        
        this_session->conn = NULL;
    }else{
        if (this_session->opentables != NULL){
            // PG session ended, but local data needs cleanup
            fprintf(hargs->log, "%f %d %s:%d %s %s\n",
                gettime(), hargs->request_id, __func__, __LINE__,
                "clearing opentables on bad connection for",
                this_session->user);
            fflush(hargs->log); 

            xmlHashFree(this_session->opentables, 
                (xmlHashDeallocator) free_table_scanner);

            this_session->opentables = NULL;
        }
    }
    return;
}

/*
 *  session_housekeeping_scanner
 *
 *  Check a specific session and clean up as required.
 */

struct session_housekeeping_data{
    int session_inactivity_timeout;
    xmlHashTablePtr sessions;
    struct handler_args* hargs;
};

void session_housekeeping_scanner(void* val, void* data, const xmlChar* name){

    struct session* this_session = val;
    struct session_housekeeping_data* shk_data = data;
    bool deleted = false;

    // Do a non blocking mutex and skip if locked.
    if (pthread_mutex_trylock(&(this_session->session_lock)) == 0){

        // If the session record is removed while pending requests
        // are queued waiting for the session mutex, then they 
        // execute with their environment yanked away.
        // Instead remove a session record in two steps,
        //   1.  Close and deallocate, but leave record in place
        //   2.  Remove the record.


        if (this_session->is_logged_in){

            if (time(NULL) > (this_session->last_activity_time + 
                shk_data->session_inactivity_timeout)){

                     // Logged in but timed out
                    fprintf(shk_data->hargs->log, 
                        "%f %d %s:%d closing session on timeout for user %s\n",
                        gettime(), shk_data->hargs->request_id, __func__,
                        __LINE__, this_session->user);

                    close_session(shk_data->hargs, this_session);
            }else{

                 // Logged in and active, work on expired forms 

                fprintf(shk_data->hargs->log, 
                    "%f %d %s:%d starting form_tag_housekeeping for user %s\n",
                    gettime(), shk_data->hargs->request_id, __func__, __LINE__,
                    this_session->user);
    
                form_tag_housekeeping(shk_data->hargs, this_session);
            }
        }else{
            // Logged out but still have record
            // 
   
            if (time(NULL) > (this_session->logged_out_time + 
                2*shk_data->session_inactivity_timeout)){
 
                close_session(shk_data->hargs, this_session);

                deleted = true;    
                pthread_mutex_unlock(&(this_session->session_lock));
                pthread_mutex_destroy(&(this_session->session_lock));
                xmlHashRemoveEntry(shk_data->sessions, name, NULL);
                free(this_session);
            }    
        }
        
        if (!deleted) pthread_mutex_unlock(&(this_session->session_lock));
    }

    return;
} 

/*
 *  do_housekeeping
 *
 *  Search for expired things.
 *  Expired sessions, form_tags,
 *
 *  This section of code is dedicated to Luz Maria.
 *  Luz Maria was the best housekeeper ever. 
 *  We miss you Luz.
 */ 

void do_housekeeping(struct handler_args* h, xmlHashTablePtr sessions, 
    struct qz_config* conf, uint64_t thread_state[]){

    double start =  gettime();

    struct session_housekeeping_data data = (struct session_housekeeping_data){
        .session_inactivity_timeout = conf->session_inactivity_timeout,
        .sessions = sessions,
        .hargs = h,
    };
    
    fprintf(h->log, 
        "%f %d %s:%d starting housekeeping\n",
        gettime(), h->request_id, __func__, __LINE__);
     
    xmlHashScan(sessions, session_housekeeping_scanner, &data);

    struct stat logsb;
    if (stat(conf->logfile_name, &logsb) == 0){
        if ((conf->max_log_file_size > 0) &&  
            (S_ISREG(logsb.st_mode)) &&
            (logsb.st_size > conf->max_log_file_size)){
    
            fprintf(h->log, "%f %d %s:%d rotating logs\n",
                gettime(), h->request_id, __func__, __LINE__);

            log_file_rotation(conf);

        }
    }

    login_tracking_housekeeping(h);

    int k;
    int active_count = 0;
    double oldest = DBL_MAX;

    for(k=0; k<conf->number_of_threads; k++){
        if (thread_state[k] > 0) active_count++;
        if ((thread_state[k] > 0) && (thread_state[k] < oldest)){
            oldest = thread_state[k];
        }
    }
    fprintf(h->log,
        "%f %d %s:%d active threads %d of %d oldest %f "
        "integrity token %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        active_count, conf->number_of_threads,
        (oldest == DBL_MAX) ? 0.0 : oldest,
        (thread_state[conf->number_of_threads] == conf->integrity_token) ?
            "valid":"invalid fail");

    fprintf(h->log, 
        "%f %d %s:%d housekeeping complete duration %f\n",
        gettime(), h->request_id, __func__, __LINE__,
        gettime() - start);
}

#ifdef SESSION_MAIN
#define NBRTESTS 5

pid_t tagger_pid = 0;

void cleanup(sig){

    // kill tagger 
    fprintf(stderr, "cleanup kill pid %d\n", tagger_pid);
    kill(tagger_pid, SIGTERM);

    int status;
    waitpid(tagger_pid, &status, 0);
    fprintf(stderr, "waitpid status = %d\n", status);
    exit(0);
}

int main(int argc, char* argv[]){

     struct qz_config* conf = init_config();

     tagger_pid = tagger_init(conf, argv);
     printf( "tagger_pid = %d\n", tagger_pid);
     signal( SIGTERM, cleanup );

     int j;
     xmlHashTablePtr ht = xmlHashCreate(NBRTESTS);
     struct handler_args hargs[NBRTESTS];
     struct session* s;

     const char* kw[] = { "host", "dbname", "user", "password", 
         "application_name", NULL };
     const char* vals[] = { "localhost", "info", "qz", "42", "qztest", NULL };

     const char* parts[] = { "qz", "one", "two", "three",  NULL };

     for(j=0; j<NBRTESTS; j++){
         fprintf(stderr, "setup_session %d\n", j);
         hargs[j].log = fopen("testsession.log", "w");
         hargs[j].conf = conf;
         hargs[j].uri_parts = parts;

         setup_session(&(hargs[j]), ht, conf);     
         hargs[j].uri_parts = NULL;
     }
     
     s = session_from_hargs(&hargs[NBRTESTS-1], ht, conf);
     s->is_logged_in = true;
     s->logged_in_time = time(NULL);

     s = session_from_hargs(&hargs[NBRTESTS-2], ht, conf);
     s->is_logged_in = true;
     s->logged_in_time = time(NULL);
     s->conn = PQconnectdbParams(kw, vals, 0);

     s = session_from_hargs(&hargs[NBRTESTS-3], ht, conf);
     s->is_logged_in = false;
     s->logged_in_time = time(NULL);
     s->conn = PQconnectdbParams(kw, vals, 0);

     for(j=0; j<NBRTESTS; j++){
         s = session_from_hargs(&hargs[j], ht, conf);
         printf("session_state[%d]:     %s\n", j,
             state_text( get_session_state(&(hargs[j])) ));
         //fclose( hargs[j].log );
     }
     printf("sleeping\n");
     sleep(5);
     printf("fin\n");

     for(j=0; j<100; j++){
         if (s->conn != NULL){
             PQfinish(s->conn);
             s->conn = NULL;
         }
     }
     kill( tagger_pid, 15);
     int status;
     waitpid(tagger_pid, &status, 0);
     fprintf(stderr, "killed tagger pid %d waitpid status = %d\n", 
         tagger_pid, status);

     return 0;
}
#endif
     
