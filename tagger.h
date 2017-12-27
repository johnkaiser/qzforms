#define ETAG_MAX_LENGTH 65

extern pid_t tagger_init(struct qz_config* conf, char* argv[]);

extern void make_etag(char* tagbuf, char* sockname, uint64_t payload);

extern uint64_t validate_etag(char* sockname, char* etag);
