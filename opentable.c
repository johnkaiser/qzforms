
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
 * init_open_table
 *
 * Called once per session during login.
 * Prepare a statement to access the table_action table,
 * i.e., open the open table table.
 */

void init_open_table(struct handler_args* h){

    PGresult* rs;  // reused without apology.

    // But first, check schema version for trouble.
    char schema_version_query[] =
        "SELECT schema_version,  $1::int expected_version, "
        "$1::int = schema_version does_match "
        "FROM qz.constants";

    const char* sv_ver[2];
    sv_ver[0] = SCHEMA_VER;
    sv_ver[1] = NULL;

    PGresult* sv_rs = PQprepare(h->session->conn, "schema_version",
        schema_version_query, 0, NULL);

    if (PQresultStatus(sv_rs) != PGRES_COMMAND_OK){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "prepare schema_version failed");
        return;
    }

    rs = PQexecPrepared(h->session->conn, "schema_version", 1,
        (const char * const *) &sv_ver, NULL, NULL, 0);

    if ( PQresultStatus(rs) == PGRES_TUPLES_OK ){
        char* does_match = PQgetvalue(rs, 0, 2);
        if ( does_match[0] == 't'){

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d schema_version %s matches\n",
                gettime(), h->request_id, __func__, __LINE__,
                SCHEMA_VER);
            pthread_mutex_unlock(&log_mutex);

        }else{
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d fail schema_version mismatch "
                "program expected %s but database is %s\n",
                gettime(), h->request_id, __func__, __LINE__,
                SCHEMA_VER, PQgetvalue(rs, 0, 0));
            pthread_mutex_unlock(&log_mutex);
        }
    }else{
        char* sv_ver_err_msg = nlfree_error_msg(rs);
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail schema_version mismatch check %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            sv_ver_err_msg);
        pthread_mutex_unlock(&log_mutex);
        free(sv_ver_err_msg);
        sv_ver_err_msg = NULL;
    }
    PQclear(sv_rs);
    PQclear(rs);

    // fetch_table_action

    char fetch_table_action[] =
        "SELECT fm.schema_name, fm.table_name, ta.sql, ta.fieldnames, "
        "fm.pkey, ta.etag, ta.clear_context_parameters, fm.target_div, "
        "fm.xml_template, fm.add_description, "
        "fm.prompt_container, fm.form_set_name, fs.context_parameters, "
        "ta.helpful_text, ta.inline_js, ta.inline_css, "
        "ta.is_callback, ta.callback_attached_action, ta.callback_response, "
        "CASE WHEN ta.is_callback THEN "
        "   'callback' "
        "ELSE "
        "  fm.handler_name "
        "END handler_name, "
        "CASE WHEN ta.is_callback THEN "
        "   NULL "
        "ELSE "
        "  ARRAY( "
        "    SELECT action "
        "    FROM qz.table_action cbta "
        "    WHERE cbta.form_name = $1 "
        "    AND (cbta.callback_attached_action) IN ('any', $2) "
        "    AND cbta.is_callback "
        "    ORDER BY action "
        "  ) "
        "END callbacks, "
        "ARRAY( "
        "  SELECT 'js/get/'|| f.filename filename "
        "  FROM qz.page_js f "
        "  WHERE f.form_name = $1 "
        "  ORDER BY sequence "
        ") js_filenames, "
        "ARRAY( "
        "  SELECT 'css/get/' || c.filename filename "
        "  FROM qz.page_css c "
        "  WHERE c.form_name = $1 "
        "  ORDER BY sequence "
        ") css_filenames "
        "FROM qz.table_action ta "
        "JOIN qz.form fm USING (form_name) "
        "LEFT JOIN qz.form_set fs ON fm.form_set_name = fs.set_name "
        "WHERE ta.form_name = $1 "
        "AND ta.action = $2";


    rs = PQprepare(h->session->conn, "fetch_table_action", fetch_table_action, 0, NULL);

    char* error_msg = nlfree_error_msg(rs);
    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d %s %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        "fetch_table_action", PQresStatus(PQresultStatus(rs)), error_msg);
    pthread_mutex_unlock(&log_mutex);
    free(error_msg);
    error_msg = NULL;

    if (PQresultStatus(rs) != PGRES_COMMAND_OK){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "prepare fetch_table_action failed");
        return;
    }

    PQclear(rs);

    // fetch_datum

    char fetch_datum[] =
        "SELECT version, table_schema, table_name, column_name, "
        "ordinal_position, column_default, is_nullable, typname, typtype, "
        "typdelim, is_base_type, is_boolean, is_composite, is_domain, is_enum, "
        "is_pseudo_type, character_maximum_length, "
        "character_octet_length, numeric_precision, numeric_precision_radix, "
        "numeric_scale, datetime_precision, typcategory_name, "
        "domain_schema, domain_name, domain_check_clause, "
        "udt_schema, udt_name, "
        "description, is_updatable, enum_labels, composite_attributes "
        "has_fkey, fkey_schema, fkey_table, fkey_attribute "
        "FROM qz.get_pgtype_datum($1, $2)";

    rs = PQprepare(h->session->conn, "fetch_datum",  fetch_datum, 0, NULL);

    error_msg = nlfree_error_msg(rs);
    pthread_mutex_lock(&log_mutex);
    fprintf(h->log,
        "%f %d %s:%d fetch_datum resStatus:%s cmdStatus:%s %s%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)),
        PQcmdStatus(rs),
        (strlen(error_msg) > 0) ? "ErrorMessage:" : "",
        error_msg);
    pthread_mutex_unlock(&log_mutex);

    if (PQresultStatus(rs) != PGRES_COMMAND_OK){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "prepare fetch_datum failed");
        return;
    }
 
    free(error_msg);
    error_msg = NULL;
    PQclear(rs);

    //  fetch_table_action_etag

    char fetch_table_action_etag[] =
        "SELECT etag "
        "FROM qz.table_action "
        "WHERE form_name = $1 AND action = $2";

    rs = PQprepare(h->session->conn, "fetch_table_action_etag",
       fetch_table_action_etag, 0, NULL);

    error_msg = nlfree_error_msg(rs);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d fetch_table_action_etag "
        "resStatus:%s cmdStatus:%s %s%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)),
        PQcmdStatus(rs),
        (strlen(error_msg) > 0) ? "ErrorMessage:" : "",
        error_msg);
    pthread_mutex_unlock(&log_mutex);

    if (PQresultStatus(rs) != PGRES_COMMAND_OK){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "prepare fetch_table_action_etag failed");
        return;
    }
 
    free(error_msg);
    error_msg = NULL;
    PQclear(rs);

    // fetch_prompt_rules

    char fetch_rule[] =
        "SELECT form_name, fieldname, prompt_type, el_class, "
        "readonly, required, rows, cols, size, maxlength, tabindex, "
        "publish_pgtype, expand_percent_n, regex_pattern, options, src, "
        "onfocus, onblur, onchange, onselect, onclick, ondblclick, "
        "onmousedown, onmouseup, onmouseover, onmouseout, onkeypress, "
        "onkeydown, onkeyup, tabindex "
        "FROM qz.prompt_rule "
        "WHERE form_name = $1 AND fieldname = $2";


    rs = PQprepare(h->session->conn, "fetch_rule",
        fetch_rule, 0, NULL);

    error_msg = nlfree_error_msg(rs);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d fetch_rule "
        "resStatus:%s cmdStatus:%s %s%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)),
        PQcmdStatus(rs),
        (strlen(error_msg) > 0) ? "ErrorMessage:" : "",
        error_msg);
    pthread_mutex_unlock(&log_mutex);

    free(error_msg);
    error_msg = NULL;

    if (PQresultStatus(rs) != PGRES_COMMAND_OK){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "prepare fetch_rule failed");
        return;
    }

    PQclear(rs);

    char table_action_exists[] =
        "SELECT form_name, action "
        "FROM qz.table_action "
        "WHERE form_name = $1 "
        "AND action = $2";

    rs = PQprepare(h->session->conn, "table_action_exists",
        table_action_exists, 0, NULL);

    error_msg = nlfree_error_msg(rs);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d table_action_exists "
        "resStatus:%s cmdStatus:%s %s%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)),
        PQcmdStatus(rs),
        (strlen(error_msg) > 0) ? "ErrorMessage:" : "",
        error_msg);
    pthread_mutex_unlock(&log_mutex);

    if (PQresultStatus(rs) != PGRES_COMMAND_OK){
        error_page(h, SC_INTERNAL_SERVER_ERROR, "prepare table_action_exists failed");
        return;
    }

    free(error_msg);
    PQclear(rs);

    return;
}

/*
 *  log_table_action_details
 *
 *  Just for debugging.
 */
void log_table_action_details(struct handler_args* h,
    struct table_action* ta){
    int k;

    pthread_mutex_lock(&log_mutex);

    fprintf(h->log, "%f %d %s:%d form_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->form_name);

    fprintf(h->log, "%f %d %s:%d action=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->action);

    fprintf(h->log, "%f %d %s:%d schema_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->schema_name);

    fprintf(h->log, "%f %d %s:%d table_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->table_name);

    fprintf(h->log, "%f %d %s:%d prepare_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->prepare_name);

    fprintf(h->log, "%f %d %s:%d etag=%"PRIx64"\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->etag);

    if (ta->clear_context_parameters != NULL){
        for (k=0; ta->clear_context_parameters[k] != NULL; k++){
            fprintf(h->log, "%f %d %s:%d clear context_parameters=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                ta->clear_context_parameters[k]);
        }
    }

    fprintf(h->log, "%f %d %s:%d nbr_params=%d\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->nbr_params);

    for (k=0; k<ta->nbr_params; k++){
        fprintf(h->log, "%f %d %s:%d fieldnames[%d]=%s\n",
            gettime(), h->request_id, __func__, __LINE__,
            k, ta->fieldnames[k]);
    }

    for (k=0; k<ta->nbr_pkeys; k++){
        fprintf(h->log, "%f %d %s:%d pkey[%d]=%s\n",
            gettime(), h->request_id, __func__, __LINE__,
            k, ta->pkeys[k]);
    }

    fprintf(h->log, "%f %d %s:%d target_div=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->target_div);

    fprintf(h->log, "%f %d %s:%d xml_template=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->xml_template);

    fprintf(h->log, "%f %d %s:%d handler_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->handler_name);

    fprintf(h->log, "%f %d %s:%d add_description=%c\n",
        gettime(), h->request_id, __func__, __LINE__,
        (ta->add_description) ? 't':'f');

    fprintf(h->log, "%f %d %s:%d is_callback=%c\n",
        gettime(), h->request_id, __func__, __LINE__,
        (ta->is_callback) ? 't':'f');

    fprintf(h->log, "%f %d %s:%d prompt_container=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->prompt_container);

    if (ta->callbacks != NULL){
        for (k=0; ta->callbacks[k] != NULL; k++){
            fprintf(h->log, "%f %d %s:%d callback=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                ta->callbacks[k]);
        }
    }

    if (ta->js_filenames != NULL){
        for (k=0; ta->js_filenames[k] != NULL; k++){
            fprintf(h->log, "%f %d %s:%d js_filename=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                ta->js_filenames[k]);
        }
    }

    if (ta->css_filenames != NULL){
        for (k=0; ta->css_filenames[k] != NULL; k++){
            fprintf(h->log, "%f %d %s:%d css_filename=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                ta->css_filenames[k]);
        }
    }

    fprintf(h->log, "%f %d %s:%d inline_js %zu bytes\n",
        gettime(), h->request_id, __func__, __LINE__,
        (ta->inline_js == NULL) ? 0 : strlen(ta->inline_js));

    fprintf(h->log, "%f %d %s:%d inline_css %zu bytes\n",
        gettime(), h->request_id, __func__, __LINE__,
        (ta->inline_css == NULL) ? 0 : strlen(ta->inline_css));

    fprintf(h->log, "%f %d %s:%d form_set_name=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->form_set_name);

    if (ta->context_parameters != NULL){
        for (k=0; ta->context_parameters[k] != NULL; k++){
            fprintf(h->log, "%f %d %s:%d context_parameters=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                ta->context_parameters[k]);
        }
    }

    fprintf(h->log, "%f %d %s:%d callback_attached_action=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->callback_attached_action);

    fprintf(h->log, "%f %d %s:%d callback_response=%s\n",
        gettime(), h->request_id, __func__, __LINE__,
        callback_enum_lookup(ta->callback_response));

    fprintf(h->log, "%f %d %s:%d integrity_token=%"PRIx64"\n",
        gettime(), h->request_id, __func__, __LINE__,
        ta->integrity_token);

    pthread_mutex_unlock(&log_mutex);
}

/*
 *  init_table_entry
 *
 *  Create a new entry for the open table hash.
 *  It is a table_action struct plus the memory for the strings.
 *  This should be called once per session by open_table
 *  only when it is looked for but not found.
 */

void init_table_entry(struct handler_args* hargs,
    char* form_name,  char* action){

    static char empty[] = "\0\0\0";
    static char base_xml_template[] = "base.xml";
    static char default_prompt_container[] = "fieldset";

    pthread_mutex_lock(&log_mutex);
    fprintf(hargs->log, "%f %d %s:%d %s(%s,%s)\n",
        gettime(), hargs->request_id, __func__, __LINE__,
        "start init_table_entry", form_name, action);
    pthread_mutex_unlock(&log_mutex);

    if (form_name == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d fail, form_name is null\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        return;
    }

    if (action == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d fail, action is null\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        return;
    }

    struct table_action* new_table_action  = NULL;

    // Need to add up the size of all the chunks.
    int table_entry_size = sizeof(struct table_action);

    int form_name_len = strlen(form_name);
    table_entry_size+= form_name_len+2;

    int action_len = strlen(action);
    table_entry_size+= action_len+2;

    // The prepared name is generated to include random chars.
    char prepared_name[PG_NAMEDATALEN];
    bzero(prepared_name,PG_NAMEDATALEN);

    char random_chars[NAME_RANDOMNESS];
    gen_random_key(random_chars, NAME_RANDOMNESS);

    int pn_size = snprintf(prepared_name, PG_NAMEDATALEN-1, "%s_%s_",
        form_name, action);

    // If the names were too long and there is not room for the
    // random bytes, then shift back some.
    if ( (pn_size + NAME_RANDOMNESS) > (PG_NAMEDATALEN) ){
        pn_size = (PG_NAMEDATALEN - 1) - NAME_RANDOMNESS;
        prepared_name[pn_size] = '\0';
    }

    strncat(prepared_name, random_chars, NAME_RANDOMNESS-1);
    int prepared_name_len = strlen(prepared_name);
    table_entry_size+= prepared_name_len+2;

    //  Get the table_action from postgres.
    char* paramValues[3];
    paramValues[0] = form_name;
    paramValues[1] = action;
    paramValues[2] = NULL;

    static char view[] = "view";
    if ( ! has_data(paramValues[1]) && strcmp("menupage", hargs->handler_name)==0){
        paramValues[1] = view;
    }

    PGresult* rs_table_action;

    rs_table_action = PQexecPrepared(hargs->session->conn, "fetch_table_action",
                         2, (const char * const *) &paramValues, NULL, NULL, 0);

    if (hargs->conf->log_table_action_details){
        char* error_msg = nlfree_error_msg(rs_table_action);

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d rs_table_action(%s, %s) = %s,%s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            paramValues[0],
            paramValues[1],
            PQresStatus( PQresultStatus(rs_table_action) ),
            PQresultErrorMessage(rs_table_action));
        pthread_mutex_unlock(&log_mutex);

        free(error_msg);
    }

    if (PQntuples(rs_table_action) == 0){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d %s (%s, %s) %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            "warning, requested table_action tuple",
            paramValues[0],
            paramValues[1],
            "does not exist.");
        pthread_mutex_unlock(&log_mutex);

        PQclear(rs_table_action);
        return;
    }

    if (PQntuples(rs_table_action) > 1){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log,
            "%f %d %s:%d fail, %d rows, but there should be only 1.\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            PQntuples(rs_table_action));
        pthread_mutex_unlock(&log_mutex);

        PQclear(rs_table_action);
        return;
    }
    // Extract the data just received to some local vars

    // Note, sql is not saved locally.  It is put into a prepared
    // statement, then discarded.
    char* sql = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "sql"));

    if (sql == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d fail, null sql statement\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        PQclear(rs_table_action);
        return;
    }

    int schema_name_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "schema_name"));
    char* schema_name = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "schema_name"));
    if (schema_name == NULL) schema_name = empty;
    table_entry_size+= schema_name_len+2;

    int table_name_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "table_name"));
    char* table_name = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "table_name"));
    if (table_name == NULL) table_name = empty;
    table_entry_size+= table_name_len+2;

    int target_div_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "target_div"));
    char* target_div = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "target_div"));
    if (target_div == NULL) target_div = empty;
    table_entry_size+= target_div_len+2;

    int handler_name_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "handler_name"));
    char* handler_name = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "handler_name"));
    if (handler_name == NULL) handler_name = empty;
    table_entry_size+= handler_name_len+2;

    int xml_template_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "xml_template"));
    char* xml_template = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "xml_template"));
    if (xml_template == NULL) xml_template = base_xml_template;
    table_entry_size+= xml_template_len+2;

    int prompt_container_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "prompt_container"));
    char* prompt_container = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "prompt_container"));
    if (prompt_container == NULL){
        prompt_container = default_prompt_container;
        prompt_container_len = strlen(default_prompt_container);
    }
    table_entry_size+= prompt_container_len+2;

    int form_set_name_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "form_set_name"));
    char* form_set_name = empty;
    if (form_set_name_len > 0){
        form_set_name = PQgetvalue(rs_table_action, 0,
           PQfnumber(rs_table_action, "form_set_name"));
    }

    int helpful_text_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "helpful_text"));
    char* helpful_text = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "helpful_text"));
    if (helpful_text == NULL) helpful_text = empty;
    table_entry_size+= helpful_text_len+2;

    int inline_js_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "inline_js"));
    char* inline_js = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "inline_js"));
    if (inline_js == NULL) inline_js = empty;
    table_entry_size+= inline_js_len+2;

    int inline_css_len = PQgetlength(rs_table_action, 0,
        PQfnumber(rs_table_action, "inline_css"));
    char* inline_css = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "inline_css"));
    if (inline_css == NULL) inline_css = empty;
    table_entry_size+= inline_css_len+2;

    // Send the sql right back to create a prepared statement
    PGresult* rs_prep;
    rs_prep = PQprepare(hargs->session->conn, prepared_name, sql, 0, NULL);

    if (hargs->conf->log_table_action_details){
        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d rs_prep = %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            PQresStatus( PQresultStatus(rs_prep)));
        pthread_mutex_unlock(&log_mutex);
    }

    if ( !(PQresultStatus(rs_prep) == PGRES_TUPLES_OK)
         &&
         !(PQresultStatus(rs_prep) == PGRES_COMMAND_OK) ){
        // Oh No,
        char error_msg[1024];
        bzero(error_msg, 1024);
        snprintf(error_msg, 1023, "Prepared query failed\n%s %s\n%s",
            form_name, action,
            PQresultErrorMessage(rs_prep));

        error_page(hargs, SC_BAD_REQUEST, error_msg );

        // clean up and return
        PQclear(rs_table_action);
        PQclear(rs_prep);
        return;
    }

    // Now save all that to the open table hash.
    new_table_action = calloc(1, table_entry_size+2);

    void* tmp;
    char* data_target = tmp = new_table_action;
    data_target += sizeof(struct table_action)+2;

    new_table_action->form_name = data_target;
    memcpy(new_table_action->form_name, form_name, form_name_len+1);
    data_target+= form_name_len+2;

    new_table_action->action = data_target;
    memcpy(new_table_action->action, action, action_len+1);
    data_target+= action_len+2;

    new_table_action->prepare_name = data_target;
    memcpy(new_table_action->prepare_name, prepared_name, prepared_name_len+1);
    data_target+= prepared_name_len+2;

    new_table_action->schema_name = data_target;
    memcpy(new_table_action->schema_name, schema_name, schema_name_len+1);
    data_target+= schema_name_len+2;

    new_table_action->table_name = data_target;
    memcpy(new_table_action->table_name, table_name, table_name_len+1);
    data_target+= table_name_len+2;

    new_table_action->target_div = data_target;
    memcpy(new_table_action->target_div, target_div, target_div_len+1);
    data_target+= target_div_len+2;

    new_table_action->handler_name = data_target;
    memcpy(new_table_action->handler_name, handler_name, handler_name_len+1);
    data_target+= handler_name_len+2;

    new_table_action->xml_template = data_target;
    memcpy(new_table_action->xml_template, xml_template, xml_template_len+1);
    data_target += xml_template_len+2;

    new_table_action->prompt_container = data_target;
    memcpy(new_table_action->prompt_container, prompt_container,
        prompt_container_len+1);
    data_target += prompt_container_len+2;

    new_table_action->helpful_text = data_target;
    memcpy(new_table_action->helpful_text, helpful_text,
        helpful_text_len+1);
    data_target += helpful_text_len+2;

    new_table_action->inline_js = data_target;
    memcpy(new_table_action->inline_js, inline_js,
        inline_js_len+1);
    data_target += inline_js_len+2;

    new_table_action->inline_css = data_target;
    memcpy(new_table_action->inline_css, inline_css,
        inline_css_len+1);
    data_target += inline_css_len+2;

    memcpy(new_table_action->form_set_name, form_set_name,
       form_set_name_len+1);

    // these are char arrays and should be freed separately

    char* fieldname_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "fieldnames"));
    char** fields = parse_pg_array(fieldname_pgarray);
    new_table_action->fieldnames = fields;

    char* pkey_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "pkey"));
    char** pkeys = parse_pg_array(pkey_pgarray);
    new_table_action->pkeys = pkeys;

    char* jsfiles_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "js_filenames"));
    char** js_filenames = parse_pg_array(jsfiles_pgarray);
    new_table_action->js_filenames = js_filenames;

    char* cssfiles_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "css_filenames"));
    char** css_filenames = parse_pg_array(cssfiles_pgarray);
    new_table_action->css_filenames = css_filenames;

    char* context_parameters_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "context_parameters"));
    char** context_parameters = parse_pg_array(context_parameters_pgarray);
    new_table_action->context_parameters = context_parameters;

    char* clear_context_parameters_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "clear_context_parameters"));
    char** clear_context_parameters =
        parse_pg_array(clear_context_parameters_pgarray);
    new_table_action->clear_context_parameters  = clear_context_parameters;

    char* callbacks_pgarray = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "callbacks"));
    char** callbacks =
        parse_pg_array(callbacks_pgarray);
    new_table_action->callbacks = callbacks;

    // Count the number of params
    new_table_action->nbr_params = 0;
    if (new_table_action->fieldnames != NULL){

        for(new_table_action->nbr_params=0;
            new_table_action->fieldnames[new_table_action->nbr_params] != NULL;
            new_table_action->nbr_params++){ /* no op */ }
    }

    // Count the number of primary keys
    new_table_action->nbr_pkeys = 0;
    if (new_table_action->pkeys!= NULL){

        for(new_table_action->nbr_pkeys=0;
            new_table_action->pkeys[ new_table_action->nbr_pkeys ] != NULL;
            new_table_action->nbr_pkeys++){ /* no op */ }
    }

    //  etag value
    char* etag = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "etag"));
    if (etag != NULL){
        new_table_action->etag = strtoll(etag, NULL, 10);
    }

    // add_description is a boolean
    char* add_description = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "add_description"));
    if (add_description[0] == 't'){
        new_table_action->add_description = true;
    }else{
        new_table_action->add_description = false;
    }

    // is_callback is a boolean
    char* is_callback = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "is_callback"));
    if (is_callback[0] == 't'){
        new_table_action->is_callback = true;
    }else{
        new_table_action->is_callback = false;
    }

    // callback_attached_action has storage in struct
    char* callback_attached_action = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "callback_attached_action"));
    if (has_data(callback_attached_action)){
        snprintf(new_table_action->callback_attached_action, PG_NAMEDATALEN,
            "%s", callback_attached_action);
    }

    char* callback_response = PQgetvalue(rs_table_action, 0,
        PQfnumber(rs_table_action, "callback_response"));
    if (has_data(callback_response)){
        new_table_action->callback_response =
            callback_name_lookup(callback_response);
    }


    // Set the check token
    new_table_action->integrity_token = hargs->session->integrity_token;

    // Add it to the hash table
    if (xmlHashAddEntry2(
        hargs->session->opentables,
        new_table_action->form_name,
        new_table_action->action,
        new_table_action) != 0){

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log, "%f %d %s:%d %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            "Hash table add entry failed");
        pthread_mutex_unlock(&log_mutex);
    }

    PQclear(rs_table_action);
    PQclear(rs_prep);

    if (hargs->conf->log_table_action_details){

       log_table_action_details(hargs, new_table_action);

        pthread_mutex_lock(&log_mutex);
        fprintf(hargs->log,
            "%f %d %s:%d init_table_entry complete\n",
            gettime(), hargs->request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

    }
    return;
}

/*
 *  Free a table action structure
 *  Should only be called by a close_table function
 *  after the table has been removed from the hash.
 */
void free_table_action(struct table_action* ta){
    if (ta->fieldnames != NULL) free(ta->fieldnames);
    if (ta->pkeys != NULL) free(ta->pkeys);
    if (ta->js_filenames != NULL) free(ta->js_filenames);
    if (ta->css_filenames != NULL) free(ta->css_filenames);
    if (ta->context_parameters != NULL) free(ta->context_parameters);
    if (ta->clear_context_parameters != NULL) free(ta->clear_context_parameters);
    if (ta->callbacks != NULL) free(ta->callbacks);
    free(ta);
}

/*
 *  open_table
 *
 *  Return an action_table struct from the session hash of open tables.
 *
 *  If form_name, action is not in the hash table, then fetch the
 *  definition from postgres and add it to the hash table
 *  creating a prepared statement.
 */

struct table_action*  open_table(struct handler_args* h,
               char* form_name, char* action){

    struct table_action* table_action_ptr;

    table_action_ptr = xmlHashLookup2(h->session->opentables,
        form_name, action);

    // etag check

    if (table_action_ptr != NULL){
        const char* etag_args[3];
        etag_args[0] = form_name;
        etag_args[1] = action;
        etag_args[2] = NULL;

        uint64_t pg_etag = 0;

        PGresult* etag_check_rs = PQexecPrepared(h->session->conn,
            "fetch_table_action_etag", 2, etag_args, NULL, NULL, 0);

        if (PQntuples(etag_check_rs) == 1){
            char* ta_etag = PQgetvalue(etag_check_rs, 0,
                PQfnumber(etag_check_rs, "etag"));

            if (ta_etag != NULL){
                pg_etag = strtoll(ta_etag, NULL, 10);

                // pg_etag == 0 is the interesting case that the cache
                // is always discarded.
                if ((pg_etag != 0) && (pg_etag == table_action_ptr->etag)){
                     // OK - It's the current version
                     pthread_mutex_lock(&log_mutex);
                     fprintf(h->log, "%f %d %s:%d table action %s %s cache OK\n",
                        gettime(), h->request_id, __func__, __LINE__,
                        form_name, action);
                     pthread_mutex_unlock(&log_mutex);

                     if (table_action_ptr->integrity_token !=
                         h->session->integrity_token){

                         // This condition is double plus ungood.
                         pthread_mutex_lock(&log_mutex);
                         fprintf(h->log,
                             "%f %d %s:%d table action failed integrity token"
                             " check.\n",
                             gettime(), h->request_id, __func__, __LINE__);
                         pthread_mutex_unlock(&log_mutex);

                         close_table(h, form_name, action);
                         // Going down hard.  The session data is corrupted.
                         logout(h);

                         error_page(h, SC_INTERNAL_SERVER_ERROR, "Internal Error");
                         PQclear(etag_check_rs);
                         return NULL;

                     }else{
                         PQclear(etag_check_rs);
                         return table_action_ptr;
                     }
                }else{
                     // No good, the thing has been replaced
                     // Clear out the old and fetch the new
                    pthread_mutex_lock(&log_mutex);
                    fprintf(h->log,
                        "%f %d %s:%d table action %s %s cache invalid\n",
                        gettime(), h->request_id, __func__, __LINE__,
                        form_name, action);
                    pthread_mutex_unlock(&log_mutex);

                    close_table(h, form_name, action);
                }
            }
        }
        PQclear(etag_check_rs);
    }

    // OK, create it.
    init_table_entry(h, form_name, action);

    // Now try again
    table_action_ptr = xmlHashLookup2( h->session->opentables,
        form_name, action);

    if (table_action_ptr != NULL) return table_action_ptr;

    // Well then fail
    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d open_table (%s, %s) not found\n",
        gettime(), h->request_id, __func__, __LINE__,
        form_name, action);
    pthread_mutex_unlock(&log_mutex);

    return NULL;
}

/*
 *  build_pgarray_from_post
 *
 *  If the given element is in postdata as an array (base[n]),
 *  then assemble and return the pgarray.
 */

char* build_pgarray_from_post(xmlHashTablePtr postdata, char* element){

    char* ar_element;
    char* ar_value;
    char* ar;

    // Determine the length required.
    bool found = true;
    int ar_length = 0;
    int n;

    for(n=0; found; n++){
        asprintf(&ar_element, "%s[%d]", element, n);
        ar_value = xmlHashLookup(postdata, ar_element);
        free(ar_element);
        ar_element = NULL;

        if (ar_value != NULL){
            ar_length += strlen(ar_value) + 1; // 1 for ,
        }else{
            found = false;
        }
    }
    // Nothing found, empty set
    if (ar_length == 0){
        ar = calloc(1, sizeof(char));
        ar[0] = '\0';
        return ar;
    }

    ar_length += 4; // for { } \0 and one for good luck
    ar = calloc(1, ar_length);
    char* ch = ar;
    *ch = '{';
    ch++;

    found = true;
    bool first = true;

    for(n=0; found; n++){
        asprintf(&ar_element, "%s[%d]", element, n);
        ar_value = xmlHashLookup(postdata, ar_element);
        free(ar_element);
        ar_element = NULL;

        if ((ar_value != NULL) && (ar_value[0] != '\0')){
            if (first){
                first = false;
            }else{
                *ch = ',';
                ch++;
            }
            memcpy(ch, ar_value, strlen(ar_value));
            ch += strlen(ar_value);
        }else{
            found = false;
        }
    }
    *ch = '}';
    ch++;

    return ar;
}

/*
 *  perform_post_row_action
 *
 *  When the post data represents a table where the element is in
 *  the form column_name[row] then perform the table action on the
 *  given row.
 */
PGresult* perform_post_row_action(struct handler_args* h,
    struct table_action* ta, int row){

    PGresult* rs;

    if (ta==NULL) return NULL;

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d %s %s row=%d nbr_params=%d\n",
        gettime(), h->request_id, __func__, __LINE__,
        "perform_post_row_action started",
        ta->prepare_name, row, ta->nbr_params);
    pthread_mutex_unlock(&log_mutex);

    if ((ta->nbr_params > 0) && (h->postdata == NULL)){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            "perform_post_row_action called with null data");
        pthread_mutex_unlock(&log_mutex);

        return NULL;
    }

    char** paramdata = calloc((2 + ta->nbr_params), sizeof(char*));

    int k;
    char* fieldname;
    for(k=0; k<ta->nbr_params; k++){

        // convert the name to name[row]
        asprintf(&fieldname, "%s[%d]", ta->fieldnames[k], row);
        paramdata[k] = xmlHashLookup(h->postdata, fieldname);

        if (paramdata[k] == NULL){  // didn't work
            // Try it without the row [n] part.
            // This allows a constant to be added to the form
            // that will be used for all the rows.

            paramdata[k] = xmlHashLookup(h->postdata, ta->fieldnames[k]);
        }

        if (paramdata[k] == NULL){  //still? then error out
            free(paramdata);

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d fail param %s not found\n",
                gettime(), h->request_id, __func__, __LINE__,
                fieldname);
            pthread_mutex_unlock(&log_mutex);

            free(fieldname);
            error_page(h, SC_BAD_REQUEST, "missing parameter");
            return NULL;
        }

        free(fieldname);
    }

    paramdata[ ta->nbr_params ] = NULL;

    rs = PQexecPrepared(h->session->conn, ta->prepare_name, ta->nbr_params,
        (const char * const *) paramdata, NULL, NULL, 0);

    if (h->conf->log_table_action_details){
        int j;
        for(j = 0; j < ta->nbr_params; j++){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d arg=%d key=%s val length=%lu\n",
                gettime(), h->request_id, __func__, __LINE__,
                j, ta->fieldnames[j], strlen(paramdata[j]));
            pthread_mutex_unlock(&log_mutex);
        }
    }
    char* error_msg = nlfree_error_msg(rs);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d perform_post_row_action completed %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)), error_msg);
    pthread_mutex_unlock(&log_mutex);

    free(error_msg);
    free(paramdata);

    return rs;
}

/*
 *  perform_post_action
 *
 *  Execute the given table action using the post data
 *  as the source for the parameters.
 */

PGresult* perform_post_action(struct handler_args* h, struct table_action* ta){

    PGresult* rs;
    char* element;
    int k;

    if (ta==NULL) return NULL;

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d %s %s nbr_params=%d\n",
        gettime(), h->request_id, __func__, __LINE__,
        "perform_post_action started",
        ta->prepare_name, ta->nbr_params);
    pthread_mutex_unlock(&log_mutex);

    if ((ta->nbr_params > 0) && (h->postdata == NULL)){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d fail %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            "perform_post_action called with null data");
        pthread_mutex_unlock(&log_mutex);

        return NULL;
    }

    char** paramdata = calloc((2 + ta->nbr_params), sizeof(char*));
    bool free_array[2+ta->nbr_params];

    for(k=0; k < ta->nbr_params; k++){
        element = xmlHashLookup(h->postdata, ta->fieldnames[k]);
        free_array[k] = false;  // unless changed later

        if (element == NULL){
            // This can happen if the form does not send data that
            // matches the stored procedure, but it can also happen
            // if the field is an array and the elements need to be
            // joined together. ( ar = {ar[0],ar[1],...} )
            // If it is an array, the prompt_rule->prompt_type is
            // text_array.
            struct prompt_rule* this_rule;
            this_rule = fetch_prompt_rule(h, ta->form_name,
                 ta->fieldnames[k]);

            if ( (this_rule != NULL) &&
                  (this_rule->prompt_type != NULL) &&
                  (strncmp(this_rule->prompt_type, "text_array",
                      strlen("text_array")) == 0)){

                 element = build_pgarray_from_post(h->postdata,
                     ta->fieldnames[k]);

                 if (element != NULL){
                     free_array[k] = true;
                     pthread_mutex_lock(&log_mutex);
                     fprintf(h->log, "%f %d %s:%d pgarray %s\n",
                         gettime(), h->request_id, __func__, __LINE__,
                         element);
                     pthread_mutex_unlock(&log_mutex);
                  }
            }
            free_prompt_rule(h, this_rule);

        }

        if (element == NULL){ // still
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d %s %s\n",
                 gettime(), h->request_id, __func__, __LINE__,
                 "perform_post_action failed to find field",
                 ta->fieldnames[k]);
            pthread_mutex_unlock(&log_mutex);

            free(paramdata);
            return NULL;
        }
        if (strlen(element) == 0){
            if (free_array[k]) free(element); // An empty string
            paramdata[k] = NULL;

            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d warning param %d %s is null\n",
                gettime(), h->request_id, __func__, __LINE__,
                k, ta->fieldnames[k]);
            pthread_mutex_unlock(&log_mutex);

        }else{
            paramdata[k] = element;
        }
    }
    paramdata[ ta->nbr_params ] = NULL;

    rs = PQexecPrepared(h->session->conn, ta->prepare_name, ta->nbr_params,
        (const char * const *) paramdata, NULL, NULL, 0);

    if (h->conf->log_table_action_details){
        int j;
        for(j=0; j<ta->nbr_params; j++){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d arg=%d key=%s val length=%lu\n",
                gettime(), h->request_id, __func__, __LINE__,
                j, ta->fieldnames[j],
                (paramdata[k] == NULL) ? 0:strlen(paramdata[j]));
            pthread_mutex_unlock(&log_mutex);
        }
    }
    for(k=0; k < ta->nbr_params; k++){
        if (free_array[k]) free(paramdata[k]);
    }
    free(paramdata);

    char* error_msg = nlfree_error_msg(rs);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d perform_post_action completed %s %s %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)), error_msg,
        PQcmdTuples(rs), "rows affected");
    pthread_mutex_unlock(&log_mutex);

    free(error_msg);

    return rs;
}

/*
 *  perform_action
 *
 *  Execute the given table action using the data given
 *  as the prepared statement parameters.
 */

PGresult* perform_action(struct handler_args* h, struct table_action* ta,
    char** data){

    if (ta == NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            "perform_action called with null table_action");
        pthread_mutex_unlock(&log_mutex);

        return NULL;
    }

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        "perform_action started", ta->prepare_name);
    pthread_mutex_unlock(&log_mutex);

    fflush(h->log);
    PGresult* rs;
    rs = PQexecPrepared(h->session->conn, ta->prepare_name, ta->nbr_params,
        (const char * const *) data, NULL, NULL, 0);

    if (h->conf->log_table_action_details){
        int j;
        for(j=0; j<ta->nbr_params; j++){
            pthread_mutex_lock(&log_mutex);
            fprintf(h->log, "%f %d %s:%d arg=%d key=%s val length=%lu\n",
                gettime(), h->request_id, __func__, __LINE__,
                j, ta->fieldnames[j], strlen(data[j]));
            pthread_mutex_unlock(&log_mutex);
        }
    }
    char* error_msg = nlfree_error_msg(rs);

    pthread_mutex_lock(&log_mutex);
    fprintf(h->log, "%f %d %s:%d perform_action completed %s %s\n",
        gettime(), h->request_id, __func__, __LINE__,
        PQresStatus(PQresultStatus(rs)), PQresultErrorMessage(rs));
    pthread_mutex_unlock(&log_mutex);

    free(error_msg);
    return rs;
}

/*
 * close_table
 *
 * Reclaim resources from an open table.
 */

void close_table(struct handler_args* h, char* form_name, char* action){

    struct table_action* table_action_ptr;

    if (h==NULL) return;
    if (h->session==NULL)return;
    if (h->session->conn==NULL)return;
    if (h->session->opentables==NULL)return;

    table_action_ptr = xmlHashLookup2(h->session->opentables, form_name,
        action);

    if (table_action_ptr==NULL){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d  close_table failed on %s, %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            form_name, action);
        pthread_mutex_unlock(&log_mutex);

        return;
    }
    if (strstr(table_action_ptr->prepare_name, "\"") != NULL){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d  close_table fail - "
            "double quote in name %s, %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            form_name, action);
        pthread_mutex_unlock(&log_mutex);

        return;
    }
    // deallocate the prepared statement
    PGresult* rs;

    // I would much rather do this as a prepared statement, but pg
    // would not allow that.  It is relatively safe because prepare_name
    // is created in init_table_entry and is not user influenced, and also
    // had to be a valid form_name and action

    char* cmd_buf;
    asprintf(&cmd_buf, "DEALLOCATE \"%s\";", table_action_ptr->prepare_name);

    rs = PQexec(h->session->conn, cmd_buf);

    if (PQresultStatus(rs) != PGRES_COMMAND_OK){
        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d  %s failed %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            cmd_buf,
            PQresStatus(PQresultStatus(rs)));
        pthread_mutex_unlock(&log_mutex);
    }
    free(cmd_buf);
    PQclear(rs);

    // remove the hash table entry
    if (xmlHashRemoveEntry2(h->session->opentables, form_name, action,
        NULL) != 0){

        pthread_mutex_lock(&log_mutex);
        fprintf(h->log, "%f %d %s:%d  xmlHashRemoveEntry2 %s, %s failed\n",
            gettime(), h->request_id, __func__, __LINE__,
            form_name, action);
        pthread_mutex_unlock(&log_mutex);
    }

    // free the node
    free_table_action(table_action_ptr);

    return;
}

/*
 * close_all_tables
 *
 * Deallocate all the prepared statements and free all the table entries.
 */

void table_close_scanner(void* payload, void* data, xmlChar* form_name,
    xmlChar* action, xmlChar* not_used){

    struct handler_args* h = data;

    close_table(h, form_name, action);
}

void close_all_tables(struct handler_args* h, struct session* this_session){

    if (this_session == NULL) return;

    // housekeeper needs to assume the session identity for close_table.
    // For the normal case, this assignment is a noop.

    struct session* saved_session = h->session;
    h->session = this_session;

    if (this_session->opentables!=NULL){

        xmlHashScanFull(this_session->opentables,(void*)table_close_scanner,h);
        xmlHashFree(this_session->opentables, NULL);
        this_session->opentables= NULL;
    }
    h->session = saved_session;
    return;
}

bool table_action_exists(struct handler_args* h,
    char* form_name, char* action){

    char* paramValues[3];
    paramValues[0] = form_name;
    paramValues[1] = action;
    paramValues[2] = NULL;

    PGresult* rs;

    rs = PQexecPrepared(h->session->conn, "table_action_exists", 2,
        (const char * const *) &paramValues, NULL, NULL, 0);

    bool it_exists = false;
    if ((rs != NULL) &&
       (PQresultStatus(rs) == PGRES_TUPLES_OK) &&
       (PQntuples(rs) == 1)){

           it_exists = true;
    }
    PQclear(rs);

    return it_exists;
}

#ifdef OPENTABLE_TEST

pthread_mutex_t log_mutex;

void print_table_action(struct handler_args* h, char* name,
    struct table_action* ta){

    pthread_mutex_init(&log_mutex,NULL);

    if (h == NULL){
        fprintf(stderr, "print_table_action called with null handler\n");
        return;
    }
    if (ta == NULL){
        fprintf(stderr, "print_table_action called with null table action\n");
        return;
    }
    fprintf(h->log, "+---------------------------\n");
    fprintf(h->log, "| print_table_action %s:\n", name);
    fprintf(h->log, "| form_name=%s\n", ta->form_name);
    fprintf(h->log, "| action=%s\n", ta->action);
    fprintf(h->log, "| table_name=%s\n", ta->table_name);
    fprintf(h->log, "| prepare_name=%s\n", ta->prepare_name);
    fprintf(h->log, "| nbr_params=%d\n", ta->nbr_params);
    int k;
    for(k=0; k< ta->nbr_params; k++){
        fprintf(h->log, "| field[%d]=%s\n", k, ta->fieldnames[k]);
    }
    for (k=0; k< ta->nbr_pkeys; k++){
        fprintf(h->log, "| pkey[%d]=%s\n", k, ta->pkeys[k]);
    }
    fprintf(h->log, "+---------------------------\n");
}

int main(int argc, char* argv[]){

    printf("%s\n", argv[0]);

    printf("setup a fake environment\n");
    qzrandom_init();
    struct qz_config* conf = init_config();


    struct handler_args hargs;
    struct handler_args* h = &hargs;
    hargs.log = stdout;
    hargs.conf = conf;

    init_handler_hash();
    init_prompt_type_hash();

    struct session s;
    hargs.session = &s;

    const char* kw[] = { "host", "dbname", "user", "password",
        "application_name", NULL };

    char* vals[] = { "localhost", "info", "qz", "42", "qztest", NULL };

    h->session->conn = PQconnectdbParams(kw, vals, 0);

    if (PQstatus(h->session->conn) != CONNECTION_OK){
       fprintf(h->log, "bad connect\n");
       exit(19);
    }
    h->session->opentables = xmlHashCreate(50);
    h->session->pgtype_datum = xmlHashCreate(50);
    h->session->form_tags = xmlHashCreate(50);
    h->request_id = 42;

    init_open_table(h);

    printf("\ntesting wint insert\n\n");

    struct table_action* wint_insert;

    wint_insert = open_table(h, "wint", "insert");
    print_table_action(h, "wint insert", wint_insert);

    int k;
    PGresult* rs;

    char* upd_data[3];
    char nbuf[32];

    upd_data[0] = nbuf;
    upd_data[1] = NULL;


    for(k=0; k<9; k++){
        snprintf(nbuf, 30, "%d", k);
        rs = perform_action(h, wint_insert, upd_data);
        printf("rs = %d, %s\n", PQresultStatus(rs),
            PQresStatus( PQresultStatus(rs) ));

        printf("ntuples=%d\nnfields=%d\n",PQntuples(rs),PQnfields(rs));
    }
    PQclear(rs);

    printf("\ntesting wint list\n\n");

    struct table_action* wint_list = open_table(h, "wint", "list");
    print_table_action(h, "wint list", wint_list);
    rs = perform_action(h, wint_list, NULL);
    int row, col;

    for(row=0; row<PQntuples(rs); row++){
        for(col=0; col<PQnfields(rs); col++){
            printf("%s ", PQgetvalue(rs,row,col));
        }
        printf("\n");
    }
    PQclear(rs);

    printf("\ntesting wint delete\n\n");

    struct table_action* wint_delete = open_table(h, "wint", "delete");
    print_table_action(h, "wint delete", wint_delete);

    for(k=0; k<9; k++){
        snprintf(nbuf, 30, "%d", k);
        rs = perform_action(h, wint_delete, upd_data);
        printf("rs = %d, %s\n", PQresultStatus(rs),
            PQresStatus( PQresultStatus(rs) ));

        printf("ntuples=%d\nnfields=%d\n",PQntuples(rs),PQnfields(rs));
    }
    PQclear(rs);

    printf("\nclose table\n\n");
    close_table(h, "wint", "insert");
    close_table(h, "wint", "list");
    close_table(h, "wint", "delete");

    printf("\ntesting fs get\n\n");

    struct table_action* ta = open_table(h, "fs", "get");
    char* ta_arg[2];
    ta_arg[0] = "qzforms.js";
    ta_arg[1] = NULL;

    rs = perform_action(h, ta, ta_arg);
    printf("ntuples=%d\nnfields=%d\n",PQntuples(rs),PQnfields(rs));


    for(row=0; row<PQntuples(rs); row++){
        for(col=0; col<PQnfields(rs); col++){
            printf("%s:\n%s\n\n", PQfname(rs,col),  PQgetvalue(rs,row,col));
        }
        printf("\n");
    }

    PQclear(rs);

    PQfinish(h->session->conn);
    return 0;
}

#endif
