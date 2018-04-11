
#include "qz.h"


/*
 *  create_form_set
 *
 *  Produce an empty form set.
 *  The result must be freed with remove_form_set.
 */

struct form_set* create_form_set(struct handler_args* h, char name[64]){

    struct form_set* fs = calloc(1, sizeof(struct form_set));
    if (fs == NULL) return NULL;

    qzrandom64ch(fs->id);
    fs->zero = 0;
    fs->ref_count = 0;   // For housekeeping.
    fs->is_valid = true; // For use only once on all forms in a set.
    fs->integrity_token = h->session->integrity_token;
    // 67 should be in the config file XXXXXXXXXX
    fs->context_parameters = xmlHashCreate(67);
    snprintf(fs->name, 64, "%s", name);
    xmlHashAddEntry(h->session->form_sets, fs->id, fs);

    uint64_t fsid;
    memcpy(&fsid, fs->id, 8);
    fprintf(h->log, "%f %d %s:%d  %s %"PRIx64"\n",
        gettime(), h->request_id, __func__, __LINE__,
        fs->name, fsid);

    return fs;
}

/*
 *  get_form_set
 *
 *  Return the form set attached to a form record.
 */
struct form_set* get_form_set(struct handler_args* h, char* form_set_id){

    struct form_set* fs = xmlHashLookup(h->session->form_sets, form_set_id);

    if ( ! form_set_is_valid(h, fs)){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "bad token"); 
        return NULL;

    }else{
        return fs;
    }    
}

/*
 *  form_set_is_valid
 *
 *  Return t/f for a valid form set,
 */
bool form_set_is_valid(struct handler_args* h, struct form_set* fs){
    if (fs == NULL) return false;

    if (!fs->is_valid) return false;

    if (fs->integrity_token != h->session->integrity_token){

        fprintf(h->log, "%f %d %s:%d fail session integrity token is invalid.\n",
            gettime(), h->request_id, __func__, __LINE__);

        return false;
    }

    size_t name_len = strnlen(fs->name,64);
    if ((name_len <1) || (name_len > 63)){
    
        fprintf(h->log, "%f %d %s:%d fail name length error.\n",
            gettime(), h->request_id, __func__, __LINE__);

        return false;
    }    

    if (strnlen(fs->id,10) != 8){

        fprintf(h->log, "%f %d %s:%d fail id length error.\n",
            gettime(), h->request_id, __func__, __LINE__);

        return false;
    }    
    // OK then,
    return true;
}

/*
 *  remove_form_set
 *
 *  Remove the named form set from the session freeing the item.
 */

void remove_form_set(struct form_tag_housekeeping_data* ft_hk_data, char* id){

    struct session* this_session = ft_hk_data->this_session;
    struct handler_args* hargs = ft_hk_data->hargs;

    struct form_set* fs = xmlHashLookup(this_session->form_sets, id);

    if (hargs->conf->log_form_set_details){
        uint64_t form_set_id;
        memcpy(&form_set_id, fs->id, 8);

        fprintf(hargs->log, "%f %d %s:%d form_set->id=%"PRIx64"\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            form_set_id);
    }

    if (fs != NULL){
       xmlHashFree(fs->context_parameters, (xmlHashDeallocator)xmlFree);
       xmlHashRemoveEntry(this_session->form_sets, (const xmlChar *)fs->id,
           (xmlHashDeallocator)xmlFree);
    }
}

void decrement_form_set(struct form_record* form_rec){

    if (form_rec == NULL) return;
    if (form_rec->form_set == NULL) return;

    form_rec->form_set->ref_count--;
}

/*
 *  save_context_paramters
 *
 *  For each key, get the value from the PG result and
 *  add it to the context_parameter hash.
 *  Also check the posted form set and carry forward any values present.
 */

/*
 * Conditons:
 *
 * |posted_form_record == NULL
 * |posted_form_record != NULL 
 * |current_form_set == NULL
 * |current_form_set != NULL
 * |current_form_set_name == table_action_form_set_name
 * |current_form_set_name != table_action_form_set_name
 * 
 * Actions:
 * create a new form set
 * find a form set to use
 * add pg data to hash
 * add posted form set data to hash
 * update current_form_set
 *
 */
void save_context_parameters(struct handler_args* h, 
    struct form_record* new_form_rec, PGresult* values_rs, int row){

    if (h->page_ta == NULL){
        fprintf(h->log, "%f %d %s:%d fail h->page_ta is null\n",
            gettime(), h->request_id, __func__, __LINE__);
    
        return;
    }    
    if (h->page_ta->context_parameters == NULL){
        // Nothing to do.
        return;
    }
    // This will happen when save_contest_parameters is called
    // for an empty result set. It is not an error.
    if (new_form_rec == NULL){
        fprintf(h->log, "%f %d %s:%d new_form_rec is null\n",
            gettime(), h->request_id, __func__, __LINE__);
    }
    // To use context parameters, the page must be in a form set.
    if (h->page_ta->form_set_name[0] == '\0'){
        fprintf(h->log, "%f %d %s:%d page_ta->form_set_name not set\n",
            gettime(), h->request_id, __func__, __LINE__);
    
        return;
    }    
    // 
    if (h->posted_form == NULL){
        fprintf(h->log, "%f %d %s:%d fail posted_form_rec not found\n",
            gettime(), h->request_id, __func__, __LINE__);

        return;
    }

    fprintf(h->log, "%f %d %s:%d form=%s action=%s,form_set_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        h->page_ta->form_name, h->page_ta->action,
        h->page_ta->form_set_name);
 
     
    // End prechecks

    if (h->current_form_set == NULL){
        // The table action has specified a form set name,
        // but a form set has not been created.

        h->current_form_set = create_form_set(h, h->page_ta->form_set_name);

        if (h->current_form_set == NULL){
            // This means calloc failed, not likely.
            fprintf(h->log, "%f %d %s:%d fail new form set is null.\n",
                gettime(), h->request_id, __func__, __LINE__);

            error_page(h,  SC_INTERNAL_SERVER_ERROR, "Form Set Fail");
            return;
        }
        fprintf(h->log, "%f %d %s:%d new form set %s created.\n",
            gettime(), h->request_id, __func__, __LINE__,
            h->current_form_set->name);
    }

    // Do the posted and current form sets have the same name?
    struct form_set* matching_form_set = NULL;
    
    if ((h->posted_form->form_set != NULL) &&
       (strcmp(h->posted_form->form_set->name, 
           h->page_ta->form_set_name) == 0)){

            fprintf(h->log, "%f %d %s:%d "
                "posted and current form set names match %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                h->posted_form->form_set->name);

            matching_form_set = h->posted_form->form_set;
    }


    char** key = h->page_ta->context_parameters;
    int n;
    for(n=0; key[n] != NULL; n++){

        if (item_in_list(key[n], h->page_ta->clear_context_parameters)){
            // Well, not so much clear it as do not set it.
            continue;
        }
        char* param_value = NULL;

        param_value = xmlHashLookup(h->current_form_set->context_parameters, 
            key[n]);
        if (param_value != NULL) continue; // It's already there.    

        fprintf(h->log, "%f %d %s:%d checking context_parameter %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            key[n]);

        // Maybe it is in the current actions result set.
        param_value = get_value(values_rs, row, key[n]);
        if ((param_value != NULL) && (param_value[0] != '\0')){
            fprintf(h->log, "%f %d %s:%d PG rs found value %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                param_value);
        } 

        // If not, maybe it's in the posted form set
        if ((matching_form_set != NULL) && !has_data(param_value)){        
            param_value = xmlHashLookup(
                h->posted_form->form_set->context_parameters, key[n]);

            if (has_data(param_value)){
                fprintf(h->log, "%f %d %s:%d matching form_set found value %s\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    (param_value != NULL) ? param_value:"NULL");
            }    
        }

        if (has_data(param_value)){   // Book it.
            char* context_param;
            asprintf(&context_param, "%s%c%s%c", param_value, '\0', key[n], '\0');
            char* this_key = context_param + strlen(context_param) + 1;
   
            xmlHashAddEntry(h->current_form_set->context_parameters,
                this_key, context_param);
   
            fprintf(h->log, "%f %d %s:%d set context parameter %s=%s.\n",
                gettime(), h->request_id, __func__, __LINE__,
                this_key, context_param);
        }else{

            fprintf(h->log, "%f %d %s:%d context parameter not found %s.\n",
                gettime(), h->request_id, __func__, __LINE__,
                key[n]);
        }
    }

    // Update the form record with the new form set.
    if (new_form_rec != NULL){
        new_form_rec->form_set = h->current_form_set;
        h->current_form_set->ref_count++;
    }
}

/*
 *  close_all_form_sets
 *  close_form_set_scanner
 *
 *  Remove all form sets from the current session.
 */

void close_form_set_scanner(void* payload, void* data, const xmlChar* name){
    struct form_set* fs = payload;
    struct form_tag_housekeeping_data* ft_hk_data = data;
    struct session* session = ft_hk_data->this_session;
    struct handler_args* hargs = ft_hk_data->hargs;
    char* id;
    asprintf(&id, "%s", name);

    if (fs->context_parameters != NULL){
        xmlHashFree(fs->context_parameters, (xmlHashDeallocator)xmlFree);
    }
    if (hargs->conf->log_form_set_details){

        uint64_t form_set_id;
        memcpy(&form_set_id, id, 8);

        fprintf(hargs->log, "%f %d %s:%d remove_form_set %"PRIx64".\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            form_set_id);
    }
    xmlHashRemoveEntry(session->form_sets, id, (xmlHashDeallocator)xmlFree);

    free(id);
}

void close_all_form_sets(struct form_tag_housekeeping_data* ft_hk_data){
    if (ft_hk_data->this_session->form_sets != NULL){

        xmlHashScan(ft_hk_data->this_session->form_sets,
            close_form_set_scanner, (void*)ft_hk_data);
    }
}


/*
 *  audit_form_set_scanner
 *
 *  Search form_tags for references to the form_set being audited.
 *  For each tag found, increment audit_count.
 */
struct audit_details {
    struct form_set* form_set;
    uint64_t integrity_token;
    struct handler_args* hargs;
};

void audit_form_set_scanner(void* payload, void* data, const xmlChar* name){

    struct audit_details* aud_det = data;
    struct form_set* form_set = aud_det->form_set;
    struct handler_args* h = aud_det->hargs;

    struct form_record* form_rec = payload;

    // Is there something to check?
    if (form_rec->form_set == NULL) return; // many forms are not in a set. 
    if (form_set == NULL) return;

    // Is the form set valid?
    if (form_set->integrity_token != aud_det->integrity_token){
        
        fprintf(h->log, "%f %d %s:%d form_set integrity "
           "token failed validation\n",
           gettime(), 0, __func__, __LINE__);
 
        return;
    }
    size_t name_len = strnlen(form_set->name,64);
    if ((name_len <1) || (name_len > 63)){

        fprintf(h->log, "%f %d %s:%d form_set name length fail %zu\n",
           gettime(), h->request_id, __func__, __LINE__,
           name_len);

        return;
    }    

    if (strnlen(form_set->id,10) != 8){

        fprintf(h->log, "%f %d %s:%d form_set id length fail %zu\n",
           gettime(), h->request_id, __func__, __LINE__,
           strnlen(form_set->id,10));

        return;
    }    
    //form_set  seems OK.

    // Does the form rec reference the form set being checked?
    if (strncmp(form_rec->form_set->id, form_set->id, 9) == 0){ 
        form_set->audit_count++;
    }
}

/*
 *  form_set_housekeeping_scanner
 *
 *  An xmlHashScan to remove no longer referenced form sets.
 *  If a config option is set, then audit the reference count.
 */

void form_set_housekeeping_scanner(void* payload, void* data, const xmlChar* name){

    struct form_tag_housekeeping_data* ft_hk_data = data; 
    struct session* session = ft_hk_data->this_session;
    struct form_set* form_set = payload;
    struct handler_args* h = ft_hk_data->hargs;

    if (h->conf->audit_form_set_ref_count){

        // Do a slow check of the reference count and log details.

        struct audit_details aud_det = 
            (struct audit_details) {
                .form_set = form_set,
                .integrity_token = session->integrity_token,
                .hargs = ft_hk_data->hargs
        };
    
        form_set->audit_count = 0;
        xmlHashScan(session->form_tags, audit_form_set_scanner, &aud_det);
    
        uint64_t fsid;
        memcpy(&fsid, form_set->id, 8);
        if (form_set->audit_count == form_set->ref_count){
    
            fprintf(h->log, "%f %d %s:%d form_set %s %"PRIx64" audit passed\n",
               gettime(), h->request_id, __func__, __LINE__,
               form_set->name, fsid);
    
            // What this is all about
            if  (form_set->ref_count == 0){
                remove_form_set(ft_hk_data, form_set->id);
            }    
        }else{
            fprintf(h->log, "%f %d %s:%d form_set %s %"PRIx64" "
                "ref count %"PRId64" audit count %"PRId64"\n",
                gettime(), h->request_id, __func__, __LINE__,
                form_set->name, fsid, form_set->ref_count, form_set->audit_count);
        }
    }else{
     
        // The normal case - auditing is turned off.

        if  (form_set->ref_count == 0){
            remove_form_set(ft_hk_data, form_set->id);
        }    
    }
}


#ifdef FORM_SET_MAIN

pid_t tagger_pid = 0;

int main(int argc, char* argv[]){


    // setup a fake environment
    qzrandom64_init();
    struct qz_config* conf = init_config();
 
    struct FCGX_Request freq = (struct FCGX_Request){
       .requestId = 1,
    };
    struct handler_args hargs = (struct handler_args){
       .request_id = 1,
       .request = &freq,
    };
    struct handler_args* h = &hargs;
    hargs.log = stdout;

    tagger_pid = tagger_init(conf, argv);

    struct session s;
    hargs.session = &s;
    
    doc_from_file(h, "base.xml");

    xmlNodePtr qzdiv = qzGetElementByID(h, "qz");
    
    const char* kw[] = { "host", "dbname", "user", "password",
        "application_name", NULL };

    char* vals[] = { "localhost", "test2", "qz", "42", "qztest", NULL };

    h->session->conn = PQconnectdbParams(kw, (const char* const*) vals, 0);

    if (PQstatus(h->session->conn) != CONNECTION_OK){
       fprintf(h->log, "bad connect\n");
       exit(53);
    }

    h->session->opentables = xmlHashCreate(50); 
    h->session->pgtype_datum = xmlHashCreate(50);
    h->session->form_tags = xmlHashCreate(50);
    h->request_id = 42;

    h->uri_parts =  str_to_array("/qz/form/edit", '/');
    init_open_table(h);

    char* fname;
    asprintf(&fname, "%s%c%s%c\n", "form_name", '\0', "form", '\0');
    xmlHashAddEntry(h->postdata, fname, fname);

    char* handler_name_ro;
    asprintf(&handler_name_ro, "%s%c%s%c\n", "handler_name_ro", '\0', "onetable", '\0');
    xmlHashAddEntry(h->postdata, handler_name_ro, handler_name_ro);

    h->page_ta = open_table(h, "form", "edit");
    onetable(h);


     kill( tagger_pid, 15);
     int status;
     waitpid(tagger_pid, &status, 0);
     fprintf(stderr, "killed tagger pid %d waitpid status = %d\n", 
         tagger_pid, status);

     return 0;

}
#endif

