
#include <sys/types.h>
#include <sys/param.h>
#include <stdbool.h>

#define DEFAULT_CONFIG_FILE  "config/qzforms.conf"
#define DEFAULT_TAGGER_SOCKET_PATH  "run/qzfcgi.sock"
#define DEFAULT_LOGFILE_NAME "logs/qzfcgi.log"
#define DEFAULT_SESSION_INACTIVITY_TIMEOUT 600
#define DEFAULT_TEMPLATE_PATH "templates"
#define DEFAULT_FORM_DURATION 240
#define DEFAULT_NUMBER_OF_THREADS 10
#define DEFAULT_HOUSEKEEPER_NAP_TIME 120
#define DEFAULT_AUDIT_FORM_SET_REF_COUNT false
#define DEFAULT_MAX_LOG_FILE_SIZE 10000000
#define DEFAULT_MAX_LOG_FILE_COUNT 9

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
    char template_path[MAXPATHLEN+2];
    unsigned int session_inactivity_timeout;
    int number_of_threads;
    unsigned int form_duration;
    unsigned int housekeeper_nap_time;
    bool audit_form_set_ref_count;
    uint64_t max_log_file_size;
    uint8_t  max_log_file_count;
    uint64_t integrity_token;
};


extern struct qz_config* init_config(void);

