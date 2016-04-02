
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

#define BUFSIZE (1024)

void open_table_scanner(void* val, void* data, xmlChar* name){

    struct table_action* ta = val;
    xmlNodePtr thead = data;
    char* buf;
    xmlNodePtr tr = xmlNewChild(thead, NULL, "tr", NULL);

    xmlNewTextChild(tr, NULL, "td", ta->form_name);
    xmlNewTextChild(tr, NULL, "td", ta->action);
    xmlNewTextChild(tr, NULL, "td", ta->table_name);
    xmlNewTextChild(tr, NULL, "td", ta->prepare_name);

    asprintf(&buf, "%llu", ta->etag);
    xmlNewTextChild(tr, NULL, "td", buf);
    free(buf);

    return;    
}

void form_tag_status_scanner(void* val, void* data, xmlChar* name){
    struct form_record* form_tag = val;
    xmlNodePtr thead = data;

    xmlNodePtr tr = xmlNewChild(thead, NULL, "tr", NULL);
    xmlNewTextChild(tr, NULL, "td", form_tag->form_action);

    xmlNewTextChild(tr, NULL,  "td", 
        (form_tag->is_valid) ?  "t":"f");
        
    char buf[256];
    struct tm time_tm;
    localtime_r(&form_tag->created, &time_tm);
    strftime(buf, 255, "%Y-%m-%d %H:%M:%S", &time_tm); 
    xmlNewTextChild(tr, NULL,  "td", buf);

    localtime_r(&form_tag->expires, &time_tm);
    strftime(buf, 255, "%Y-%m-%d %H:%M:%S", &time_tm); 
    xmlNewTextChild(tr, NULL,  "td", buf);

    xmlNewTextChild(tr, NULL,  "td", 
        (form_tag->submit_only_once) ?  "t":"f");

    return;
}
void qz_status(struct handler_args* h){

    xmlNodePtr cur = NULL;
    xmlNodePtr divqz = NULL;

    content_type(h, "text/html");

    h->page_ta = open_table(h, "status", "view");
 
    h->doc = doc_from_file(h, "base.xml");

    if ((cur = xmlDocGetRootElement(h->doc)) == NULL){
        error_page(h, 500,  "Root element not found");
        return;
    }

    add_all_menus(h, cur);

    if ((divqz = qzGetElementByID(h, cur, "qz")) == NULL){
        error_page(h, 500,  "Element with id qz not found");
        return;
    }

    char* form_name = get_uri_part(h, QZ_URI_FORM_NAME);
    xmlNewTextChild(divqz,NULL,"h2", form_name);
    
    // Version
    char* version;
    asprintf(&version, "QZ Forms Version %.3f", QZVER);
    xmlNewTextChild(divqz, NULL, "p", version);
    free(version);

    asprintf(&version, "Expected Schema Version %s", SCHEMA_VER);
    xmlNewTextChild(divqz, NULL, "p", version);
    free(version);

    struct table_action* schema_ver_ta = open_table(h, "status", 
        "schema_version");

    PGresult* schema_ver_rs = perform_post_action(h, schema_ver_ta);
    asprintf(&version, "Installed Schema Version %s", 
        get_value(schema_ver_rs, 0, "schema_version"));

    xmlNewTextChild(divqz, NULL, "p", version);
    free(version);

    // button menu
    xmlNodePtr menu = xmlNewChild(divqz, NULL, "div", NULL);

    xmlNodePtr menu_item;

    menu_item = xmlNewTextChild(menu, NULL, "button", "change_history");
    xmlNewProp(menu_item, "onclick", "menu_click(\"change_history\")");

    menu_item = xmlNewTextChild(menu, NULL, "button", "pg_stat");
    xmlNewProp(menu_item, "onclick", "menu_click(\"pg_stat_activity\")");
/*
 *  see note below on handlers array
 *
 *   menu_item = xmlNewTextChild(menu, NULL, "button", "counters");
 *   xmlNewProp(menu_item, "onclick", "menu_click(\"counters\")");
 */

    menu_item = xmlNewTextChild(menu, NULL, "button", "open_tables");
    xmlNewProp(menu_item, "onclick", "menu_click(\"open_tables\")");

    menu_item = xmlNewTextChild(menu, NULL, "button", "form_tags");
    xmlNewProp(menu_item, "onclick", "menu_click(\"form_tags\")");

    // show change history
    struct table_action* change_history_ta = open_table(h, "status", 
        "change_history");
    PGresult* chg_hist_rs = perform_post_action(h, change_history_ta);
    rs_to_table(divqz, chg_hist_rs, "change_history");


    // show pg_stat_activity from postgresql
    struct table_action* stat_ta = open_table(h, "status", "pg_stat_activity");
    PGresult* stat_rs = perform_post_action(h, stat_ta);
    rs_to_table(divqz, stat_rs, "pg_stat_activity");

/*
 *  This needs to be changed from accessing a handlers array
 *  to a hash table callback now that handlers are now in a hash table.
 *
 *    // show handler access counter
 *    xmlNodePtr counter_table = xmlNewChild(divqz, NULL, "table", NULL);
 *    xmlNewProp(counter_table, "class", "qztable tablesorter");
 *    xmlNewProp(counter_table, "id", "counters");
 *
 *    xmlNodePtr counter_thead = xmlNewChild(counter_table, NULL, "thead", NULL);
 *    xmlNodePtr counter_tr = xmlNewChild(counter_thead, NULL, "tr", NULL);
 *    xmlNewTextChild(counter_tr, NULL, "th", "Handler");
 *    xmlNewTextChild(counter_tr, NULL, "th", "Count");
 *
 *    xmlNodePtr counter_tbody = xmlNewChild(counter_table, NULL, "tbody", NULL);
 *
 *    int n;
 *    for(n=0; 
 *        strlen(handlers[n].name) > 0 &&  handlers[n].handler != NULL; 
 *        n++){
 *
 *        counter_tr = xmlNewChild(counter_tbody, NULL, "tr", NULL);
 *        xmlNewTextChild(counter_tr, NULL, "td", handlers[n].name); 
 *        snprintf(buf, 1023, "%d", handlers[n].count);
 *        xmlNewTextChild(counter_tr, NULL, "td", buf);
 *    }
 *
 */
    // open tables
    xmlNodePtr ot_table = xmlNewChild(divqz, NULL, "table", NULL);
    xmlNewProp(ot_table, "class", "qztable tablesorter");
    xmlNewProp(ot_table, "id", "open_tables");

    xmlNodePtr ot_thead = xmlNewChild(ot_table, NULL, "thead", NULL);
    xmlNodePtr ot_tr = xmlNewChild(ot_thead, NULL, "tr", NULL);
    
    xmlNewTextChild(ot_tr, NULL, "th", "form_name");
    xmlNewTextChild(ot_tr, NULL, "th", "action");
    xmlNewTextChild(ot_tr, NULL, "th", "table_name");
    xmlNewTextChild(ot_tr, NULL, "th", "prepare_name");
    xmlNewTextChild(ot_tr, NULL, "th", "etag");

    xmlNodePtr ot_tbody = xmlNewChild(ot_table, NULL, "tbody", NULL);
    xmlHashScan(h->session->opentables, open_table_scanner, ot_tbody);

    // form tags XXXXXXXXX
    xmlNodePtr ft_table = xmlNewChild(divqz, NULL, "table", NULL);
    xmlNewProp(ft_table, "class", "qztable tablesorter");
    xmlNewProp(ft_table, "id", "form_tags");

    xmlNodePtr ft_thead = xmlNewChild(ft_table, NULL, "thead", NULL);
    xmlNodePtr ft_tr = xmlNewChild(ft_thead, NULL, "tr", NULL);

    xmlNewTextChild(ft_tr, NULL, "th", "form action");
    xmlNewTextChild(ft_tr, NULL, "th", "is valid");
    xmlNewTextChild(ft_tr, NULL, "th", "created");
    xmlNewTextChild(ft_tr, NULL, "th", "expires");
    xmlNewTextChild(ft_tr, NULL, "th", "submit only once");

    xmlNodePtr ft_tbody = xmlNewChild(ft_table, NULL, "tbody", NULL);
    xmlHashScan(h->session->form_tags, form_tag_status_scanner, ft_tbody);

    PQclear(chg_hist_rs);
    PQclear(stat_rs);

    return;
}
