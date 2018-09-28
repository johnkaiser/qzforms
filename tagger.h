#define ETAG_MAX_LENGTH 98

extern pid_t tagger_init(struct qz_config* conf, char* argv[]);

extern void make_etag(char* tagbuf, char* sockname, uint64_t domain_token,
    unsigned char payload[16]);

extern void validate_etag(char* payload, char* sockname,
    uint64_t domain_token, char* etag);
