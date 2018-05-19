
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
 *  parse_config
 *
 *  Turn a text buffer into a hash table of keys and values.
 *  Set parse_error to a static string on errors.
 *
 */
char* parse_error = NULL;
int error_line = 0;

xmlHashTablePtr parse_config(char* filebuf){

    /*
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------+
     *  |state\ch        |isalpha _ |isdigit     |isspace        | #         | =              | \r  or \n      | other     |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |looking_for_key |in_key    |error 1     |no action      |in_comment |error           |no action       |error 1    |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |in_key          |no action |error 1     |looking_for_eq |error      |looking_for_val |error           |error 1    |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |in_comment      |no action |no action   |no action      |no action  |no action       |looking_for_key |no action  |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |looking_for_eq  |error     |error       |no action      |error      |looking_for_val |error           |error      |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |looking_for_val |in_val    |in_val      |no action      |error      |error           |error           |in_val 2   |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |in_val          |no action |no action   |no action      |no action  |no action       |looking_for_key |no action 2|
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------|
     *  |error           |error     |error       |error          |error      |error           |error           |error      |
     *  +----------------+----------+------------+---------------+-----------+----------------+----------------+-----------+
     *
     *    1.  Keys are always 7bit ASCII alpha characters. 
     *    2.  Values can contain symbols, punctuation, UTF8, etc.
     *
     */

    enum {looking_for_key, in_key, in_comment, looking_for_eq, looking_for_val, in_val, error} searchstate;
    searchstate = looking_for_key;

    char* akey = NULL;
    char* ch;
    int line_nbr = 1;
    // There are about 30 things that can be set, but it is not an important value. 
    xmlHashTablePtr pt = xmlHashCreate(30);

    static char* key_with_start_digit = "Key can not start with a digit";
    static char* key_with_start_eq = "Key can not start with =";
    static char* key_with_bad_char = "Key must be 7bit ASCII alpha characters";
    static char* eq_not_found = "= not found";
    static char* hash_add_failed = "xmlHashAddEntry failed";
    static char* bad_value = "Bad value";

    for (ch = filebuf; *ch != '\0'; ch++){
        if (*ch == '\n') line_nbr++;

        switch(searchstate){
            case looking_for_key: 
               if (isalpha(*ch) || (*ch == '_')) {
                  akey = ch;
                  searchstate = in_key;
               }else if (*ch == '#'){
                  searchstate = in_comment;
               }else if (isspace(*ch) || (*ch == '\r') || (*ch == '\n')){
                  ; // no op
               }else if (isdigit(*ch)) {
                   parse_error = key_with_start_digit;
                   error_line = line_nbr;
                   searchstate = error;
               }else if (*ch == '=')  {
                   parse_error = key_with_start_eq;
                   error_line = line_nbr;
                   searchstate = error;
               }else {
                   parse_error = key_with_bad_char;
                   error_line = line_nbr;
                   searchstate = error;
               }
               break;

            case in_key: 
                if (isspace(*ch)){
                    *ch = '\0';
                    searchstate = looking_for_eq;
                }else if (*ch == '='){
                    *ch = '\0';
                    searchstate = looking_for_val;
                }else if (isalpha(*ch) || (*ch == '_')){
                    ; // no op
                }else {
                     parse_error = key_with_bad_char;
                     error_line = line_nbr;
                     searchstate = error;
                }
                break;

            case in_comment: 
                if ((*ch == '\r') || (*ch == '\n')){
                    searchstate = looking_for_key;
                }
                break;

            case looking_for_eq: 
                if (*ch == '='){
                    searchstate = looking_for_val;
                }else if (isspace(*ch)){
                    ; // no op
                }else{    
                    parse_error = eq_not_found;
                    error_line = line_nbr;
                }
                break;

            case looking_for_val: 
                 if (isspace(*ch)){
                     ; // no op
                 }else if ((*ch == '#')||(*ch == '=')||(*ch == '\r')||(*ch == '\n')){
                     parse_error = bad_value;
                     error_line = line_nbr;
                     searchstate = error;
                 }else{ 
                     // other cases, alpha, numeric, _, UTF8   
                     searchstate = in_val;
                     
                     // XXXXXXXXXXXXXXXXX
                     // check if akey is in hash table and
                     // error out hard if it is,
                     // or what does AddEntry do in on a dupe key? 
                     if ( xmlHashAddEntry( pt, akey, ch ) == -1 ){
                        parse_error = hash_add_failed;
                        error_line = line_nbr;
                        searchstate = error;
                     }
                 }
                 break;

            case in_val: 
                if ((*ch == '\r')||(*ch == '\n')){
                    *ch = '\0';
                    searchstate = looking_for_key;
                    // Work backwards now turning spaces to nulls, 
                    char* trim;
                    for(trim=ch-1; isspace(*trim); trim--){
                        *trim = '\0';
                    }    
                }
                break;

            case error:
                break;
        }
    }

    if (searchstate == error){
        // Leaving hash table allocated.
        // It's OK, the program is crashing anyway.
        return NULL;
    }else{
        return pt;
    }    
}

/*
 *  is_true
 *
 *  turn a string into a boolean
 */
bool is_true(char* str){

    if (str != NULL){

        if (strncasecmp("TRUE", str, 4) == 0) return true;
        if (strncasecmp("yes", str, 4) == 0) return true;
        if (strncasecmp("on", str, 4) == 0) return true;
        if (strlen(str) == 1){
            if ((str[0] == 't') || (str[0] == 'T')) return true;
            if ((str[0] == 'y') || (str[0] == 'Y')) return true;
            if (str[0] == '1') return true;
        }

        return false;
    }else{
        return false;
    }
}
/*
 *  set_config
 *
 *  Set the attributes of conf from the values in conf_hash
 *  respecting default values.
 */
void set_config(struct qz_config* conf, xmlHashTablePtr conf_hash){

    qzrandom64_init();

    if (conf == NULL){
       fprintf(stderr, "set_config given null conf\n");   
       exit(31);
    }

    // Set the defaults first
    snprintf(conf->tagger_socket_path, MAX_SOCKET_NAME_LEN, "%s", 
        DEFAULT_TAGGER_SOCKET_PATH);

    conf->number_of_users =  DEFAULT_NUMBER_OF_USERS;
    conf->server_token[0] = '\0';
    conf->server_key[0] = '\0';
    conf->session_inactivity_timeout = DEFAULT_SESSION_INACTIVITY_TIMEOUT;
    conf->number_of_threads = DEFAULT_NUMBER_OF_THREADS;
    conf->form_duration = DEFAULT_FORM_DURATION;
    conf->housekeeper_nap_time = DEFAULT_HOUSEKEEPER_NAP_TIME;
    conf->max_log_file_size = DEFAULT_MAX_LOG_FILE_SIZE;
    conf->max_log_file_count = DEFAULT_MAX_LOG_FILE_COUNT;
    conf->failed_login_block_timeout = DEFAULT_FAILED_LOGIN_BLOCK_TIMEOUT;
    conf->max_failed_logins = DEFAULT_MAX_FAILED_LOGINS;
    conf->audit_form_set_ref_count = DEFAULT_AUDIT_FORM_SET_REF_COUNT;
    conf->log_id_index_details = DEFAULT_LOG_ID_INDEX_DETAILS;
    conf->log_table_action_details = DEFAULT_LOG_TABLE_ACTION_DETAILS;
    conf->log_form_tag_details = DEFAULT_LOG_FORM_TAG_DETAILS;
    conf->log_form_set_details = DEFAULT_LOG_FORM_SET_DETAILS;
    conf->log_login_tracker_details = DEFAULT_LOG_LOGIN_TRACKER_DETAILS;
    conf->log_validate_rule_details = DEFAULT_LOG_VALIDATE_RULE_DETAILS;

    snprintf(conf->logfile_name, MAXPATHLEN, "%s", DEFAULT_LOGFILE_NAME);
    snprintf(conf->template_path, MAXPATHLEN, "%s", DEFAULT_TEMPLATE_PATH);

    char* setting;

    setting = xmlHashLookup(conf_hash, "TAGGER_SOCKET");
    // Many settings were originally named with the prefix QZ_
    // Allow them for backwards compatibility.
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_TAGGER_SOCKET");
    if (setting == NULL) setting = getenv("TAGGER_SOCKET");
    if ((setting != NULL) && (strlen(setting) > 0)){
        snprintf(conf->tagger_socket_path, MAX_SOCKET_NAME_LEN, "%s", setting);
    }

    setting = xmlHashLookup(conf_hash, "NUMBER_OF_USERS");
    printf("NUMBER_OF_USERS=%s\n", setting);
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_NUMBER_OF_USERS");
    if (setting == NULL) setting = getenv("NUMBER_OF_USERS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int nbr_users = (unsigned int) strtol(setting, NULL, 10);

        conf->number_of_users = nbr_users;
    } 
    
    setting = xmlHashLookup(conf_hash, "LOG_FILENAME"); 
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_LOG_FILENAME");
    if (setting == NULL) setting = getenv("LOG_FILENAME");
    if ((setting != NULL) && (strlen(setting) > 0)){
        snprintf(conf->logfile_name, MAXPATHLEN, "%s", setting);
    }
    
    setting = xmlHashLookup(conf_hash, "TEMPLATE_PATH");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_TEMPLATE_PATH");
    if (setting == NULL) setting = getenv("TEMPLATE_PATH");
    snprintf(conf->template_path, MAXPATHLEN, "%s", 
        ((setting != NULL) && (strlen(setting) > 0)) ?  
            setting:DEFAULT_TEMPLATE_PATH );

    // SERVER_TOKEN and SERVER_KEY can not be set from environment
    // variables. That would be an unmiticated security disaster.
    setting = xmlHashLookup(conf_hash, "SERVER_TOKEN");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_SERVER_TOKEN");
    if ((setting != NULL) && (strlen(setting) > 0)){
        snprintf(conf->server_token, SERVER_TOKEN_HEX_LENGTH+1, "%s", setting);
    }
    
    setting = xmlHashLookup(conf_hash, "SERVER_KEY");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_SERVER_KEY");
    if ((setting != NULL) && (strlen(setting) > 0)){
        snprintf(conf->server_key, SERVER_KEY_HEX_LENGTH+1, "%s", setting);
    }

    setting = xmlHashLookup(conf_hash, "SESSION_INACTIVITY_TIMEOUT");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, 
        "QZ_SESSION_INACTIVITY_TIMEOUT");

    if (setting == NULL) setting = getenv("SESSION_INACTIVITY_TIMEOUT");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int inactivity_timeout = (unsigned int) 
            strtol(setting, NULL, 10);
        conf->session_inactivity_timeout = inactivity_timeout;
    } 

    setting = xmlHashLookup(conf_hash, "FORM_DURATION");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, "QZ_FORM_DURATION");
    if (setting == NULL) setting = getenv("FORM_DURATION");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int form_duration = (unsigned int) 
            strtol(setting, NULL, 10);
        conf->form_duration = form_duration;
    }

    setting = xmlHashLookup(conf_hash, "HOUSEKEEPER_NAP_TIME");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, 
        "QZ_HOUSEKEEPER_NAP_TIME");

    if (setting == NULL) setting = getenv("HOUSEKEEPER_NAP_TIME");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int housekeeper_nap_time = (unsigned int)
            strtol(setting, NULL, 10);
        conf->housekeeper_nap_time = housekeeper_nap_time;
    }    

    setting = xmlHashLookup(conf_hash, "AUDIT_FORM_SET_REF_COUNT");
    if (setting == NULL) setting = getenv("AUDIT_FORM_SET_REF_COUNT");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->audit_form_set_ref_count = is_true(setting);
    }

    setting = xmlHashLookup(conf_hash, "NUMBER_OF_THREADS");
    if (setting == NULL) setting = xmlHashLookup(conf_hash, 
        "QZ_NUMBER_OF_THREADS");

    if (setting == NULL) setting = getenv("NUMBER_OF_THREADS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int number_of_threads = (unsigned int)
            strtol(setting, NULL, 10);
        conf->number_of_threads = number_of_threads;
    }

    setting = xmlHashLookup(conf_hash, "MAX_LOG_FILE_SIZE");
    if (setting == NULL) setting = getenv("MAX_LOG_FILE_SIZE");
    if ((setting != NULL) && (strlen(setting) > 0)){
        uint64_t max_log_file_size = (uint64_t)
            strtoull(setting, NULL, 10);
        conf->max_log_file_size = max_log_file_size;
    }

    setting = xmlHashLookup(conf_hash, "MAX_LOG_FILE_COUNT");
    if (setting == NULL) setting = getenv("MAX_LOG_FILE_COUNT");
    if ((setting != NULL) && (strlen(setting) > 0)){
        uint8_t max_log_file_count = (uint8_t)
            strtoull(setting, NULL, 10);
            conf->max_log_file_count = max_log_file_count;
    }

    setting = xmlHashLookup(conf_hash, "LOG_ID_INDEX_DETAILS");
    if (setting == NULL) setting = getenv("LOG_ID_INDEX_DETAILS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->log_id_index_details = is_true(setting);
    }

    setting = xmlHashLookup(conf_hash, "LOG_TABLE_ACTION_DETAILS");
    if (setting == NULL) setting = getenv("LOG_TABLE_ACTION_DETAILS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->log_table_action_details = is_true(setting);
    }
    
    setting = xmlHashLookup(conf_hash, "LOG_FORM_TAG_DETAILS");
    if (setting == NULL) setting = getenv("LOG_FORM_TAG_DETAILS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->log_form_tag_details = is_true(setting);
    }

    setting = xmlHashLookup(conf_hash, "LOG_FORM_SET_DETAILS");
    if (setting == NULL) setting = getenv("LOG_FORM_SET_DETAILS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->log_form_set_details = is_true(setting);
    }

    setting = xmlHashLookup(conf_hash, "LOG_VALIDATE_RULE_DETAILS");
    if (setting == NULL) setting = getenv("LOG_VALIDATE_RULE_DETAILS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->log_validate_rule_details = is_true(setting);
    }

    setting = xmlHashLookup(conf_hash, "FAILED_LOGIN_BLOCK_TIMEOUT");
    if (setting == NULL) setting = getenv("FAILED_LOGIN_BLOCK_TIMEOUT");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int block_timeout = (unsigned int)
            strtol(setting, NULL, 10);
        conf->failed_login_block_timeout = block_timeout;
    }

    setting = xmlHashLookup(conf_hash, "MAX_FAILED_LOGINS");
    if (setting == NULL) setting = getenv("MAX_FAILED_LOGINS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        unsigned int failed_logins = (unsigned int)
            strtol(setting, NULL, 10);
        conf->max_failed_logins = failed_logins;
    }

    setting = xmlHashLookup(conf_hash, "LOG_LOGIN_TRACKER_DETAILS");
    if (setting == NULL) setting = getenv("LOG_LOGIN_TRACKER_DETAILS");
    if ((setting != NULL) && (strlen(setting) > 0)){
        conf->log_login_tracker_details = is_true(setting);
    }

    // Put postgres vars into environment.
    setenv("PGAPPNAME", "qzforms", 0);

    char* allowed_vars[] = {
        "PGAPPNAME",
        "PGCLIENTENCODING",
        "PGCONNECT_TIMEOUT",
        "PGDATABASE",
        "PGDATASTYLE",
        "PGGEQO",
        "PGGSSLIB",
        "PGHOST",
        "PGHOSTADDR",
        "PGOPTIONS",
        "PGPASSFILE",
        "PGPORT",
        "PGREALM",
        "PGREQUIREPEER",
        "PGREQUIRESSL",
        "PGSERVICE",
        "PGSERVICEFILE",
        "PGSSLCERT",
        "PGSSLCRL",
        "PGSSLKEY",
        "PGSSLMODE",
        "PGSSLROOTCERT",
        "PGTZ",
        NULL
    };    
    char* var;
    int n = 0;
    for (var=allowed_vars[n]; var!=NULL; var=allowed_vars[++n]){
        setting = xmlHashLookup(conf_hash, var);
        if ((setting != NULL) && (strlen(setting) > 0)){
            setenv(var, setting, 1);
        }
    }

    char* PGUSER = getenv("PGUSER");
    char* PGPASSWORD = getenv("PGPASSWORD");

    if ( (PGUSER != NULL) || (PGPASSWORD != NULL) ){
        fprintf(stderr, "Neither PGUSER nor  PGPASSWORD "
            "environment variables may be set\n");

        exit(54);
    }

    conf->integrity_token = qzrandom64();
}

/*
 *  init_config
 * 
 *  Open and load the configuration file.
 */  
struct qz_config* init_config(void){

    char* config_file = getenv("QZ_CONFIG_FILENAME");
    
    if (config_file == NULL){
        config_file = DEFAULT_CONFIG_FILE;
    }

    struct stat sb;
    char* config_file_buf = NULL;
    xmlHashTablePtr conf_hash = NULL;

    struct qz_config* conf = calloc(1, sizeof(struct qz_config));

    if (strlen(config_file) > 0){
        if (stat(config_file, &sb) == 0){ 
            int conf_fd = -1;
    
            conf_fd = open(config_file, O_RDONLY, 0);
            if (conf_fd > 0){
                ssize_t bytesread;
                config_file_buf = malloc(sb.st_size+1);
                bytesread = read(conf_fd, config_file_buf, sb.st_size);
                if (bytesread == sb.st_size){
                    config_file_buf[bytesread] = '\0';
    
                }else{
                    // bytes read != file size.
                    perror("Short read on config file");
                    exit(33);
                }    
                conf_hash = parse_config(config_file_buf);

                if (conf_hash == NULL){
                    fprintf(stderr, "Parsing config file %s failed near line %d, %s\n",
                        config_file, error_line, parse_error);

                    exit (32);
                }


            }else{
                perror("open on config file failed");
                exit(34);
            }    
        }else{
            // stat on config file name has failed.
            perror("No configuration file");
            exit(35);
        }
    } 


    set_config(conf, conf_hash);
    free(config_file_buf);

    return conf;
}


#ifdef QZCONFIG_TEST

int main(int argc, char* argv[], char* env[]){

    struct qz_config* config;
    char* configfilename;
    int ch;

    double starttime = gettime();

    while ((ch = getopt(argc, argv, "l:f:")) != -1) {

        switch (ch){
            case 'l':
                setenv("QZ_LOG_FILENAME", optarg, true);
                break;
            case 'f':
                configfilename = optarg;
                setenv("QZ_CONFIG_FILENAME", optarg, true);
                break;
        }
    }
    printf("begin init_config\n");
    config = init_config();
    printf("fin init_config\n\n");

    printf("tagger_socket_path=%s\n", config->tagger_socket_path);
    printf("number_of_users=%d\n", config->number_of_users);
    printf("number_of_threads=%d\n", config->number_of_threads);
    printf("server_token=%s\n", config->server_token);
    printf("server_key=%s\n", config->server_key);
    printf("template_path=%s\n", config->template_path);
    printf("logfile_name=%s\n", config->logfile_name);
    printf("session_inactivity_timeout=%d\n", config->session_inactivity_timeout);
    printf("form_duration=%d\n", config->form_duration);
    printf("housekeeper_nap_time=%d\n", config->housekeeper_nap_time);
    printf("integrity_token=%"PRIx64"\n", config->integrity_token);
    printf("audit_form_set_ref_count=%c\n", (config->audit_form_set_ref_count) ? 't':'f');
    printf("max_log_file_size=%"PRIu64"\n", config->max_log_file_size);
    printf("max_log_file_count=%d\n",  config->max_log_file_count);
    printf("log_id_index_details=%c\n", (config->log_id_index_details) ? 't':'f');
    printf("log_table_action_details=%c\n", (config->log_table_action_details) ? 't':'f'); 
    printf("log_form_tag_details=%c\n", (config->log_form_tag_details) ? 't':'f'); 
    printf("log_form_set_details=%c\n", (config->log_form_set_details) ? 't':'f'); 
    printf("log_validate_rule_details=%c\n", (config->log_validate_rule_details) ? 't':'f');

    
    char* allowed_vars[] = {
        "PGAPPNAME",
        "PGCLIENTENCODING",
        "PGCONNECT_TIMEOUT",
        "PGDATABASE",
        "PGDATASTYLE",
        "PGGEQO",
        "PGGSSLIB",
        "PGHOST",
        "PGHOSTADDR",
        "PGOPTIONS",
        "PGPASSFILE",
        "PGPORT",
        "PGREALM",
        "PGREQUIREPEER",
        "PGREQUIRESSL",
        "PGSERVICE",
        "PGSERVICEFILE",
        "PGSSLCERT",
        "PGSSLCRL",
        "PGSSLKEY",
        "PGSSLMODE",
        "PGSSLROOTCERT",
        "PGTZ",
        NULL
    }; 
    char* var;
    char* val;
    int n = 0;
    printf("\nChecking Environment\n");
    for (var=allowed_vars[n]; var!=NULL; var=allowed_vars[++n]){
        if ((val = getenv(var)) != NULL){
            printf("%s=\"%s\"\n", var, val);
        }
    }

    double fintime = gettime();
    printf("\nexecute time = %f\n", fintime - starttime);
    return(0);
}

#endif
