
#include <sys/types.h>
#include <sys/param.h>
#include <stdbool.h>
#include <inttypes.h>

/*
 *  To add a new config option:
 *
 *  1. define a default value in qzconfig.h
 *  2. add a field to struct qzconfig before integrity_token in qzconfig.h
 *  3. set the field to its default at the top of set_config in qzconfig.c
 *  3. add a setting block to function set_config in qzconfig.c
 *  4. add a test under QZCONFIG_MAIN near the end of qzconfig.c
 *  5. make qzconfig_test
 *  6. export QZ_CONFIG_FILENAME=some/path/x.conf
 *  7. ./qzconfig_test
 */
 
#define DEFAULT_CONFIG_FILE  "config/qzforms.conf"
#define DEFAULT_TAGGER_SOCKET_PATH  "run/qzfcgi.sock"
#define DEFAULT_NUMBER_OF_USERS (42)
#define DEFAULT_LOGFILE_NAME "logs/qzfcgi.log"
#define DEFAULT_STDERR_FILE ""
#define DEFAULT_SESSION_INACTIVITY_TIMEOUT (600)
#define DEFAULT_TEMPLATE_PATH "templates"
#define DEFAULT_FORM_DURATION (180)
#define DEFAULT_NUMBER_OF_THREADS (10)
#define DEFAULT_HOUSEKEEPER_NAP_TIME (120)
#define DEFAULT_AUDIT_FORM_SET_REF_COUNT false
#define DEFAULT_MAX_LOG_FILE_SIZE (1000000)
#define DEFAULT_MAX_LOG_FILE_COUNT (8)
#define DEFAULT_LOG_ID_INDEX_DETAILS false
#define DEFAULT_LOG_TABLE_ACTION_DETAILS false
#define DEFAULT_LOG_FORM_TAG_DETAILS false
#define DEFAULT_LOG_FORM_SET_DETAILS false
#define DEFAULT_FAILED_LOGIN_BLOCK_TIMEOUT (300)
#define DEFAULT_MAX_FAILED_LOGINS (20)
#define DEFAULT_LOG_LOGIN_TRACKER_DETAILS false
#define DEFAULT_LOG_VALIDATE_RULE_DETAILS false
#define DEFAULT_LOG_FS_ETAG_DETAILS false
#define DEFAULT_LOG_TAGGER_DETAILS false
#define DEFAULT_LOG_PROMPT_RULE_DETAILS false
#define DEFAULT_LOG_CONTEXT_PARAMETER_DETAILS false
#define DEFAULT_LOG_CALLBACK_DETAILS false
#define DEFAULT_LOG_COOKIE_DETAILS false
#define DEFAULT_LOG_DOC_DETAILS false
#define DEFAULT_LOG_POST_DATA false

// 104 from un.h max socket length
#define MAX_SOCKET_NAME_LEN 104

#define SERVER_TOKEN_HEX_LENGTH (16)
#define SERVER_KEY_HEX_LENGTH (32)


struct qz_config {
    char tagger_socket_path[MAX_SOCKET_NAME_LEN+2];
    unsigned int number_of_users;
    char server_token[SERVER_TOKEN_HEX_LENGTH+2];
    char server_key[SERVER_KEY_HEX_LENGTH+2];
    char logfile_name[MAXPATHLEN+2];
    char stderr_file[MAXPATHLEN+2];
    char template_path[MAXPATHLEN+2];
    unsigned int session_inactivity_timeout;
    int number_of_threads;
    unsigned int form_duration;
    unsigned int housekeeper_nap_time;
    uint64_t max_log_file_size;
    uint8_t  max_log_file_count;
    unsigned int failed_login_block_timeout;
    unsigned int max_failed_logins;
    bool audit_form_set_ref_count;
    bool log_id_index_details;
    bool log_table_action_details;
    bool log_form_tag_details;
    bool log_form_set_details;
    bool log_login_tracker_details;
    bool log_validate_rule_details;
    bool log_fs_etag_details;
    bool log_tagger_details;
    bool log_prompt_rule_details;
    bool log_context_parameter_details;
    bool log_callback_details;
    bool log_cookie_details;
    bool log_doc_details;
    bool log_post_data;
    uint64_t integrity_token;
};


extern struct qz_config* init_config(void);

