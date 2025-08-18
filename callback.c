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
 *  callback_name_lookup
 *
 *  Turn a callback name into an enum.
 *  Default to plain_text on error, no error reporting.
 */
enum callback_response_type callback_name_lookup(char* type_name){

    // {plain_text,qzforms_json,postgresql_json,html_table}

    if ( ! has_data(type_name)) return plain_text;

    // switch on the 2nd char since p* happens twice
    char ch = type_name[1];
    switch (ch){
        case 'l':
            if (strcmp("plain_text", type_name) == 0){
                return plain_text;
            }
            break;

        case 'z':
            if (strcmp("qzforms_json", type_name) == 0){
                return qzforms_json;
            }
            break;

        case 'o':
            if (strcmp("postgresql_json", type_name) == 0){
                return postgresql_json;
            }
            break;

        case 't':
           if (strcmp("html_table", type_name) == 0){
               return html_table;
           }
           break;
    }
    return plain_text;
}

/*
 *  callback_enum_lookup
 *
 *  Turn an enum response type into a text string
 *  Do not free
 */
char* callback_enum_lookup(enum callback_response_type cb_response){
    static char plain_text_txt[] = "plain_text";
    static char qzforms_json_txt[] = "qzforms_json";
    static char postgresql_json_txt[] = "postgresql_json";
    static char html_table_txt[] = "html_table";
    static char error_txt[] = "error";

    switch (cb_response){
        case plain_text:      return plain_text_txt;
        case qzforms_json:    return qzforms_json_txt;
        case postgresql_json: return postgresql_json_txt;
        case html_table:      return html_table_txt;
    }
    return error_txt;
}
/*
 *  callback_adder
 *  Add a callback:
 *     1. to registered forms
 *     2. to form as unnamed button
 *     3. to javascript __EVENTS__ section as an addEventListener call
 *
 *  <button
 *    id="__cbid0"
 *    formaction="/qz/worktasks/project"
 *    type="button"
 *    class="callbacks"
 *    hidden="hidden"
 *    x_form_tag="1734eff3a19dd84f57b9ac011fc803df.90636a742c54eb24b1b20a12edf5546da939776097398aa81c666f633e60bd7f"
 *  >
 *
 *  There are 2 table_action records in this function.
 *  ta is for the form being added
 *  cb_ta is for the callback being added
 */
void callback_adder(struct handler_args* h, xmlNodePtr form_node,
    struct table_action* ta){

    if ((ta == NULL) || (ta->callbacks == NULL) || (form_node == NULL)){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail NULL value: %s%s%s\n",
            gettime(), h->request_id, __func__, __LINE__,
            (ta == NULL) ? "ta ":"",
            ((ta != NULL) && (ta->callbacks == NULL)) ? "ta->callbacks ":"",
            (form_node == NULL) ? "node":"");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    int cbn;
    for (cbn=0; ta->callbacks[cbn] != NULL; cbn++){
        char* form_action;
        char tagbuf[ETAG_MAX_LENGTH];

        char* cbid;
        asprintf(&cbid, "__cbid%d", h->counter++);

        struct table_action* cb_ta =
            open_table(h, ta->form_name, ta->callbacks[cbn]);

        if (cb_ta == NULL){

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d open_table %s failed \n",
                gettime(), h->request_id, __func__, __LINE__,
                ta->callbacks[cbn]);
            pthread_mutex_unlock(&log_mutex);

            return;
        }
        // register the form

        asprintf(&form_action, "/%s/%s/%s", h->uri_parts[0],
            ta->form_name, ta->callbacks[cbn]);

        struct form_record* cb_form_rec =
            register_form(h, NULL, false, form_action);

        if (cb_form_rec == NULL){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                "fail cb_form_rec is null");
        }else{
            if (h->conf->log_callback_details){
                pthread_mutex_lock(&log_mutex);
                fprintf(h->log, "%f %d %s:%d callback form registererd %s\n",
                    gettime(), h->request_id, __func__, __LINE__,
                     cb_form_rec->form_name);
                pthread_mutex_unlock(&log_mutex);
            }
        }

        make_etag(tagbuf, h->conf->tagger_socket_path,
            h->session->form_tag_token, cb_form_rec->form_id);

        xmlNodePtr callback_button = xmlNewChild(form_node, NULL, "button", "\n");
        xmlNewProp(callback_button, "id", cbid);
        xmlNewProp(callback_button, "formaction", form_action);
        xmlNewProp(callback_button, "type", "button");
        xmlNewProp(callback_button, "class", "__CALLBACKS__");
        xmlNewProp(callback_button, "hidden", "hidden");
        xmlNewProp(callback_button, "x-form_tag", tagbuf);
        xmlNewProp(callback_button, "value", ta->callbacks[cbn]);
        xmlNodeAddContent(callback_button, ta->callbacks[cbn]);
        xmlAddNextSibling(callback_button, xmlNewText("\n"));

        // A bit of quoting madness going on.
        // JSON wants to use double quotes around each value,
        // but libxml wants to use double quotes around the whole list.
        // "["one","two","three"]" needs to be "['one','two','three']"
        // Javascript will need to replace quotes:
        // JSON.parse(xfieldnames.value.replace(/\'/g, '"'));

        char* fields = NULL;
        if (cb_ta->fieldnames){
            fields = str_ar_to_json(cb_ta->fieldnames);
            int k;
            for (k=0; k<strlen(fields); k++){
               if (fields[k] == '"') fields[k] = '\'';
            }
        }
        if (fields != NULL){
            xmlNewProp(callback_button, "x-fieldnames", fields);
            if (h->conf->log_callback_details){
                pthread_mutex_lock(&log_mutex);
                fprintf(h->log, "%f %d %s:%d callback fieldnames %s\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    fields);
                pthread_mutex_unlock(&log_mutex);
            }
            free(fields);
        }
        free(form_action);
    }
}

/*
 *  callback_plain_text
 *
 *  Return tab delimited plain text.
 */
void callback_plain_text(struct handler_args* h, PGresult* cb_rs){

    int nr;
    int nf;
    if (h->data == NULL) h->data = xmlBufferCreate();

    for (nr=0; nr < PQntuples(cb_rs); nr++){
        if (nr != 0) xmlBufferCat(h->data, "\r\n");
        for (nf=0; nf < PQnfields(cb_rs); nf++){
             if (nf != 0) xmlBufferCat(h->data, "\t");
             xmlBufferCat(h->data, PQgetvalue(cb_rs, nr, nf));
        }
    }
    content_type(h, "text/plain");

    return;
}

/*
 *  callback_qzforms_json
 *
 *  QZForms will turn the result set into json
 */

void json_escape_to_buf(struct handler_args* h, char* str){

     int nch;
     char cstr[2];
     cstr[1] = '\0';
     for (nch=0; str[nch]!='\0'; nch++){

         switch(str[nch]){
             case '\\':  xmlBufferCat(h->data, "\\"); break;
             case '/':  xmlBufferCat(h->data, "\\/"); break;
             case '\b': xmlBufferCat(h->data, "\\b"); break;
             case '\f': xmlBufferCat(h->data, "\\f"); break;
             case '\n': xmlBufferCat(h->data, "\\n"); break;
             case '\r': xmlBufferCat(h->data, "\\r"); break;
             case '\t': xmlBufferCat(h->data, "\\t"); break;
             default:
                 cstr[0] = str[nch];
                 xmlBufferCat(h->data, cstr);
                 break;

         }
     }
}
void callback_qzforms_json(struct handler_args* h, PGresult* cb_rs){

    if (h->data == NULL) h->data = xmlBufferCreate();

    xmlBufferCat(h->data, "[\r\n");

    int nr;
    int nf;

    for (nr=0; nr < PQntuples(cb_rs); nr++){
        if (nr == 0){
            xmlBufferCat(h->data, "  {\r\n");
        }else{
            xmlBufferCat(h->data, ",\r\n  {\r\n");
        }
        for (nf=0; nf < PQnfields(cb_rs); nf++){

             if (nf == 0){
                 xmlBufferCat(h->data, "    ");
             }else{
                 xmlBufferCat(h->data, ",\r\n    ");
             }
             // field name
             xmlBufferCat(h->data, "\"");
             xmlBufferCat(h->data, PQfname(cb_rs, nf));
             xmlBufferCat(h->data, "\": ");

             // attribute
             xmlBufferCat(h->data, "\"");
             json_escape_to_buf(h, PQgetvalue(cb_rs, nr, nf));
             xmlBufferCat(h->data, "\"");
        }
        xmlBufferCat(h->data, "\r\n  }");
    }
    xmlBufferCat(h->data, "\r\n]\r\n");

    content_type(h, "application/json");
}

/*
 *  callback_postgresql_json
 *
 *  The Postgresql result set is already json,
 *  return it.
 */
void callback_postgresql_json(struct handler_args* h, PGresult* cb_rs){

    int nr;
    int nf;
    if (h->data == NULL) h->data = xmlBufferCreate();

    for (nr=0; nr < PQntuples(cb_rs); nr++){
        for (nf=0; nf < PQnfields(cb_rs); nf++){
             xmlBufferCat(h->data, PQgetvalue(cb_rs, nr, nf));
        }
    }

    content_type(h, "application/json");
}

/*
 *  callback_html_table
 *
 *  Turn the Postgresql result set into an html table
 */
void callback_html_table(struct handler_args* h, PGresult* cb_rs){

    h->doc = xmlNewDoc("1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, "div");
    xmlDocSetRootElement(h->doc, root_node);

    rs_to_table(root_node, cb_rs, NULL);

    content_type(h, "text/html");

    return;
}

// XXXXXXXXXX I think this is dead code.
PGresult* server_callback(struct handler_args* h, PGresult* form_rs, int row,
    char* form_name, char* callback_name){

    char* fname;
    char* fvalue;
    int k;
    PGresult* cb_rs;

    struct table_action* cb_ta = open_table(h, form_name, callback_name);

    char* params[ cb_ta->nbr_params + 1 ];

    for (k=0; k<cb_ta->nbr_params; k++){
        fname = cb_ta->fieldnames[k];
        fvalue = get_value(form_rs, row, fname);

        if (( ! has_data(fvalue) && (h->current_form_set != NULL)) ){
            fvalue = xmlHashLookup(h->current_form_set->context_parameters,
            fvalue);
        }
        if (has_data(fvalue)){
            params[k] = fvalue;
        }else{

        }
    }
    params[cb_ta->nbr_params] = NULL;

    cb_rs = perform_action(h, cb_ta, params);

    return cb_rs;

}
void callback(struct handler_args* h){

    struct table_action* cb_ta;
    cb_ta = open_table(h, get_uri_part(h, QZ_URI_FORM_NAME),
        get_uri_part(h, QZ_URI_ACTION));

    PGresult* cb_rs = perform_post_action(h, cb_ta);

    switch (cb_ta->callback_response){

        case plain_text:
            callback_plain_text(h, cb_rs);
            break;

        case qzforms_json:
            callback_qzforms_json(h, cb_rs);
            break;

        case postgresql_json:
            callback_postgresql_json(h, cb_rs);
            break;

        case html_table:
            callback_html_table(h, cb_rs);
            break;
    }
    PQclear(cb_rs);
    return;
}
