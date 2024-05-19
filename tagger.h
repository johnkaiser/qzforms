#include "qzconfig.h"
#include "crypto_etag.h"

#define ETAG_MAX_LENGTH 98

/*
 * Call this first.
 */
extern pid_t tagger_init(struct qz_config* conf, char* argv[]);

/*
 * Call this to make an etag, result in tagbuf.
 * Make tagbuf at least ETAG_MAX_LENGTH which includes 1 ending null char.
 */
extern void make_etag(char* tagbuf, char* sockname, uint64_t domain_token,
    char payload[16]);

/*
 * Call this to get the 128 bits of payload from an etag
 */
extern void validate_etag(char payload[16], char* sockname,
    uint64_t domain_token, char* etag);
