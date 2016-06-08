
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

    snprintf((char*)fs->id, 16, "%llx", qzrandom64());
    fs->zero = 0;
    fs->ref_count = 0;   // For housekeeping.
    fs->is_valid = true; // For use only once on all forms in a set.
    // 67 should be in the config file XXXXXXXXXX
    fs->context_parameters = xmlHashCreate(67);

    snprintf(fs->name, 64, "%s", name);

    xmlHashAddEntry(h->session->form_sets, fs->id, fs);
    xmlHashAddEntry(h->form_sets, fs->name, fs);

    fprintf(h->log, "%f %d %s:%d  %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        fs->name, fs->id);

    return fs;
}

/*
 *  get_form_set
 *
 *  Return the form set attached to a form record.
 */
struct form_set* get_form_set(struct handler_args* h, struct form_record* form){
    if (form == NULL) return NULL;

    struct form_set* fs = xmlHashLookup(h->session->form_sets,
        form->form_set_id);

    return fs;
}

/*
 *  remove_form_set
 *
 *  Remove the named form set from the session freeing the item.
 */

void remove_form_set(struct session* this_session, char* id){

    struct form_set* fs = xmlHashLookup(this_session->form_sets, id);

    if (fs != NULL){
       xmlHashFree(fs->context_parameters, (xmlHashDeallocator)xmlFree);
       xmlHashRemoveEntry(this_session->form_sets, (const xmlChar *)fs->id,
           (xmlHashDeallocator)xmlFree);
    }
}

void decrement_form_set(struct form_record* form_rec,xmlHashTablePtr form_sets){

    if (form_rec == NULL) return;
    if (form_rec->form_set_id[0] == '\0') return;
    if (form_rec->form_set_id[16] != '\0') return;

    struct form_set* fs = xmlHashLookup(form_sets, form_rec->form_set_id);
    if (fs != NULL){
        fs->ref_count--;
    }
}

/*
 *  set_context_paramters
 *
 *  For each key, get the value from the PG result and
 *  add it to the context_parameter hash.
 *  Also check the posted form set and carry forward any values present.
 */

void set_context_parameters(struct handler_args* h, struct form_record*
    new_form_rec, PGresult* values_rs, int row) {

    if (h->page_ta == NULL) return;
    //if (h->page_ta->set_context_parameters == false) return;
    if (h->page_ta->context_parameters == NULL) return;
    if (new_form_rec == NULL) return;

    if ((h->page_ta->form_set_name == NULL) ||
        (h->page_ta->form_set_name[0] == '\0')){

        return;
    }

    fprintf(h->log, "%f %d %s:%d form=%s action=%s,form_set_name=%s,"
        "h->page_ta->set_context_parameters=%d\n",
        gettime(), h->request_id, __func__, __LINE__,
        h->page_ta->form_name, h->page_ta->action,
        h->page_ta->form_set_name,
        h->page_ta->set_context_parameters);


    struct form_record* posted_form_rec = get_posted_form_record(h);
    if (posted_form_rec == NULL){

        fprintf(h->log, "%f %d %s:%d post_form_rec not found\n",
            gettime(), h->request_id, __func__, __LINE__);

        return;
    }

    struct form_set* posted_form_set = get_form_set(h, posted_form_rec);
    struct form_set* new_form_set = xmlHashLookup(h->form_sets,
        h->page_ta->form_set_name);

    if (posted_form_set != NULL){

        fprintf(h->log, "%f %d %s:%d posted form set name %s new set name %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            posted_form_set->name, h->page_ta->form_set_name);

        if (strcmp(posted_form_set->name, h->page_ta->form_set_name) == 0){
            // The posted form and the form being created are part of the
            // same form set.

            memcpy(new_form_rec->form_set_id, posted_form_set->id, 16);
            new_form_set = posted_form_set;

            fprintf(h->log, "%f %d %s:%d posted and new form set names match. "
                "id=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                posted_form_set->id);

        }
    }
    if (new_form_set == NULL){

        // The posted form and the form being created are different
        // form sets, or there is  no posted form set,
        // so need a new form set record.
        new_form_set = create_form_set(h, h->page_ta->form_set_name);

        fprintf(h->log, "%f %d %s:%d new form set %s.\n",
            gettime(), h->request_id, __func__, __LINE__,
            new_form_set->id);
    }
    new_form_set->ref_count++;
    memcpy(new_form_rec->form_set_id, new_form_set->id, 16);
    new_form_rec->form_set_id[16] = '\0';

    char** keys = h->page_ta->context_parameters;
    int k;
    for(k=0; keys[k] != 0; k++){

        char* value = get_value(values_rs, row, keys[k]);
        if (has_data(value)){
            // The context parameter key exists in the PG result set.

            // Remove the previous value if it exists.
            char* old_entry;
            old_entry = xmlHashLookup(new_form_set->context_parameters,keys[k]);
            if (old_entry != NULL){
                xmlHashRemoveEntry(new_form_set->context_parameters, keys[k],
                    (xmlHashDeallocator)xmlFree);
            }
            char* cparam;
            asprintf(&cparam, "%s%c%s%c", keys[k], '\0', value, '\0');
            xmlHashAddEntry(new_form_set->context_parameters, keys[k], cparam);

            fprintf(h->log, "%f %d %s:%d context_parameter %s.\n",
                gettime(), h->request_id, __func__, __LINE__,
                keys[k]);
        }
    }
}


void clear_form_set_scanner(void* payload, void* data, xmlChar* name){
    struct form_set* fs = payload;
    struct session* session = (struct session*)data;
    char* id = name;

    if (fs->context_parameters != NULL){
        xmlHashFree(fs->context_parameters, (xmlHashDeallocator)xmlFree);
    }
    xmlHashRemoveEntry(session->form_sets, id, (xmlHashDeallocator)xmlFree);
}

void clear_form_sets(struct session* session){
    xmlHashScan(session->form_sets, clear_form_set_scanner, (void*)session);
}

void form_set_housekeeping_scanner(void* payload, void* data, xmlChar* name){

    struct form_set* form_set = payload;
    struct session* session = data;

    if  (form_set->ref_count == 0){
        remove_form_set(session, form_set->id);
    }
}

