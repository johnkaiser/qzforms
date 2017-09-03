
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


void timestamp( struct handler_args *h ){

    xmlNodePtr divqz;
    xmlNodePtr cur = NULL;
    xmlNodePtr timelist;

    PGresult* rs;
    PGconn* conn = h->session->conn;
    char text[1024];


    double start = gettime();
    //rs = PQexec(conn, "SELECT now()");
    rs = PQprepare(conn, "timenow", "SELECT now()", 0, NULL);
    

    rs = PQexecPrepared(conn, "timenow", 0, NULL, NULL, NULL,0);

    if (PQresultStatus(rs) != PGRES_TUPLES_OK){
        PQclear(rs);
        return;
    }
    
    double finish = gettime();
    snprintf(text, 1024, "Access time for PQprepare and PQexecPrepared "
            "running \"SELECT now()\" is %f seconds", 
            finish - start);



    doc_from_file(h, "base.xml");
    if (h->error_exists) return;

    content_type(h, "text/html");

    divqz = qzGetElementByID(h, "qz");
    if (divqz != NULL){
        xmlNewTextChild(divqz, NULL, "p", PQgetvalue(rs,0,0));
        cur = xmlNewTextChild(divqz, NULL, "p", text);
        xmlNewProp(cur, "class", "light");
    }
    PQclear(rs);

    double t_op_start = gettime();

    struct table_action* words_edit = open_table(h, "words", "edit");
    double t_op_open_table = gettime();

    words_edit = open_table(h,"words", "edit");
    double t_op_table_from_hash = gettime();

    struct timeval tp;
    gettimeofday(&tp,NULL);
    int not_very_random =  (tp.tv_usec % 234978);
    char lookup_buf[1024];
    snprintf(lookup_buf, 1023, "%d", not_very_random);
    char* data[2];
    data[0] = lookup_buf; 
    data[1] = NULL;
    rs = perform_action(h, words_edit, data);
    double t_op_perform_action = gettime();

    timelist = xmlNewChild(cur, NULL, "ol", NULL);

    // open ta
    char open_ta[1024];
    snprintf(open_ta, 1023, "Time to open table_action %f", 
        t_op_open_table - t_op_start);
    xmlNewTextChild(timelist, NULL, "li", open_ta);

    // ta from hash
    char from_hash[1024];
    snprintf(from_hash, 1023, "Time to fetch table_action from hash %f", 
        t_op_table_from_hash - t_op_open_table);
    xmlNewTextChild(timelist, NULL, "li", from_hash);

    // perform table action
    char perf_ta[1024];
    snprintf(perf_ta, 1023, "Time to read word #%d \"%s\" is %f",
        not_very_random, 
        PQgetvalue(rs, 0, PQfnumber(rs, "word")),
        t_op_perform_action - t_op_table_from_hash
        );
    xmlNewTextChild(timelist, NULL, "li", perf_ta);


    double t_op_xml_stuff = gettime();

    // close table
    close_table(h, "words", "edit");
    double t_op_close =  gettime();

    char xml_stuff[1024];
    snprintf(xml_stuff,1023, "Time for new child list and items %f", 
        t_op_xml_stuff - t_op_perform_action);
    xmlNewTextChild(timelist, NULL, "li", xml_stuff);

    char close_table_time[1024];
    snprintf(close_table_time, 1023, "Time to close the table %f", 
        t_op_close - t_op_xml_stuff);
    xmlNewTextChild(timelist, NULL, "li", close_table_time);

    PQexec(conn, "DEALLOCATE timenow");
    PQclear(rs);


    // create an etag
    double t_create_etag = gettime();
    char tag_buf[50];

    make_etag(tag_buf, h->session->tagger_socket_path, 42);

    double t_validate_etag = gettime();

    uint64_t payload;

    payload = validate_etag(h->session->tagger_socket_path, tag_buf);

    double t_etag_fin = gettime();

    char etag_time[1024];

    snprintf(etag_time, 1023, "Time to create an etag %f (%s)",
        t_validate_etag - t_create_etag, tag_buf);
    
    xmlNewTextChild(timelist, NULL, "li", etag_time);

    char validate_time[1024];
    snprintf(validate_time, 1023, "Time to validate an etag %f (%llu)",
        t_etag_fin - t_validate_etag, payload);

    xmlNewTextChild(timelist, NULL, "li", validate_time);

    return;
}

