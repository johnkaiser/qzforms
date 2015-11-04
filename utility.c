
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
    int keysize = strlen(key);
    int valsize = strlen(value);

    // 3 is ": " plus \0
    struct strbuf* new_header = new_strbuf(NULL, keysize+valsize+3);

    strncpy(new_header->str, key, keysize+1 );
    strncat(new_header->str, ": ", 3);
    strncat(new_header->str, value, valsize+1);

    new_header->next = h->headers;
    new_header->prev = NULL;

    if (h->headers != NULL){
        h->headers->prev = new_header;
    }
    h->headers = new_header; 
    
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
            fprintf(h->log, "%f %d %s:%d fail URI segment %d exceeds "
                "MAX_SEGMENT_LENGTH\n", 
                gettime(), h->request_id, __func__, __LINE__,
                position);

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

    fprintf(h->log, "%f %d %s:%d location: %s\n", 
        gettime(), h->request_id, __func__, __LINE__, new_url);
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

void add_jscss_links(struct handler_args* h, xmlDocPtr doc){

    if (doc == NULL) return;
    if (h->page_ta  == NULL){
        fprintf(h->log, "%f %d %s:%d page_ta is null\n", 
            gettime(), h->request_id, __func__, __LINE__);
    
        return;
    }

    xmlNodePtr root_el;
    if ((root_el = xmlDocGetRootElement(doc)) == NULL) return; 

    fprintf(h->log, "%f %d %s:%d root_el->name=%s\n", 
        gettime(), h->request_id, __func__, __LINE__, 
        root_el->name);

    if (xmlStrcmp(root_el->name, "html") != 0) return;

    xmlNodePtr child = root_el->xmlChildrenNode;

    // If there is a text node under html skip over it.
    if (xmlStrEqual(child->name, "text") && 
        (child->xmlChildrenNode != NULL)) {

          child = child->xmlChildrenNode->next;
    }

    bool found_head = false;

    while (child != NULL){
        fprintf(h->log, "%f %d %s:%d child->name=%s\n", 
            gettime(), h->request_id, __func__, __LINE__, 
            child->name);

        if (xmlStrEqual(child->name, "head")){
            xmlNodePtr head = child;
            
            found_head = true;

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

                    fprintf(h->log, "%f %d %s:%d add script %s\n",
                        gettime(), h->request_id, __func__, __LINE__,
                        script);

                    free(script);
                    script = NULL;
                }
            }else{
                fprintf(h->log, "%f %d %s:%d  js_filenames is null\n",
                    gettime(), h->request_id, __func__, __LINE__);
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

                    fprintf(h->log, "%f %d %s:%d add stylesheet %s\n",
                        gettime(), h->request_id, __func__, __LINE__,
                        link);

                    free(link);
                    link = NULL;
                }
            }else{
                fprintf(h->log, "%f %d %s:%d  css_filenames is null\n",
                    gettime(), h->request_id, __func__, __LINE__);
            }

            // stop here 
            child = NULL;
        }else{
           child = child->next; 
        }   
    }

    if (! found_head){
        fprintf(h->log, "%f %d %s:%d failed to find head for script\n", 
            gettime(), h->request_id, __func__, __LINE__);
    }
    return;
}


/*
 *  add_helpful_text
 *
 *  If the table action has a value for helpful_text,
 *  add it to the element with the id of "helpful_text" 
 */
 
void add_helpful_text(struct handler_args* h, struct table_action* ta,
    xmlNodePtr root_node){

    if ((ta == NULL) || (ta->helpful_text == NULL) || 
        (ta->helpful_text[0] == '\0')){

        return;
    }

    xmlNodePtr add_here = qzGetElementByID(h, root_node, "helpful_text");
    xmlNodePtr helpful_text;
    if (add_here != NULL){
        helpful_text = xmlNewTextChild(add_here, NULL, "p", ta->helpful_text);
        xmlNewProp(helpful_text, "class", "helpful_text");
    }
    return;
}
