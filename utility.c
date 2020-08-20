
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
 *  add_header
 *
 *  Adds a new header to a list of headers to be sent before
 *  the content.
 */
void add_header(struct handler_args* h, char* key, char* value){

    if (h->headers==NULL) h->headers = xmlBufferCreate();
    char* new_hdr;

    asprintf(&new_hdr, "%s: %s\r\n", key, value);
    xmlBufferCat(h->headers, new_hdr);
    free(new_hdr);

    return; 
}

/*
 *  get_uri_part
 *
 *  Return the URL segment identified by the position arg.
 *  Make it safe to address things not there.
 */
char* get_uri_part(struct handler_args* h, u_int position){
    if ( position < h->nbr_uri_parts ){
        if (strlen( h->uri_parts[ position ] ) <= MAX_SEGMENT_LENGTH ){ 
            return h->uri_parts[ position ];
        }else{ 
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d fail URI segment %d exceeds "
                "MAX_SEGMENT_LENGTH\n", 
                gettime(), h->request_id, __func__, __LINE__,
                position);
            pthread_mutex_unlock(&log_mutex);

            return NULL;
        }
    }else{
        return NULL;
    } 
}

/*
 *  uri_part_n_is
 *
 *  Return t/f if the nth URL segment is a string match for the part arg.
 */
bool uri_part_n_is(struct handler_args* h, u_int n, char* part){

    char* uri_part;

    if (n > h->nbr_uri_parts){
        return false;
    }
    uri_part = get_uri_part(h,n);

    if (uri_part == NULL) return false;
    
    return ( strncmp(uri_part, part, MAX_SEGMENT_LENGTH) == 0);
}

/*
 *  location
 *
 *  Add a location header for a redirect.
 */
void location(struct handler_args* h, char* new_url){
    add_header(h, "Location", new_url); 
    add_header(h, "Status", "302");
    FCGX_SetExitStatus(SC_FOUND, h->out);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d location: %s\n", 
        gettime(), h->request_id, __func__, __LINE__, new_url);
    pthread_mutex_unlock(&log_mutex);
    return;
}


/* 
 *  build_path
 *
 *  Given an array of strings, pack them into a buffer separated by /
 *  Kinda like python "/".join([]) but with a leading /
 *  Returned buffer must be freed by the calling code. 
 *
 *  Use asprintf instead.
 */

char* build_path(char* str_ar[]){

    int path_len = 1; // for leading /
    int j,k;
    char* new_path;

    for (k=0; str_ar[k]!=NULL; k++){
        path_len+= strlen(str_ar[k]) + 2; 
        if (k>MAX_NBR_SEGMENTS) return NULL;
    }

    new_path = calloc(1, path_len);
    int place = 0;

    for (k=0; str_ar[k]!=NULL; k++){
       new_path[place++] = '/';
       for (j=0; str_ar[k][j] != '\0'; j++){
           new_path[place++] = str_ar[k][j];
       }
    }
    return new_path; 
}

/*
 *  append_class
 *
 *  Append the given html class to the list of element classes.
 *  Check for duplicates.
 */
void append_class(xmlNodePtr the_node, xmlChar* new_class){

    xmlChar* html_class = xmlGetProp(the_node, "class");
    
    if (html_class == NULL){
        // A simple add
        xmlNewProp(the_node, "class", new_class);
        return;
    }else{
        // Class is defined
        // Check to see of class_name is already there
        // XXXXX what happens with double space?
        char** class_ar = str_to_array(html_class, ' ');
        int k;
        bool found = false;
        for (k=0; class_ar[k] != NULL; k++){
            if (strcmp(class_ar[k], new_class) == 0){
                found = true;
                break;
            }
        }
        free(class_ar);
        
        if (found){
            xmlFree(html_class);
            return;
        }

        // OK, it is really not there.
        char* new_class_list;
        asprintf(&new_class_list, "%s %s", html_class, new_class);
        xmlSetProp(the_node, "class", new_class_list);

        xmlFree(html_class);
        free(new_class_list);

        return; 
    }    
}

/*
 *  add_script_tag
 *
 *  Add an html script tag referencing src. Like this:
 *   <html><head>
 *   <script src="/qz/fs/get/qzforms.js"></script>
 *  Append it to html, head unless there is no html, head,
 *  then do nothing.
 */

void add_jscss_links(struct handler_args* h){

    if (h->doc == NULL) return;
    if (h->page_ta  == NULL){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d page_ta is null\n", 
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);
    
        return;
    }

    xmlNodePtr head = qzGetElementByID(h, "__HEAD__");
    if (head == NULL){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d failed to find head for script\n", 
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        return;
    }

    if (h->page_ta->js_filenames != NULL){

        char** js_f = h->page_ta->js_filenames;
        int k;
        xmlNodePtr script_el;

        for (k=0; js_f[k] != NULL; k++){

            char* script;
            asprintf(&script, "/%s/%s",
                get_uri_part(h, QZ_URI_BASE_SEGMENT),
                js_f[k]);

            // html script tags in xml are empty text.
            script_el = xmlNewTextChild(head, NULL, "script", "\n");
            xmlNewProp(script_el, "src", script);

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d add script %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                script);
            pthread_mutex_unlock(&log_mutex);

            free(script);
            script = NULL;
        }
    }else{
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d  js_filenames is null\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);
    }

    if (h->page_ta->css_filenames != NULL){

        char** css_f = h->page_ta->css_filenames;
        int j;
        xmlNodePtr link_el;

            for (j=0; css_f[j] != NULL; j++){
                char* link;
                asprintf(&link, "/%s/%s",
                    get_uri_part(h, QZ_URI_BASE_SEGMENT),
                    css_f[j]);

                link_el = xmlNewChild(head, NULL, "link", NULL);
            xmlNewProp(link_el, "rel", "stylesheet");
            xmlNewProp(link_el, "href", link);

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d add stylesheet %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                link);
            pthread_mutex_unlock(&log_mutex);

            free(link);
            link = NULL;
        }
    }else{
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d  css_filenames is null\n",
            gettime(), h->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);
    }

    if (has_data(h->page_ta->inline_js)){
        xmlNodePtr new_script =
            xmlNewTextChild(head, NULL, "script", h->page_ta->inline_js);

        xmlNewProp(new_script, "type", "text/javascript");
    }
    if (has_data(h->page_ta->inline_css)){
        xmlNodePtr new_style =
            xmlNewTextChild(head, NULL, "style", h->page_ta->inline_css);

        xmlNewProp(new_style, "type", "text/css");
    }

    return;
}


/*
 *  add_helpful_text
 *
 *  If the table action has a value for helpful_text,
 *  add it to the element with the id of "helpful_text" 
 */
 
void add_helpful_text(struct handler_args* h, struct table_action* ta){

    if ((ta == NULL) || (ta->helpful_text == NULL) || 
        (ta->helpful_text[0] == '\0')){

        return;
    }

    xmlNodePtr add_here = qzGetElementByID(h, "helpful_text");
    xmlNodePtr helpful_text;
    if (add_here != NULL){
        helpful_text = xmlNewTextChild(add_here, NULL, "p", ta->helpful_text);
        xmlNewProp(helpful_text, "class", "helpful_text");
    }
    return;
}


/*
 *  base64_encode
 *
 *  Return a string that is base64 encoded from the given string.
 *
 *  The result must be freed.
 */
char* base64_encode(char* astr){
    if (astr == NULL) return NULL;

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO* bio = BIO_new(BIO_s_mem());
    BIO_push(b64, bio);

    BIO_write(b64, astr, strlen(astr));
    BIO_flush(b64);
    int nbytes = BIO_pending(bio);
    char* str_encoded = calloc(1, nbytes+2);
    BIO_read(bio, str_encoded, nbytes);
    BIO_free(bio);
    BIO_free(b64);
    return str_encoded;
}

/*
 *  log_file_rotation
 *
 *  If the log file is too big, split it off.
 *  If too many files have been split off, delete the oldest.
 *
 *  This uses dirname and readdir, which may not be thread safe
 *  but it is OK because only housekeeper is using them.
 */

void log_file_rotation(struct qz_config* conf, char* logfile_name){

    // The file is too big, but need to know
    // how many numbered logs there, e.g qz.log.0
    char* lognamecopy1;
    char* lognamecopy2;
    char* logdirname;
    char* logbasename;
    unsigned int baselen;
    DIR*  logdp;
    struct dirent* dent;
    char* smallest = NULL;

    unsigned int filenbr;
    unsigned int max_filenbr = 0;
    unsigned int min_filenbr = UINT_MAX;
    unsigned int nbrcount = 0;

    asprintf(&lognamecopy1, "%s", logfile_name);
    logdirname = dirname(lognamecopy1);


    asprintf(&lognamecopy2, "%s", logfile_name);
    logbasename = basename(lognamecopy2);
    baselen = strlen(logbasename);

    if ((logdirname != NULL) && (logbasename != NULL)) {
        logdp = opendir(logdirname);
        if (logdp != NULL){

            while ((dent = readdir(logdp)) != NULL){
                // the filename must  be the same as the base name...
                if (strncmp(logbasename, dent->d_name, 
                    baselen) == 0){
                    
                    // ... and be at least 2 chars longer for ".1"...
                    if (strlen(dent->d_name) >= baselen + 2){
                        
                        // ...then if it's a number...
                        char* nbr_start = dent->d_name + baselen + 1;
                        filenbr = strtol(nbr_start, NULL, 10);

                        // ... keep track of min max and count
                        if (filenbr > 0) nbrcount++;

                        if ((filenbr > 0) && (filenbr < min_filenbr)){
                            min_filenbr = filenbr;
                            if (smallest  != NULL) free(smallest);
                            asprintf(&smallest, "%s/%s", logdirname, dent->d_name);
                        }    
                        if ((filenbr > 0) && (filenbr > max_filenbr)){
                            max_filenbr = filenbr;
                        }    
                    }
                }    
            } // while
            // .. so move log file to new numbered name
            char* newlogname;
            asprintf(&newlogname, "%s.%d", logfile_name,
                max_filenbr + 1);
            
            link(logfile_name, newlogname);
            unlink(logfile_name);
            free(newlogname);
            newlogname = NULL;

            if ((nbrcount >= conf->max_log_file_count) &&
                (smallest != NULL)){
               
                if (unlink(smallest) != 0){
                    FILE* log = fopen(logfile_name, "a");
                    pthread_mutex_lock(&log_mutex);
                    fprintf(log, "%f %d %s:%d fail on unlink %s errno=%d\n",
                        gettime(), 0, __func__, __LINE__,
                        smallest, errno);
                    pthread_mutex_unlock(&log_mutex);

                    fclose(log);
                }
            }
            closedir(logdp);
            free(smallest);
        }
    }else{
        // This is an improbable error
        FILE* log = fopen(logfile_name, "a");
        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d %s\n",
            gettime(), 0, __func__, __LINE__,
            "failed to parse log file name into directory and file name");
        pthread_mutex_unlock(&log_mutex);

       fclose(log);
    }

    free(lognamecopy1);
    free(lognamecopy2);
}

/*
 *  qzGetElementByID
 *
 *  Use the index built under doc_from_file to return
 *  the node for the id, or the magic ids
 *  __HEAD__ or __BODY__
 */
xmlNodePtr qzGetElementByID(struct handler_args* h, xmlChar* id){

    struct id_node* a_node;

    a_node = xmlHashLookup(h->id_index, id);
    if (a_node == NULL){
        return NULL;
    }
    return a_node->node;
}

/*
 *  add_listenr
 *
 *  Add an addEventListener js call to <src id=__EVENTS__...
 */
void add_listener(struct handler_args* h, char* id, char* event, char* action){

    xmlNodePtr events_node = qzGetElementByID(h, "__EVENTS__");
    if (events_node == NULL){
        xmlNodePtr head = qzGetElementByID(h, "__HEAD__");
        if (head == NULL){
            // log it and give up.
            return;
        }
        events_node = xmlNewChild(head, NULL, "script", "\n");
        xmlNewProp(events_node, "id", "__EVENTS__");
        xmlNewProp(events_node, "type", "text/javascript");

        add_to_id_index(h, events_node);
    }
    char* content;

    if (id == NULL){
        asprintf(&content, "document.addEventListener(\"%s\",%s);\n",
            event, action);

    }else{
        asprintf(&content,
            "document.getElementById(\"%s\").addEventListener(\"%s\",%s);\n",
            id, event, action);
    }
    xmlNodeAddContent(events_node, content);
    free(content);
    return;
}

/*
 *  item_in_list
 *
 *  If the first item is in the null terminated array of the second.
 */
bool item_in_list(char* item, char** list){

    if (list == NULL) return false;

    int k;
    for(k=0; list[k] != NULL; k++){
        if (strcmp(item, list[k]) == 0){
            return true;
        }
    }
    return false;
}

/*
 *  array_base
 *
 *  Given an array name, field[5], return the name without
 *  the brackets and index.
 *  The result must be freed.
 */
char* array_base(const char* name){

    char* base;
    char ch;
    int len, j;
    bool is_valid = false;

    len = asprintf(&base, "%s", name);

    if (base == NULL) return NULL;
    if ((len == 0) || (base[len-1] != ']')){
        free(base);
        return NULL;
    }

    // Count backwards from the end of the string to the [ char
    for(j=len-1; j>0; j--){  // This does skip the first char, "x[0]" not "[0]"
       ch = base[j];
       base[j] = '\0';
       if (ch == '['){
           is_valid = true;
           break;
       }
    }
    if (is_valid){
        return base;
    }else{
        free(base);
        return NULL;
    }
}

#ifdef ARRAY_BASE_TEST
pthread_mutex_t log_mutex;

int main(void){

    char* test[] = {"test[1]", "X[2]", "[3]", "test4]", "test5",
        "test[[6]", "test[7][x]", "test[]", NULL};
    char* base;
    pthread_mutex_init(&log_mutex,NULL);

    int n;
    for (n=0; test[n] != NULL; n++){
        base = array_base(test[n]);
        printf("%d %s %s\n", n, test[n], base);
        free(base);
    }
}
#endif
