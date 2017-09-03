
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

//  To add each prompt, a search is made for a prompt rule,
//  A rule that matches the form_name, fieldname tuple is used.
//  The rule will direct a specific type of prompt to be added.
//  The type named must call its specific function. 
//  The name to function pointer mapping is managed in  
//  struct session->prompt_types which is a hash table. 
//  In each hash table record is a struct prompt_adder.
//  struct prompt_add_args is the data blob that the
//  various functions take as an argument.
//  There should be one function for each item in enum prompt_types.

struct prompt_add_args {
    struct handler_args* hargs;
    struct table_action* t_action;
    struct prompt_rule* rule;
    struct pgtype_datum* pgtype;
    char** options;
    int row_index;
    xmlNodePtr child_of; 
    xmlChar* fname;
    xmlChar* fvalue;
};

struct prompt_adder{
    enum prompt_types prompt_type;
    char* name;
    xmlNodePtr (*add_prompt)(struct prompt_add_args*);
};

// Prompt name to function pointer mapping is initialized 
// by init_prompt_type_hash() at program startup.
static xmlHashTablePtr prompt_type_hash = NULL;

/*
 *  default_prompt_rule
 *
 *  Create one for when does not exist.
 *  Must be freed just like any other rule.
 */
struct prompt_rule* default_prompt_rule(struct handler_args* h,
    char* form_name, char* fieldname){

    static const char onchange_grid_default[] = "change_status(%n,'U')";

    bool is_grid = false;
    if ((h->page_ta != NULL) && (h->page_ta->handler_name != NULL)){
        if (strcmp(h->page_ta->handler_name, "grid") == 0){
            is_grid = true;
        }
    }

    unsigned int len = sizeof(struct prompt_rule) + 2;
    len += strlen(form_name) + 2;
    len += strlen(fieldname) + 2;
    char* prompt_type = "input_text";
    len += strlen(prompt_type) + 2;

    if (is_grid){
        len += strlen(onchange_grid_default) + 2;
    }

    struct prompt_rule* new_rule = calloc(1, len);

    char* marker =  new_rule->strdata;

    memcpy(marker, form_name, strlen(form_name)+1);
    new_rule->form_name = marker;
    marker += strlen(form_name)+2;

    memcpy(marker, fieldname, strlen(fieldname)+1);
    new_rule->fieldname = marker;
    marker += strlen(fieldname)+2;

    memcpy(marker, prompt_type, strlen(prompt_type)+1);
    new_rule->prompt_type = marker;
    marker += strlen(prompt_type)+2;

    if (is_grid){
        new_rule->expand_percent_n = true;

        memcpy(marker, onchange_grid_default, strlen(onchange_grid_default)+1);
        new_rule->onchange = marker;
        marker += strlen(onchange_grid_default)+2;
    }

    return new_rule;
}

/*
 *  fetch_prompt_rule
 * 
 *  Return the set of parameters to be used to turn the form_name, fieldname
 *  pair into an input prompt.
 *
 *  The result will contain both locally allocated and PGresult data.
 *  It must be freed with free_prompt_rule.
 */
struct prompt_rule* fetch_prompt_rule(struct handler_args* h, 
    char* form_name, char* fieldname){

    double start = gettime();
    bool log_errors = false;

    char* paramValues[] = {form_name, fieldname, NULL};

    PGresult* rs = PQexecPrepared(h->session->conn, "fetch_rule", 2,
        (const char* const*) &paramValues, NULL, NULL, 0);

    if ((PQresultStatus(rs) == PGRES_TUPLES_OK) && (PQntuples(rs) == 1)) {
   
        struct prompt_rule* rule = calloc(1,sizeof(struct prompt_rule));

        rule->form_name = get_value(rs, 0, "form_name");
        rule->fieldname =  get_value(rs, 0, "fieldname");
        rule->prompt_type = get_value(rs, 0, "prompt_type");
        rule->el_class = get_value(rs, 0, "el_class");

        rule->readonly =  get_bool(rs, 0, "readonly");

        rule->publish_pgtype = get_bool(rs, 0, "publish_pgtype");
       
        rule->rows = strtol( get_value(rs, 0, "rows"), NULL, 10);
        rule->cols = strtol( get_value(rs, 0, "cols"), NULL, 10);
        rule->size = strtol( get_value(rs, 0, "size"), NULL, 10);
        rule->maxlength = strtol(get_value(rs, 0, "maxlength"), NULL, 10);
        rule->tabindex = strtol(get_value(rs, 0, "tabindex"), NULL, 10);
        rule->regex_pattern = get_value(rs, 0, "regex_pattern");
        if (rule->regex_pattern != NULL){
            const char* error = NULL;
            int erroffset = 0;
            rule->comp_regex = pcre_compile( rule->regex_pattern, 
                PCRE_JAVASCRIPT_COMPAT|PCRE_UTF8,
                &error, &erroffset, NULL);

            if (rule->comp_regex == NULL){
                fprintf(h->log, "%f %d %s:%d "
                    "regex compile failed form %s field %s offset %d: %s\n",
                    gettime(), h->request_id, __func__, __LINE__,
                    form_name, fieldname, erroffset, error);
            }
        }

        rule->expand_percent_n = get_bool(rs, 0, "expand_percent_n");

        // The default value for rule->options is a pointer to a 
        // pg result set.  Whether rule->options is overwritten or not, 
        // the pg string will be freed by free_prompt_rule.
        // In a few cases, the options need to be generated dynamically.
        // Select pkey and enum are examples. In such a case, the 
        // string which needs to be independently freed is referenced
        // in free_options.
        rule->options = get_value(rs, 0, "options");
        rule->free_options = NULL;

        rule->src = get_value(rs, 0, "src");
        rule->onblur = get_value(rs, 0, "onblur");
        rule->onchange = get_value(rs, 0, "onchange");
        rule->onclick = get_value(rs, 0, "onclick");
        rule->ondblclick = get_value(rs, 0, "ondblclick");
        rule->onfocus = get_value(rs, 0, "onfocus");
        rule->onkeypress = get_value(rs, 0, "onkeypress");
        rule->onkeyup = get_value(rs, 0, "onkeyup");
        rule->onkeydown = get_value(rs, 0, "onkeydown");
        rule->onmousedown = get_value(rs, 0, "onmousedown");
        rule->onmouseup = get_value(rs, 0, "onmouseup");
        rule->onmouseout = get_value(rs, 0, "onmouseout");
        rule->onmouseover = get_value(rs, 0, "onmouseover");
        rule->onselect = get_value(rs, 0, "onselect");
 
        rule->result = rs;

        fprintf(h->log, "%f %d %s:%d fetch_prompt_rule (%s,%s) in %f\n",
                gettime(), h->request_id, __func__, __LINE__,
                form_name, fieldname, gettime() - start);

        return rule;
    }else{
        if (log_errors){
            fprintf(h->log, "%f %d %s:%d fetch_prompt_rules"
                "resStatus:%s cmdStatus:%s ErrorMessage:%s\n", 
                gettime(), h->request_id, __func__, __LINE__,
                PQresStatus(PQresultStatus(rs)),
                PQcmdStatus(rs),
                PQresultErrorMessage(rs));
        }
        PQclear(rs);
        return default_prompt_rule(h, form_name, fieldname);
    }    
}

/*
 *  free_prompt_rule
 *
 *  Frees a previously created prompt rule.
 */
void free_prompt_rule(struct handler_args* h, 
    struct prompt_rule* rule){

    if (rule == NULL) return;

    if (rule->result != NULL){
        PQclear(rule->result);
        rule->result = NULL;
    }
    if (rule->free_options != NULL){
        free(rule->free_options);
    }
    if (rule->comp_regex != NULL){
        pcre_free(rule->comp_regex);
        rule->comp_regex = NULL;
    }
    free(rule);
}


/*
 *  just a placeholder.
 */
xmlNodePtr add_not_implemented(struct prompt_add_args* args){

    char* note;
    asprintf(&note, "%s not implemented", args->rule->prompt_type );

    xmlNodePtr nimp = xmlNewTextChild(args->child_of, NULL, "p", note);

    free(note);
    return nimp;
}

/*
 *  truthishness
 *
 *  Evaluate the given string as a boolean.
 *  Uses Postgresql's definition of string literals.
 *  TRUE 't' 'true' 'y' 'yes' 'on' '1'
 *  FALSE 'f' 'false' 'n' 'no' 'off' '0'`
 */
bool truthishness(struct handler_args* h, char* str){

   // XXXXXXXXXXXXXx lose the n characters
   if (strncasecmp("TRUE", str, 4) == 0) return true;
   if (strncasecmp("FALSE", str, 4) == 0) return false;

   if (strncasecmp("yes", str, 4) == 0) return true;
   if (strncasecmp("no", str, 4) == 0) return false;

   if (strncasecmp("on", str, 4) == 0) return true;
   if (strncasecmp("off", str, 4) == 0) return false;

   if (strlen(str) == 1){
       if (str[0] == 't') return true;
       if (str[0] == 'f') return false;

       if (str[0] == 'y') return true;
       if (str[0] == 'n') return false;

       if (str[0] == '1') return true;
       if (str[0] == '0') return false;
   }

   // fail massively 
   fprintf(h->log, "%f %d %s:%d fail, truthishness of %s "
        "could not be determined\n",
        gettime(), h->request_id, __func__, __LINE__, str);

   return false;
}

/*
 *  quantify_percent_n
 *
 *  Fundamentally, I want to be able to tell a javascript
 *  function what object to bind to.  For arrays, it is a
 *  numerically unique value stored as base[%n] and filled
 *  in by this function, which when n = 6 would be base[6]

 *  I am quite sure this is the hard way, but I don't want
 *  to pass the string to c lib, so here it is.
 *
 *  The result must be freed.
 */

#define MAX_SUBSTITUTIONS (100)
#define MAX_EVENT_LENGTH (4096)

char* quantify_percent_n(char* event, int n){

    int event_len = strlen(event);
    if (event_len == 0) return NULL;
    if (event_len > MAX_EVENT_LENGTH) return NULL;

    // I need to know how long n will be as text.
    char* n_str;
    asprintf(&n_str, "%d", n);
    int len_n = strlen(n_str);
    
    // How many times
    int nbr_substitutions = 0;
    enum  {looking_for_percent, looking_for_n} state;
    state = looking_for_percent;

    char* ch;
    for (ch=event; *ch!='\0'; ch++){
        if ((state == looking_for_percent) && (*ch == '%')){
            state = looking_for_n;
        }else if (state == looking_for_n){
            if (*ch == 'n'){
                nbr_substitutions++;
            }
            state = looking_for_percent;
        }
    } 
    if (nbr_substitutions > MAX_SUBSTITUTIONS) return NULL;

    // Each %n, 2 chars will become len_n, a gain or loss of (len_n - 2)
    // and two nulls at the end.
    int total_length = event_len + nbr_substitutions * (len_n-2) + 2;
    char* return_str;
    return_str = calloc(1, total_length);

    // look for a shortcut
    if (nbr_substitutions == 0){
        memcpy(return_str, event, event_len);
    }else{
         
         char* to = return_str;
         char* from;
         state = looking_for_percent;

         for (from=event; *event != '\0'; event++){
             switch(state){
                 case looking_for_percent:
                     if (*event == '%'){
                         state = looking_for_n;
                         from++;
                     }else{
                         *to = *from;
                         to++;
                         from++;
                     }    
                     break;

                 case looking_for_n:
                     if (*event == 'n'){
                         char* nch;
                         for (nch=n_str; *nch!='\0'; nch++){
                             *to = *nch;
                             to++;
                         }
                         from++;
                     }else{
                         *to = '%';
                         to++;
                         *to = *from;
                         to++;
                         from++;
                     }
                     state = looking_for_percent;
                     break;
             }
         }    
    }
    
    free(n_str);
    return return_str;
}



/*
 *  add_event
 *
 *  Add the event, expanding any %n values to the given row.
 *
 */

void add_event(struct prompt_add_args* args, xmlNodePtr input, char* event_name, char* event_action){

    if ((args->rule != NULL) && (args->rule->expand_percent_n)){

        char* new_event_action = quantify_percent_n(event_action, args->row_index);
        xmlNewProp(input, event_name, new_event_action);
        free(new_event_action);
        
    }else{
        xmlNewProp(input, event_name, event_action);
    }
}

/*
 *  set_common_attributes
 *
 *  Set the on event attributes, 
 *
 *  This should be called by every prompt adding function.
 */
void set_common_attributes(struct prompt_add_args* args, xmlNodePtr input){

    if (args->rule->tabindex > 0){
        char* intbuf;
        asprintf(&intbuf, "%d", args->rule->tabindex);
        xmlNewProp(input, "tabindex", intbuf);
        free(intbuf);
    } 

    // class gets inserted to a list
    if ((args->rule->el_class != NULL) && args->rule->el_class[0]!='\0'){
        append_class(input, args->rule->el_class);
    }    
    // Add a class for the pg attribute name
    // but remove the [n] from base[n]
    char* fieldname;
    asprintf(&fieldname, "%s", args->rule->fieldname);
    int k;
    for (k=0; k<strlen(fieldname); k++){
        if (fieldname[k] == '[') fieldname[k] = '\0';
    }
    append_class(input, fieldname);
    free(fieldname);
    fieldname = NULL;

    // add the pgtype_datum as a json attribute 
    if ((args->pgtype != NULL) && (args->rule->publish_pgtype)){
        char* pgtype_json = pgtype_datum_to_json(args->pgtype);
        if (pgtype_json != NULL){
            char* escaped_pgtype = xmlURIEscapeStr(pgtype_json, NULL);
            if (escaped_pgtype != NULL){
                xmlNewProp(input, "pgtype", escaped_pgtype);
                free(escaped_pgtype);
            }    
            free(pgtype_json);
        }    
    }

    if (has_data(args->rule->regex_pattern)){
        xmlNewProp(input, "pattern", args->rule->regex_pattern);
    }    

    if (has_data(args->rule->onfocus)) add_event(args, input, 
        "onfocus", args->rule->onfocus);

    if (has_data(args->rule->onblur))  add_event(args, input, 
        "onblur", args->rule->onblur);

    if (has_data(args->rule->onchange))  add_event(args, input, 
        "onchange", args->rule->onchange);

    if (has_data(args->rule->onselect))  add_event(args, input, 
        "onselect", args->rule->onselect);

    if (has_data(args->rule->onclick))  add_event(args, input, 
        "onclick", args->rule->onclick);

    if (has_data(args->rule->ondblclick))  add_event(args, input, 
        "ondblclick", args->rule->ondblclick);

    if (has_data(args->rule->onmousedown))  add_event(args, input, 
        "onmousedown", args->rule->onmousedown);

    if (has_data(args->rule->onmouseup))  add_event(args, input, 
        "onmouseup", args->rule->onmouseup);

    if (has_data(args->rule->onmouseover))  add_event(args, input, 
        "onmouseover", args->rule->onmouseover);

    if (has_data(args->rule->onmouseout))  add_event(args, input, 
        "onmouseout", args->rule->onmouseout);

    if (has_data(args->rule->onkeypress))  add_event(args, input, 
        "onkeypress", args->rule->onkeypress);

    if (has_data(args->rule->onkeydown))  add_event(args, input, 
        "onkeydown", args->rule->onkeydown);

    if (has_data(args->rule->onkeyup))  add_event(args, input, 
        "onkeyup", args->rule->onkeyup);

    if (has_data(args->rule->src)) add_event(args, input, 
        "src", args->rule->src);

    return;
}

/*
 *  add_button
 *
 *  Add a push button.
 */
xmlNodePtr add_button(struct prompt_add_args* args){

    xmlNodePtr button;
    button = xmlNewTextChild(args->child_of, NULL, "button", args->fvalue);
    xmlNewProp(button, "name", args->fname);
    xmlNewProp(button, "id", args->fname);
    // A button type button, not a reset or submit,
    // but perfect for js.
    xmlNewProp(button, "type", "button");  

    set_common_attributes(args, button);

    return button; 
}


xmlNodePtr add_input_button(struct prompt_add_args* args){ return add_not_implemented(args); }
xmlNodePtr add_input_checkbox(struct prompt_add_args* args){ return add_not_implemented(args); }
xmlNodePtr add_input_file(struct prompt_add_args* args){ return add_not_implemented(args); }
xmlNodePtr add_input_image(struct prompt_add_args* args){ return add_not_implemented(args); }
xmlNodePtr add_input_password(struct prompt_add_args* args){ return add_not_implemented(args); }
xmlNodePtr add_input_reset(struct prompt_add_args* args){ return add_not_implemented(args); }
xmlNodePtr add_input_submit(struct prompt_add_args* args){ return add_not_implemented(args); }

/*
 *  add_input_hidden
 *
 *  Add a hidden field.
 */
xmlNodePtr add_input_hidden(struct prompt_add_args* args){

    xmlNodePtr hidden;

    hidden = xmlNewChild(args->child_of, NULL, "input", NULL);
    xmlNewProp(hidden, "type", "hidden");
    xmlNewProp(hidden, "name", args->fname);
    xmlNewProp(hidden, "id", args->fname);
    xmlNewProp(hidden, "value", args->fvalue);

    set_common_attributes(args, hidden);

    return hidden;
}
 
/*
 *  add_input_text
 *
 *  Add an input type=text style prompt.
 */
xmlNodePtr add_input_text(struct prompt_add_args* args){ 
    xmlNodePtr input;
    char* buf;

    input = xmlNewChild(args->child_of, NULL, "input", NULL);
    xmlNewProp(input, "type", "text");
    xmlNewProp(input,"name", args->fname);
    xmlNewProp(input,"id", args->fname);

    if (args->fvalue!=NULL) xmlNewProp(input, "value", args->fvalue);

    //  rule can not be null as add_prompt gives it a default value.
    //  Checking anyway to please clang scan-build
    if (args->rule != NULL){
        if (args->rule->size > 0){
            asprintf(&buf, "%d", args->rule->size);
            xmlNewProp(input, "size", buf);
        }

        if (args->rule->maxlength > 0){
            char* intbuf;
            asprintf(&intbuf, "%d", args->rule->maxlength);
            xmlNewProp(input, "maxlength", intbuf);
            free(intbuf);
        }
        set_common_attributes(args, input);
    }

    return input;
}

/*
 *  add_select_options
 *
 *  Create a drop down, select options from the array in
 *  the prompt rule attribute, options 
 *  or prompt_type attribute, enum_labels.
 */

xmlNodePtr add_select_options(struct prompt_add_args* args){
    xmlNodePtr select;
    xmlNodePtr select_option;

    select = xmlNewChild(args->child_of, NULL, "select", NULL);
    xmlNewProp(select, "name", args->fname);
    xmlNewProp(select, "id", args->fname);

    // If pgtype->is_nullable then add empty option
    // XXXXX Will need a prompt rule for required prompts
    if ((args->pgtype != NULL) && (args->pgtype->is_nullable)){
        select_option = xmlNewTextChild(select, NULL, "option", "_");
        xmlNewProp(select_option, "value", "");

        //if there is no fvalue, then this is selected
        if ((args->fvalue == NULL) || (args->fvalue[0] == '\0')){
            xmlNewProp(select_option, "selected", "selected");
        }
    }    

    
    if (args->options == NULL){
         //log it and error out
        fprintf(args->hargs->log, "%f %d %s:%d fail, options not found "
            "- set prompt rule options\n",
            gettime(), args->hargs->request_id, __func__, __LINE__);

        return select;
    }

    int n;
    for(n=0; args->options[n] != NULL; n++){

        // add the option
        select_option = xmlNewTextChild(select, NULL, "option", 
            args->options[n]);

        xmlNewProp(select_option, "value", args->options[n]);
            
        // and look to see if it is the one selected
        bool selected = false;

        if ((args->fvalue != NULL) &&
            (strcasecmp(args->fvalue, args->options[n]) == 0)){
                selected = true;
        }

        if ((args->pgtype != NULL) && (args->pgtype->is_boolean)){
            if ((truthishness(args->hargs, args->fvalue) == true) 
                 && (truthishness(args->hargs, args->options[n]) == true)){
                
                selected = true;
            }    
            if ((truthishness(args->hargs, args->fvalue) == false) 
                && (truthishness(args->hargs, args->options[n]) == false)){

                selected = true;
            }    
        }
        if (selected){
            xmlNewProp(select_option, "selected", "selected");
        }
    }

    set_common_attributes(args, select);

    return select;
}

/*
 *  add_select_fkey
 *
 *  Add a select drop down where the list of items comes
 *  from the foreign key.
 *
 *  XXXXX This will need to be extended to handle 10,000 
 *  items or more in the foreign key.  This implementation
 *  only handles a reasonable number of options.
 */

xmlNodePtr add_select_fkey(struct prompt_add_args* args){


    if (args->pgtype == NULL){
        fprintf(args->hargs->log, "%f %d %s:%d fail, "
            "no pgtype record available\n", 
            gettime(), args->hargs->request_id, __func__, __LINE__);

        // Add a text input instead.      
        return add_input_text(args);
    }    

    if (! args->pgtype->has_fkey){

        fprintf(args->hargs->log, "%f %d %s:%d fail, "
            "select_fkey called for non foreign key field %s.%s %s\n",
            gettime(), args->hargs->request_id, __func__, __LINE__,
            args->pgtype->table_schema, args->pgtype->table_name,
            args->pgtype->column_name);

        // Add a text input instead.      
        return add_input_text(args);
    } 
    return add_select_options(args);
}
/*
 *  add_input_radio
 *
 *  Add a set of radio buttons for the input.
 *  Use the same option, enum logic as select options.
 *  Handle booleans.
 */
xmlNodePtr add_input_radio(struct prompt_add_args* args){ 

    xmlNodePtr radio_btn = NULL;
    xmlNodePtr next_btn;
    xmlNodePtr label;
    int n;
    char* id_buf;
    
    if (args->options == NULL){
         fprintf(args->hargs->log, "%f %d %s:%d fail, "
            "radio button requires options. "
            "form_name=%s fieldname=%s\n",
            gettime(), args->hargs->request_id, __func__, __LINE__, 
            args->rule->form_name, args->rule->fieldname);
 
        return NULL;
    }    

    for(n=0; args->options[n] != NULL; n++){ 
        next_btn = xmlNewChild(args->child_of, NULL, "input", NULL);
        if (radio_btn == NULL) radio_btn = next_btn; //save the first to return.

        xmlNewProp(next_btn, "type", "radio");
        xmlNewProp(next_btn, "name", args->fname);
        xmlNewProp(next_btn, "value", args->options[n]);
        
        asprintf(&id_buf, "%s_%d", args->fname, n);
        xmlNewProp(next_btn,"id", id_buf);

        label = xmlNewTextChild(args->child_of,NULL, "label", args->options[n]);
        xmlNewProp(label, "for", id_buf);

        char* classname;
        asprintf(&classname, "%s", args->fname);
        int k;
        for (k=0; k<strlen(classname); k++){
            if (classname[k] == '[') classname[k] = '\0';
        }
 
        xmlNewProp(label, "class", classname);
        free(classname);
        classname = NULL;

        free(id_buf);
        id_buf = NULL;

        // There are different comparisons that may work depending on the
        // data type of the pg attribute.  
        bool checked = false;

        // By this I am deciding that options that differ only by case will
        // break in unexpeced ways.
        if ((args->fvalue != NULL)  && 
            (strcasecmp(args->fvalue, args->options[n]) == 0)){
            
            checked = true;
        } 
        
        if ((args->pgtype != NULL) && (args->pgtype->is_boolean)){
            if ((truthishness(args->hargs, args->fvalue) == true) 
                 && (truthishness(args->hargs, args->options[n]) == true)){
                
                checked = true;
            }    
            if ((truthishness(args->hargs, args->fvalue) == false) 
                && (truthishness(args->hargs, args->options[n]) == false)){

                checked = true;
            }    
        }
        if (checked){
            xmlNewProp(next_btn, "checked", "checked");
        }
        
        set_common_attributes(args, next_btn);
    }    

    return radio_btn;
}

/*
 *  add_textarea
 *
 *  Add a place for a block of text.
 */

xmlNodePtr add_textarea(struct prompt_add_args* args){
    xmlNodePtr textarea;

    textarea = xmlNewTextChild(args->child_of, NULL, "textarea", args->fvalue);
    xmlNewProp(textarea,"name", args->fname);
    xmlNewProp(textarea,"id", args->fname);

    char* value_buf;
    if (args->rule->rows > 0){
        asprintf(&value_buf, "%d", args->rule->rows);
        xmlNewProp(textarea, "rows", value_buf);
        free(value_buf);
        value_buf = NULL;
    }
    if (args->rule->cols > 0){
        asprintf(&value_buf, "%d", args->rule->cols);
        xmlNewProp(textarea, "cols", value_buf);
        free(value_buf);
        value_buf = NULL;
    }
    
    set_common_attributes(args, textarea);

    return textarea;
}


/*
 *  json_add_element_args
 *
 *  I need to pass to javascript, a representation of the
 *  prompt for input arrays so that javascript can add input 
 *  elements with all the proper attributes.
 *
 *  This creates that representation.
 *
 *  If func_name is not null, it becomes a wrapping function call:
 *  func_name({...})
 *  If func_name is null, this just returns the a json object {...}.
 *
 *  The result must be freed.
 */

char* json_add_element_args(char* func_name, struct prompt_rule* rule, 
    char* option_ar[]){

     if ((rule==NULL) || (rule->fieldname==NULL)){
         char* empty = calloc(1,2);
         return (char*) empty;
     }

     char* fieldname = NULL;
     asprintf(&fieldname, "\"fieldname\":\"%s\" ", rule->fieldname);

     char* prompt_type = "";
     if ((rule->prompt_type != NULL) && (rule->prompt_type[0] != '\0')){
        asprintf(&prompt_type, "\"prompt_type\":\"%s\", ", rule->prompt_type);
     }

     char* el_class = "";
     if ((rule->el_class != NULL) && (rule->el_class[0] != '\0')){
         asprintf(&el_class, "\"class\":\"%s\", ", rule->el_class);
     }

     char* expand_percent_n = "";
     if (rule->expand_percent_n){
         asprintf(&expand_percent_n, "\"expand_percent_n\":\"%s\", ",
             (rule->expand_percent_n) ? "true":"false");
     }        
     char* readonly = "";
     if (rule->readonly){
         asprintf(&readonly, "\"readonly\":\"%s\", ", 
             (rule->readonly) ? "true":"false");
     }        

     char* pattern = "";
     if ((rule->regex_pattern != NULL) && (rule->regex_pattern[0] != '\0')){
         char* pattern64 = base64_encode(rule->regex_pattern);
         asprintf(&pattern, "\"pattern\":\"%s\", ", pattern64);
         free(pattern64);
     }

     char* rows = "";
     if (rule->rows > 0){
         asprintf(&rows, "\"rows\":\"%d\", ", rule->rows);
     }    

     char* cols = "";
     if (rule->cols > 0){
         asprintf(&cols, "\"cols\":\"%d\", ", rule->cols);
     }    

     char* size = "";
     if (rule->size > 0){
         asprintf(&size, "\"size\":\"%d\", ", rule->size);
     }

     char* maxlength = "";
     if (rule->maxlength > 0){
         asprintf(&maxlength, "\"maxlength\":\"%d\", ", rule->maxlength);
     }

     char* tabindex = "";
     if (rule->tabindex > 0){
         asprintf(&tabindex, "\"tabindex\":\"%d\", ", rule->tabindex);
     }    

     char* options = "";
     if (option_ar != NULL){
         char* js_options = NULL;
         js_options = str_ar_to_json(option_ar);
         asprintf(&options, "\"options\":%s, ", js_options);
         free(js_options);
         js_options = NULL;
     }
     
     char* onblur = "";
     if ((rule->onblur != NULL) && (rule->onblur[0] != '\0')){
         char* onblur64 = base64_encode(rule->onblur);
         asprintf(&onblur, "\"onblur\":\"%s\", ", onblur64);
         free(onblur64);
     }    

     char* onchange = "";
     if ((rule->onchange != NULL) &&  (rule->onchange[0] != '\0')){
         char* onchange64 = base64_encode(rule->onchange);
         asprintf(&onchange, "\"onchange\":\"%s\", ", onchange64);
         free(onchange64);
     }    

     char* onclick = "";
     if ((rule->onclick != NULL) && (rule->onclick[0] != '\0')){
         char* onclick64 = base64_encode(rule->onclick);
         asprintf(&onclick, "\"onclick\":\"%s\", ", onclick64);
         free(onclick64);
     }    

     char* ondblclick = "";
     if ((rule->ondblclick != NULL) && (rule->ondblclick[0] != '\0')){
         char* ondblclick64 = base64_encode(rule->ondblclick);
         asprintf(&ondblclick, "\"ondblclick\":\"%s\", ", ondblclick64);
         free(ondblclick64);
     }

     char* onfocus = "";
     if ((rule->onfocus != NULL) && (rule->onfocus[0] != '\0')){ 
         char* onfocus64 = base64_encode(rule->onfocus);
         asprintf(&onfocus, "\"onfocus\":\"%s\", ", rule->onfocus);
         free(onfocus64);
     }    

     char* onkeypress = "";
     if ((rule->onkeypress != NULL) && (rule->onkeypress[0] != '\0')){
         char* onkeypress64 = base64_encode(rule->onkeypress);
         asprintf(&onkeypress, "\"onkeypress\":\"%s\", ", rule->onkeypress);
         free(onkeypress64);
     }    

     char* onkeyup = "";
     if ((rule->onkeyup != NULL) && (rule->onkeyup[0] != '\0')){
         char* onkeyup64 = base64_encode(rule->onkeyup);
         asprintf(&onkeyup, "\"onkeyup\":\"%s\", ", rule->onkeyup);
         free(onkeyup64);
     }    

     char* onkeydown = "";
     if ((rule->onkeydown != NULL) && (rule->onkeydown[0] != '\0')){
         char* onkeydown64 = base64_encode(rule->onkeydown);
         asprintf(&onkeydown, "\"onkeydown\":\"%s\", ", rule->onkeydown);
         free(onkeydown64);
     }    

     char* onmousedown = "";
     if ((rule->onmousedown != NULL) && (rule->onmousedown[0] != '\0')){
         char* onmousedown64 = base64_encode(rule->onmousedown);
         asprintf(&onmousedown, "\"onmousedown\":\"%s\", ", rule->onmousedown);
         free(onmousedown64);
     }

     char* onmouseout = "";
     if ((rule->onmouseout != NULL) && (rule->onmouseout[0] != '\0')){
         char* onmouseout64 = base64_encode(rule->onmouseout);
         asprintf(&onmouseout, "\"onmouseout\":\"%s\", ", rule->onmouseout);
         free(onmouseout64);
     }

     char* onmouseover = "";
     if ((rule->onmouseover != NULL) && (rule->onmouseover[0] != '\0')){
         char* onmouseover64 = base64_encode(rule->onmouseover);
         asprintf(&onmouseover, "\"onmouseover\":\"%s\",  ", onmouseover64);
         free(onmouseover64);
     }

     char* onselect = "";
     if ((rule->onselect != NULL) && (rule->onselect[0] != '\0')){
         char* onselect64 = base64_encode(rule->onselect);
         asprintf(&onselect, "\"onselect\":\"%s\", ", onselect64);
         free(onselect64);
     }

     //char* end = "\"z\":\"end\""; // no comma, lame hack
     
     char* json_args;


     if (func_name == NULL){
         asprintf(&json_args, "{%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s}",
             prompt_type, el_class, expand_percent_n, readonly, pattern, 
             rows, cols, size, maxlength, tabindex, options,
             onblur, onchange, onclick, ondblclick, onfocus,
             onkeypress, onkeyup, onkeydown, onmousedown, onmouseout,
             onmouseover, onselect,
             fieldname);

     }else{
         asprintf(&json_args, "%s({%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s});",
             func_name,  // Having this requires name() around the object
             prompt_type, el_class, expand_percent_n, readonly, pattern,
             rows, cols, size, maxlength, tabindex, options,
             onblur, onchange, onclick, ondblclick, onfocus,
             onkeypress, onkeyup, onkeydown, onmousedown, onmouseout,
             onmouseover, onselect,
             fieldname);

     }
     free(fieldname);
     if (*prompt_type != '\0') free(prompt_type);
     if (el_class[0] != '\0') free(el_class);
     if (expand_percent_n[0] != '\0') free(expand_percent_n);
     if (readonly[0] != '\0') free(readonly);
     if (pattern[0] != '\0') free(pattern);
     if (rows[0] != '\0') free(rows);
     if (size[0] != '\0') free(size);
     if (maxlength[0] != '\0') free(maxlength);
     if (tabindex[0] != '\0') free(tabindex);
     if (options[0] != '\0') free(options);
     if (onblur[0] != '\0') free(onblur);
     if (onchange[0] != '\0') free(onchange);
     if (onclick[0] != '\0') free(onclick);
     if (ondblclick[0] != '\0') free(ondblclick);
     if (onfocus[0] != '\0') free(onfocus);
     if (onkeypress[0] != '\0') free(onkeypress);
     if (onkeyup[0] != '\0') free(onkeyup);
     if (onkeydown[0] != '\0') free(onkeydown);
     if (onmousedown[0] != '\0') free(onmousedown);
     if (onmouseout[0] != '\0') free(onmouseout);
     if (onmouseover[0] != '\0') free(onmouseover);
     if (onselect[0] != '\0') free(onselect);

     return json_args;    
}

/*
 *  add_text_array
 *
 *  Add a list of entries where each list item
 *  is an array element.
 */

xmlNodePtr add_text_array(struct prompt_add_args* args){

    xmlNodePtr text_array;
    text_array = xmlNewChild(args->child_of, NULL, "ol", NULL);
    xmlNewProp(text_array, "id", args->fname);

    char** array_elements = parse_pg_array(args->fvalue);

    if (array_elements != NULL){

        int n;
        for(n=0; array_elements[n] != NULL; n++){
            xmlNodePtr array_item;
            array_item = xmlNewChild(text_array, NULL, "li", NULL);
    
            // create a name like base[0], base[1], etc.
            char* fname_buf;
            asprintf(&fname_buf, "%s[%d]", args->fname, n);

            struct prompt_add_args newargs = (struct prompt_add_args){
                .hargs = args->hargs,
                .t_action = args->t_action,
                .rule = args->rule,
                .pgtype = args->pgtype,
                .row_index = n,
                .child_of = array_item,
                .fname = fname_buf,
                .fvalue = array_elements[n]
            };
            args->rule->publish_pgtype = false;
            
            xmlNodePtr new_input = add_input_text( &newargs );
            
            set_common_attributes(&newargs, new_input);

            free(fname_buf);
        }

        free(array_elements);
    }    

    // add an add new item button.
    char* button_name;
    asprintf(&button_name, "%s_btn", args->fname);

    xmlNodePtr button;
    button = xmlNewTextChild(args->child_of, NULL, "button", "+Add Element");
    xmlNewProp(button, "name", button_name);
    xmlNewProp(button, "id", button_name);
    // A button type button, not a reset or submit, but perfect for js.
    xmlNewProp(button, "type", "button");  

    if (args->rule != NULL){
        char* args_rule_js = json_add_element_args(NULL, args->rule, NULL);
        if (args_rule_js != NULL){
            char* args_rule_esc = xmlURIEscapeStr(args_rule_js, NULL); 
            char* event_esc;
            asprintf(&event_esc, "%s('%s');", "add_array_input", args_rule_esc);

            xmlNewProp(button, "onclick", event_esc); 
            free(event_esc);
            free(args_rule_esc);
            free(args_rule_js);
        }    
    }
    free(button_name);
    return text_array;
}

/*
 * add_prompt
 *
 * Add the intput elements and attributes specified by rule
 * for field name fname with a default value of fvalue.
 * fvalue may be null, fname not.
 *
 *  XXXXXXX Maybe have separate setup and doit functions so 
 *  XXXXXXX that the component can be created without the fieldset.
 *  XXXXXXX For now, call the particular directly and set the
 *  XXXXXXX events in the common function.
 */

void add_prompt(struct handler_args* hargs,
              struct table_action* t_action,
              struct prompt_rule* rule,
              struct pgtype_datum* pgtype,
              char* options[],
              int row_index,
              xmlNodePtr child_of, 
              xmlChar* fname, 
              xmlChar* fvalue){
    
    // Initialize a struct prompt_add_args
    static char* default_name = "default";
    static char* input_text = "input_text";

    struct prompt_add_args arg_record = (struct prompt_add_args){
        .hargs = hargs,
        .t_action = t_action,
        .rule = rule,
        .pgtype = pgtype,
        .options = options,
        .row_index = row_index,
        .child_of = child_of, 
        .fname = fname,
        .fvalue = fvalue
    };
    struct prompt_add_args* args = &arg_record;

    struct prompt_rule default_rule = (struct prompt_rule){
        .form_name = default_name,
        .fieldname = fname,
        .prompt_type = "input_text",
    };

    if (args->rule == NULL) args->rule = &default_rule;
    if (args->rule->prompt_type == NULL) args->rule->prompt_type = input_text;

    xmlNodePtr input = NULL;
    // !*&^$#%!!!
    // 
    struct prompt_adder* prompt = xmlHashLookup(prompt_type_hash, 
        args->rule->prompt_type);

    if (prompt == NULL){
        fprintf(hargs->log, "%f %d %s:%d fail prompt type not found %s\n",
            gettime(), hargs->request_id, __func__, __LINE__,
            args->rule->prompt_type);
        error_page(hargs, SC_INTERNAL_SERVER_ERROR, "Prompt Type not found");

        return;
    }

    int pk;
    bool is_pkey = false;
    char* mod_fname;
    asprintf(&mod_fname, "%s", fname);
    int k;
    for(k=0; k<strlen(mod_fname); k++){
        if (mod_fname[k] == '[') mod_fname[k] = '\0';
    }    
    if (t_action != NULL){
        for (pk=0; pk<t_action->nbr_pkeys; pk++){
            if (strcmp(mod_fname, t_action->pkeys[pk]) == 0){
                is_pkey = true;
                break;
            }
        }
    }
    free(mod_fname);
    mod_fname = NULL;

    xmlNodePtr fieldset = NULL; 
    if ((t_action != NULL) && (t_action->prompt_container != NULL) && ( 
        strcmp(t_action->prompt_container, "fieldset") == 0)){
        // Everything except a hidden field gets a fieldset.
        // XXXXX Make fieldset a field in prompt_rule so that
        // XXXXX multiple fields can exist within one fieldset.
        // XXXXX The managing of fieldsets would have to happen
        // XXXXX further up the call stack, otherwise add_prompts
        // XXXXX would have to keep state to know about prior fieldsets.
        if ((args->rule != NULL) && (args->rule->prompt_type != NULL) &&
            strncmp("input_hidden", args->rule->prompt_type, strlen("input_hidden")
            ) != 0){
            
            fieldset = xmlNewChild(child_of, NULL, "fieldset", NULL);
            xmlNewTextChild(fieldset, NULL, "legend", fname );
            args->child_of = fieldset;    
        }
    }

    // !*&^$%#!!!
    // Execute the prompt adder function for the given prompt type.
    input = prompt->add_prompt(args);

    if (input == NULL) return; // Some error occured 

    if (t_action != NULL){
        fprintf(hargs->log, "%f %d %s:%d t_action->add_description=%s\n", 
            gettime(), hargs->request_id, __func__, __LINE__,
            (t_action->add_description) ? "true":"false");
    }

    // Add the description off to the side if it is available.
    if ( (t_action != NULL) &&
         (t_action->add_description) &&
         (args->pgtype != NULL) && 
         (args->pgtype->description != NULL) &&
         (args->pgtype->description[0] != '\0') 
         && (fieldset != NULL)){

        xmlNodePtr description;
        description = xmlNewTextChild(fieldset, NULL, "span", 
            args->pgtype->description);

        xmlNewProp(description, "class", "description");
    } 
    
    if ((args->rule->readonly) || (is_pkey && has_data(fvalue))){
        xmlNewProp(input,"readonly","readonly"); 
    }

    return;
}

void init_prompt_type_hash(void){


    static struct prompt_adder const static_prompt_types[] =
    {

        {
            .prompt_type = button,
            .name = "button",
            .add_prompt = add_button
        },
        {
            .prompt_type = input_checkbox,
            .name = "input_checkbox",
            .add_prompt = add_input_checkbox
        },
        {
            .prompt_type = input_file,
            .name = "input_file",
            .add_prompt = add_input_file
        },
        {
            .prompt_type = input_hidden,
            .name = "input_hidden",
            .add_prompt = add_input_hidden
        },
        {
            .prompt_type = input_image,
            .name = "input_image",
            .add_prompt = add_input_image
        },
        {
            .prompt_type = input_password,
            .name = "input_password",
            .add_prompt = add_input_password
        },
        {
            .prompt_type = input_radio,
            .name = "input_radio",
            .add_prompt = add_input_radio
        },
        {
            .prompt_type = input_reset,
            .name = "input_reset",
            .add_prompt = add_input_reset
        },
        {
            .prompt_type = input_submit,
            .name = "input_submit",
            .add_prompt = add_input_submit
        },
        {
            .prompt_type = input_text,
            .name = "input_text",
            .add_prompt = add_input_text
        },
        {
            .prompt_type = select_options,
            .name = "select_options",
            .add_prompt = add_select_options
        },
        {
            .prompt_type = select_fkey,
            .name = "select_fkey",
            .add_prompt = add_select_fkey
        },    
        {
            .prompt_type = textarea,
            .name = "textarea",
            .add_prompt = add_textarea
        },
        {
            .prompt_type = text_array,
            .name = "text_array",
            .add_prompt = add_text_array
        },
        {
            .prompt_type = none,
            .name = NULL,
            .add_prompt = NULL
        }    
     };
    
    prompt_type_hash = xmlHashCreate(31);
    int n;
    for(n=0; static_prompt_types[n].name != NULL; n++){
        xmlHashAddEntry( prompt_type_hash, static_prompt_types[n].name,
           (void*) &static_prompt_types[n]);
    }        

}


/*
 *  fetch_options
 *
 *
 *  options[] as in select options or radio button labels
 *  as a null termintated array of strings.
 *
 */
char**  fetch_options(
    struct handler_args* h, 
    struct pgtype_datum* pgtype, 
    struct prompt_rule* p_rule, 
    char* fname){

    static char* boolean_options = "{yes,no}";
    char** new_options = NULL;

    if ((p_rule != NULL) && (p_rule->options != NULL) && (p_rule->options[0] != '\0')){
        // Use the option list specified.
        new_options = parse_pg_array(p_rule->options);

        fprintf(h->log, "%f %d %s:%d prompt rule options %s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            p_rule->options);
 

    }else if ((pgtype != NULL) && 
              (pgtype->enum_labels!= NULL) &&
              (pgtype->enum_labels[0] != '\0')){

        new_options = parse_pg_array(pgtype->enum_labels);
        
        fprintf(h->log, "%f %d %s:%d pgtype enum %s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            pgtype->enum_labels);

    }else if ((pgtype != NULL) && (pgtype->is_boolean)){  
        new_options = parse_pg_array(boolean_options);

        fprintf(h->log, "%f %d %s:%d boolean %s\n", 
            gettime(), h->request_id, __func__, __LINE__,
            boolean_options);

    }else if ((pgtype != NULL) && (pgtype->has_fkey) && (p_rule != NULL)){
        if (strcmp("select_fkey", p_rule->prompt_type) == 0){
            new_options = foreign_key_list(h, pgtype);
        
            fprintf(h->log, "%f %d %s:%d fkey %s\n", 
                gettime(), h->request_id, __func__, __LINE__,
                    ((new_options != NULL) && (new_options[0] != '\0')) ?
                        "OK":"foreign_key_list fail");
        }
    }

    return new_options;
}


#ifdef PROMPT_RULE_MAIN

void print_prompt_rule(struct prompt_rule* rule, double elapsed){

    if (rule==NULL){
        printf("rule is null\n");
        return;
    }

    printf("form_name=%s\n", rule->form_name);
    printf("fieldname=%s\n", rule->fieldname);
    printf("prompt_type=%s\n", rule->prompt_type);
    printf("el_class=%s\n", rule->el_class);
    printf("readonly=%c\n", (rule->readonly) ? 't':'f');
    printf("rows=%d\n", rule->rows);
    printf("cols=%d\n", rule->cols);
    printf("size=%d\n", rule->size);
    printf("result %s null\n", (rule->result==NULL) ? "is":"is not");
    printf("elapsed time %f\n", elapsed);
    printf("---------------------\n");

    return; 
}

int main(int argc, char* argv[]){


    // setup a fake environment

    qzrandom64_init();

    struct handler_args hargs = (struct handler_args){
       .request_id = 1,
    };
    struct handler_args* h = &hargs;
    hargs.log = stdout;

    struct session s;
    hargs.session = &s;
    
    hargs.conf = init_config();

    h->doc = doc_from_file(h, "base.xml");

    xmlNodePtr qzdiv = qzGetElementByID(h, "qz");
    
    const char* kw[] = { "host", "dbname", "user", "password", 
        "application_name", NULL };

    char* vals[] = { "localhost", "info", "qz", "42", "qztest", NULL };

    h->session->conn = PQconnectdbParams(kw, (const char* const*) vals, 0);

    if (PQstatus(h->session->conn) != CONNECTION_OK){
       fprintf(h->log, "bad connect\n");
       exit(36);
    }
    h->session->opentables = xmlHashCreate(50); 
    h->session->pgtype_datum = xmlHashCreate(50);
    h->session->form_tags = xmlHashCreate(50);
    h->request_id = 42;

    init_prompt_type_hash();
    init_open_table(h);
    struct table_action* t_action = open_table(h, "todo", "getone");
    
    char* data[] = {"26",NULL};
    perform_action(h, t_action, data);

    char* fields[] = { "uid", "seq", "created", "last_mod", "priority", 
        "summary", "status", "completed", "percent", "description", "location", 
        "class", "url", "notfound", NULL};

    double start, finish;

    struct prompt_rule* rule;

    int j;
    for( j=0; fields[j] != NULL; j++){
        printf("field is %s\n", fields[j]); 
        start = gettime();

        rule = fetch_prompt_rule(h, "todo", fields[j]);

         add_prompt(h, t_action, rule, NULL /*type*/,
              NULL /*options*/, 0 /*row*/,
              qzdiv, fields[j], NULL);
        
        finish = gettime();
        if (rule != NULL){
            print_prompt_rule(rule, finish-start); 
        }else{
            printf("null result in %f\n", finish-start);
        }    

        free_prompt_rule(h, rule);
        
    }

    xmlBufferPtr xbuf = xmlBufferCreate();
    xmlSaveCtxtPtr ctxt = xmlSaveToBuffer(xbuf, "UTF-8", 
             XML_SAVE_FORMAT|XML_SAVE_NO_DECL|XML_SAVE_AS_HTML);
    
    xmlSaveDoc(ctxt, h->doc);
    xmlSaveClose(ctxt);
    printf("%s", xbuf->content);


    rule = fetch_prompt_rule(h, "stuff", "xyz");
    printf("\njson_add_element_args empty rule\n%s\n", 
        json_add_element_args("stupid_test", rule, NULL));
    
    rule = fetch_prompt_rule(h, "stuff", "ar");
    printf("json_add_element_args\n%s\n", 
        json_add_element_args("stupid_test", rule, NULL));

    rule = fetch_prompt_rule(h, "test", "test");
    printf("json_add_element_args\n%s\n",
        json_add_element_args("stupid_test", rule, NULL));

    char* qpct[] = {"normal(x)", "single(%n)", "else(%x)", 
        "more(%n,%n,%n,%n)", NULL};

    for(j=0; qpct[j] != NULL; j++){
        char* q = quantify_percent_n(qpct[j], j);
        printf("quantify_percent_n(%s)=%s\n", qpct[j], q);
        free(q);
    }    
    char* q = quantify_percent_n("big(%n)", 1234);
    printf("quantify_percent_n(%s)=%s\n", "big(%n)", q);
    free(q);
    
    struct prompt_rule* defr = fetch_prompt_rule(h, "gluxton", "noctal");
    printf("default_prompt_rule(%s,%s) prompt_type:%s\n",
       defr->form_name, defr->fieldname, defr->prompt_type);
    free(defr);

    return 0;
}


#endif
