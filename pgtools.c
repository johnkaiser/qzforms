
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
 *  get_value
 *
 *  Wrap PQgetvalue and PQfnumber so they always return 
 *  safe data, even when asked to do stupid things.
 *  PGresult* rs must be valid, row_nbr and col_name may not.
 */
char* get_value(const PGresult* rs, int row_nbr, char* col_name){
    int fnbr;
    static char* empty = "\0\0";

    if (col_name == NULL) return empty;
    if (col_name[0] == '\0') return empty;
    if (row_nbr < 0) return empty;

    if ( ((fnbr = PQfnumber(rs, col_name)) >= 0) && 
        (PQgetlength(rs, row_nbr, fnbr) > 0) ){
        char* rval = PQgetvalue(rs, row_nbr, fnbr);
        return (rval == NULL) ? empty : rval;
    }
    return empty;
}

/* has_data - because I have to check this a bunch of times */
bool has_data(char* astr){
    if (astr == NULL) return false;
    if (astr[0] == '\0') return false;
    return true;
}

/*
 *  get_bool
 *
 *  Like get_value above, but return a boolean
 */

bool get_bool(const PGresult* rs, int row_nbr, char* col_name){
    char* answer = get_value(rs, row_nbr, col_name);
    if (answer == NULL) return false;
    if (answer[0] == 't') return true;
    return false;
}

/*
 *  nlfree_error_msg
 *
 *  Make a copy of PQresultErrorMessage with newlines 
 *  and carriage returns replaced with spaces,
 *  because the newlines mess up the log file.
 *
 *  The result must be free'd.
 */

char* nlfree_error_msg(PGresult* some_rs){

    char* pq_error;

    pq_error = PQresultErrorMessage(some_rs);

    int error_msg_len = 0;
    if (pq_error != NULL) { 
        error_msg_len = strlen(pq_error);
    }

    char* error_msg = calloc(1,error_msg_len + 2);

    char* from;
    char* to;
    to = error_msg;
    if (error_msg_len > 0){
        for(from = pq_error; *from != '\0'; from++){
            if ((*from == '\n') || (*from == '\r')){
                *to = ' ';
            }else{    
                *to = *from;
            }                
            to++;
        }
    }
    return error_msg;
}
 
/*
 *   create_pgtype_datum
 *
 *   Turn a select result from the information schema
 *   and the system catalogs into a struct.
 *
 *   The result pointer is stored as well.  Strings
 *   reference data in the result, call PQclear on ->pgresult
 *   before free'ing the returned pointer.
 */
struct pgtype_datum* create_pgtype_datum(PGresult* rs, int row){

    if (rs == NULL) return NULL;
    int col;
    col = PQfnumber(rs, "version"); 

    int pgtype_datum_version = atoi(PQgetvalue(rs,row,col)); 
    if (pgtype_datum_version != PGTYPE_DATUM_VERSION){
        fprintf(stderr, 
            "%f %s:%d fail pgtype_version error. SQL version %d, c version %d\n",
            gettime(),__func__, __LINE__, 
            pgtype_datum_version, PGTYPE_DATUM_VERSION);

        return NULL;
    }

    struct pgtype_datum* pgt = calloc(1, sizeof(struct pgtype_datum));

    pgt->version = pgtype_datum_version;
    pgt->table_schema = get_value(rs, 0, "table_schema");
    pgt->table_name = get_value(rs, 0, "table_name");
    pgt->column_name = get_value(rs, 0, "column_name");
    pgt->ordinal_position = strtol( get_value(rs, 0, "ordinal_position"), NULL, 10);
    pgt->column_default = get_value(rs, 0, "column_default");
    
    pgt->is_nullable = get_bool(rs, 0, "is_nullable");

    pgt->typname = get_value(rs, 0, "typname");
    pgt->typtype = get_value(rs, 0, "typtype");
    pgt->typdelim = get_value(rs, 0, "typdelim");

    pgt->is_base_type = get_bool(rs, 0, "is_base_type");
    pgt->is_boolean = get_bool(rs, 0, "is_boolean"); 
    pgt->is_composite = get_bool(rs, 0, "is_composite");
    pgt->is_domain = get_bool(rs, 0, "is_domain");
    pgt->is_enum = get_bool(rs, 0, "is_enum");
    pgt->is_pseudo_type = get_bool(rs, 0, "is_pseudo_type");

    pgt->character_maximum_length = 
        strtol(get_value(rs, 0, "character_maximum_length"), NULL, 10);

    pgt->character_octet_length =
        strtol(get_value(rs, 0, "character_octet_length"), NULL,10);

    pgt->numeric_precision =
        strtol(get_value(rs, 0, "numeric_precision"), NULL, 10);

    pgt->numeric_precision_radix =
        strtol(get_value(rs, 0, "numeric_precision_radix"), NULL, 10);

    pgt->numeric_scale =
        strtol(get_value(rs, 0, "numeric_scale"), NULL, 10);

    pgt->datetime_precision =
        strtol(get_value(rs, 0, "datetime_precision"), NULL, 10);

    pgt->typcategory_name = get_value(rs, 0, "typcategory_name");
    pgt->domain_schema = get_value(rs, 0, "domain_schema");
    pgt->domain_name = get_value(rs, 0, "domain_name");
    pgt->domain_check_clause = get_value(rs, 0, "domain_check_clause");
    pgt->udt_schema = get_value(rs, 0, "udt_schema");
    pgt->udt_name = get_value(rs, 0, "udt_name");
    pgt->description = get_value(rs, 0, "description");
    pgt->is_updatable = get_bool(rs, 0, "is_updatable");

    pgt->enum_labels = get_value(rs, 0, "enum_labels");
    pgt->composite_attributes = get_value(rs, 0, "composite_attributes");

    pgt->fkey_schema = get_value(rs, 0, "fkey_schema");
    pgt->fkey_table = get_value(rs, 0, "fkey_table");
    pgt->fkey_attribute = get_value(rs, 0, "fkey_attribute");
    pgt->has_fkey = ((pgt->fkey_attribute != NULL) && (strlen(pgt->fkey_attribute) > 0));

    pgt->pgresult = rs; 

    return pgt;
}

/*
 *  get_pgtype_datum
 * 
 *  Return a pgtype_datum struct, either from a hash table or from Postgresql.
 *  In general, the result should not be freed.  
 *  It is in the hash table and will be retrieved from there.
 *  If it is freed, it also must be removed from the hash table.
 */ 
struct pgtype_datum* get_pgtype_datum(
    struct handler_args* h,
    char* table_name, 
    char* column_name){

    double start = gettime();

    xmlHashTablePtr datums = h->session->pgtype_datum; 

    struct pgtype_datum* datum;

    datum = xmlHashLookup2(datums, table_name, column_name);

    if (datum != NULL){
        fprintf(h->log, 
            "%f %d %s:%d returning datum from hash (%s,%s,%s) in %f\n", 
            gettime(), h->request_id, __func__, __LINE__,
            datum->table_schema, datum->table_name, datum->column_name,
            gettime() - start); 
        return datum;
    }    

    const char* datum_args[4];
    datum_args[0] = table_name; 
    datum_args[1] = column_name; 
    datum_args[2] = NULL;

    PGresult* datum_rs = PQexecPrepared(h->session->conn,
        "fetch_datum", 2, datum_args, NULL, NULL, 0);

    if (PQntuples(datum_rs) != 1){
       
        // Before logging the error message, remove new lines
        // because they mess up the log file.
        char* error_msg = nlfree_error_msg(datum_rs);

        fprintf(h->log, "%f %d %s:%d fail (%s,%s) expected 1 row, got %d "
                "error=%s\n",
                gettime(), h->request_id, __func__, __LINE__,
                table_name, column_name,
                PQntuples(datum_rs), error_msg);
        
        free(error_msg);

        PQclear(datum_rs);
        return NULL;
    }

    int row = 0;

    datum = create_pgtype_datum(datum_rs, row);

    fprintf(h->log, "%f %d %s:%d create_pgtype_datum (%s,%s,%s) "
        "completed in %f\n", 
        gettime(), h->request_id, __func__, __LINE__,
        datum->table_schema, datum->table_name, datum->column_name,
        gettime() - start); 

    // ZZZZZZZZZZZZZZzz
    // This stopped working because reasons
    //if (xmlHashAddEntry2(datums, datum->table_name, 
    //    datum->column_name, datum) !=0){
    // Is it a problem if the names do not exist while the record
    // still does?  Suppose the names provided by the calling function
    // go out of scope.  The same names again will work ok, but
    // what about housekeeper?
    if (xmlHashAddEntry2(datums, table_name, column_name, datum) !=0){
        
        fprintf(h->log, "%f %d %s:%d xmlHashAddEntry2 failed (%s,%s)\n", 
            gettime(), h->request_id, __func__, __LINE__,
            table_name, column_name); 

        return NULL;
    }
    return datum;
}


/*
 *  pgtype_datum_to_json
 *
 *  Turn a pgtype_datum struct into a json object.
 *  The result must be freed.
 */ 
char* pgtype_datum_to_json(struct pgtype_datum* datum){

    char* fmt = 
        "{ \n"
        "  \"version\": \"%d\",\n"
        "  \"table_schema\": \"%s\",\n"
        "  \"table_name\": \"%s\",\n"
        "  \"column_name\": \"%s\",\n"
        "  \"ordinal_position\": \"%d\",\n"
        "  \"column_default\": \"%s\",\n"
        "  \"is_nullable\": \"%s\",\n"
        "  \"typname\": \"%s\",\n"
        "  \"typtype\": \"%s\",\n"
        "  \"typdelim\": \"%s\",\n"
        "  \"is_base_type\": \"%s\",\n"
        "  \"is_boolean\": \"%s\", \n"
        "  \"is_composite\": \"%s\",\n"
        "  \"is_domain\": \"%s\",\n"
        "  \"is_enum\": \"%s\",\n"
        "  \"is_pseudo_type\": \"%s\",\n"
        "  \"character_maximum_length\": \"%d\",\n"
        "  \"character_octet_length\": \"%d\",\n"
        "  \"numeric_precision\": \"%d\",\n"
        "  \"numeric_precision_radix\": \"%d\",\n"
        "  \"numeric_scale\": \"%d\",\n"
        "  \"datetime_precision\": \"%d\",\n"
        "  \"typcategory_name\": \"%s\",\n"
        "  \"domain_schema\": \"%s\",\n"
        "  \"domain_name\": \"%s\",\n"
        "  \"domain_check_clause\": \"%s\",\n"
        "  \"udt_schema\": \"%s\",\n"
        "  \"udt_name\": \"%s\",\n"
        "  \"description\": \"%s\",\n"
        "  \"is_updatable\": \"%s\",\n"
        "  \"enum_labels\": %s,\n"
        "  \"composite_attributes\": %s,\n"
        "  \"has_fkey\": \"%s\",\n"
        "  \"fkey_schema\": \"%s\",\n"
        "  \"fkey_table\": \"%s\",\n"
        "  \"fkey_attribute\": \"%s\",\n"
        "}\n";

    char* enum_json_ar = "[]";

    if (strlen(datum->enum_labels) > 0){
       enum_json_ar = pg_ar_to_json(datum->enum_labels);
    }

    char* composite_attributes_json_ar = "[]";

    if (strlen(datum->composite_attributes) > 0){
        composite_attributes_json_ar = pg_ar_to_json(datum->composite_attributes);
    }

    char* json_str;

    asprintf(&json_str, fmt,
        datum->version,
        datum->table_schema,
        datum->table_name,
        datum->column_name,
        datum->ordinal_position,
        datum->column_default,
        (datum->is_nullable) ? "true":"false",
        datum->typname,
        datum->typtype,
        datum->typdelim,
        (datum->is_base_type) ? "true":"false",
        (datum->is_boolean) ? "true":"false",
        (datum->is_composite) ? "true":"false",
        (datum->is_domain) ? "true":"false",
        (datum->is_enum) ? "true":"false",
        (datum->is_pseudo_type) ? "true":"false",
        datum->character_maximum_length,
        datum->character_octet_length,
        datum->numeric_precision,
        datum->numeric_precision_radix,
        datum->numeric_scale,
        datum->datetime_precision,
        datum->typcategory_name,
        datum->domain_schema,
        datum->domain_name,
        datum->domain_check_clause,
        datum->udt_schema,
        datum->udt_name,
        datum->description,
        (datum->is_updatable) ? "true":"false",
        enum_json_ar,
        composite_attributes_json_ar,
        (datum->has_fkey) ? "true":"false",
        datum->fkey_schema,
        datum->fkey_table,
        datum->fkey_attribute);

    if (strlen(datum->enum_labels) > 0) free(enum_json_ar);
    if (strlen(datum->composite_attributes) > 0) free(composite_attributes_json_ar);

    return json_str;
}

void testdatum(struct handler_args* h){

    struct pgtype_datum* datum;

    char* cols[] = {"n","words","trouble","ar","pos","current_mood",
        "addr","mask","nbr","fixed", NULL};

    int j;

    content_type(h, "text/plain");
    struct strbuf* heading = new_strbuf("pgtype_datum\n",0);
    h->data = heading;

    for(j=0; cols[j]!=NULL; j++){
        datum = get_pgtype_datum(h, "stuff", cols[j]);
        if (datum != NULL){
            char* json_str = pgtype_datum_to_json(datum);
            struct strbuf* sb = new_strbuf(json_str, 0) ;
            strbuf_append(heading, sb);

            char* escaped_pgtype = xmlURIEscapeStr(json_str, NULL);
            if (escaped_pgtype != NULL){
                struct strbuf* pgt = new_strbuf(escaped_pgtype, 0);
                strbuf_append(heading, pgt);
                free(escaped_pgtype);
            }    

            free(json_str);
        }else{
            error_page(h, 417, "datum is null");
        }    
    }
    char allchars[256];
    int k;
    allchars[0] = '\n';
    for(k=1; k<255; k++){
        allchars[k] = k;
        allchars[k+1] = '\0';
    }
    char* esc_allchars = xmlURIEscapeStr(allchars, "\n" );
    struct strbuf* allch = new_strbuf(esc_allchars, 0);
    strbuf_append(h->data, allch);
    free(esc_allchars);


    return;
}


/*
 *  datum_hash_scanner
 *
 *  Remove one entry from the pgtype_datum hash table.
 */
void datum_hash_scanner(void * payload, void* hash_table, xmlChar * form_name,
    xmlChar* column_name, xmlChar* notused){

    xmlHashRemoveEntry2(hash_table, form_name, column_name, NULL);

    struct pgtype_datum* pgt = payload;
    if (pgt->pgresult!= NULL) PQclear(pgt->pgresult);

    free(payload);
}

/* 
 *  close_all_pgtype_datums
 *
 *  Remove the contents of the pgtype_datum hash table 
 *  leaving the empty table in place.
 *  Called when pg sends a notify pg_db_change.
 */
void close_all_pgtype_datums(struct session* this_session){

    xmlHashScanFull(this_session->pgtype_datum, (void*)datum_hash_scanner,
        this_session->pgtype_datum);

}

/*
 *  foreign_key_list
 *
 *  For a pg_type datum, ruturn the foreign key values as a
 *  null terminated string array to be freed by the calling function.
 */
char ** foreign_key_list(struct handler_args* h, struct pgtype_datum* pg_type){

    if (pg_type == NULL){
        fprintf(h->log, "%f %d %s:%d fail %s",
            gettime(), h->request_id, __func__, __LINE__,
            "pg_type is null");

        return NULL;    
    }

    if(!pg_type->has_fkey){
        fprintf(h->log, "%f %d %s:%d fail on %s %s\n",
            gettime(), h->request_id, __func__, __LINE__,
            pg_type->column_name,
            "pg_type record has no foreign key");

        return NULL;    
    }

    // This will not work as a prepared statement because
    // the FROM is unknown here but required for a
    // prepared statement.
    // It also will not work as an SQL function because
    // the from table is not known. 

    // But first, make sure none of search terms contains a 
    // double quote, escaped or not.
    bool dbl_quote = false;
    char *ch;
    for (ch=pg_type->fkey_schema; *ch != '\0'; ch++){
        if (*ch == '"') dbl_quote = true;
    }    
    for (ch=pg_type->fkey_table; *ch != '\0'; ch++){
        if (*ch == '"') dbl_quote = true;
    }
    for (ch=pg_type->fkey_attribute; *ch != '\0'; ch++){
        if (*ch == '"') dbl_quote = true;
    }
    if (dbl_quote) return NULL;

    char* fetch_list = NULL;
    asprintf(&fetch_list, "SELECT \"%s\" FROM \"%s\".\"%s\" ORDER BY \"%s\"", 
        pg_type->fkey_attribute, pg_type->fkey_schema, 
        pg_type->fkey_table, pg_type->fkey_attribute);
        
    PGresult* rs;
    rs = PQexec(h->session->conn, fetch_list);

    if (PQresultStatus(rs) !=  PGRES_TUPLES_OK){
        // error message needs to be stripped of \n
        char* error_msg = nlfree_error_msg(rs);

        fprintf(h->log, "%f %d %s:%d fail on %s %s",
            gettime(), h->request_id, __func__, __LINE__,
            fetch_list, error_msg);

        free(error_msg);
        free(fetch_list);
        fetch_list = NULL;
        PQclear(rs);
        return NULL;
    }
    fprintf(h->log, "%f %d %s:%d select_fkey %s returned %d items\n",
        gettime(), h->request_id, __func__, __LINE__,
        fetch_list, PQntuples(rs));

    // Need the size of the data.
    int fkey_size = 0;
    int row;
    for(row=0; row<PQntuples(rs); row++){
        fkey_size += PQgetlength(rs, row, 0)+1; // +1 for ending \0
    }
    fkey_size += sizeof(char*) * (PQntuples(rs)+1); //array pointers

    char** options = calloc(1, fkey_size);

    // string data starts past the end of the options array
    char* val = (void*) &options[PQntuples(rs)+1];
    for(row=0; row<PQntuples(rs); row++){

        options[row] = val;
        int len = PQgetlength(rs, row, 0)+1; // inc ending \0
        memcpy(val, PQgetvalue(rs, row, 0), len); 
        val += len;
    }
    options[PQntuples(rs)] = NULL;

    free(fetch_list);
    fetch_list = NULL;
    PQclear(rs);

    return options;
}

