
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

#include "crypto_etag.h"
#include "qzconfig.h"
#include "qzrandom64.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <openssl/blowfish.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
extern double gettime(void);
static int request_id;

#define TAGBUF 1024

#define DEBUG if (debug) fprintf
#define FLUSH if (debug) fflush(log)

static bool debug = false;

/*  tagger_serve
 *
 *  Setup a server to fork, read a socket
 *  and reply with a tag or data.
 *  Call tagger_init instead of this.
 */

void tagger_serve(struct qz_config* conf, bool debug){
    FILE* log = NULL;
    if (debug) log = fopen(conf->logfile_name, "a");

    DEBUG(log, "%f %d %s:%d begin tagger_serve\n",
        gettime(), request_id, __func__, __LINE__);

    DEBUG(log, "%f %d %s:%d qzrandom64 %"PRIx64"\n",
        gettime(), request_id, __func__, __LINE__, qzrandom64());

    DEBUG(log, "%f %d %s:%d strlen(conf->server_token)=%zu\n",
        gettime(), request_id, __func__, __LINE__, strlen(conf->server_token));

    FLUSH;

    // Get the server token from config or make one up.
    uint64_t server_token = 0;

    if (strlen(conf->server_token) > 0){

        if (strlen(conf->server_token) == 16){
            server_token = strtoul(conf->server_token, NULL, 16);
        }
    }else{
        server_token = qzrandom64();
    }
    DEBUG(log, "%f %d %s:%d server_token=%"PRIx64"\n",
        gettime(), request_id, __func__, __LINE__, server_token);

    FLUSH;

    if (server_token == 0){
        FILE* qzlog = fopen( conf->logfile_name, "a");

        fprintf(qzlog, "%f %d %s:%d fail - bad server token in config %s len=%zu\n",
            gettime(), request_id, __func__, __LINE__,
            conf->server_token, strlen(conf->server_token));

        exit(21);
    }


    // Get the server key from config or make one up.
    uint64_t rnbr[2];
    rnbr[0] = 0;
    rnbr[1] = 0;
    unsigned char server_key[16];

    if (strlen(conf->server_key) > 0){
        if (strlen(conf->server_key) == 32){
            char keybuf[17];
            keybuf[16] = '\0';

            memcpy(keybuf, conf->server_key, 16);
            rnbr[0] = strtoull(keybuf, NULL, 16);

            memcpy(keybuf, conf->server_key+16, 16);
            rnbr[1] = strtoull(keybuf, NULL, 16);
       }
    }else{
        rnbr[0] = qzrandom64();
        rnbr[1] = qzrandom64();
    }
    memcpy(server_key, rnbr, 16);

    DEBUG(log, "%f %d %s:%d server_key=%"PRIx64"%"PRIx64"\n",
        gettime(), request_id, __func__, __LINE__, rnbr[0], rnbr[1]);

    FLUSH;
    // setup key

    // setup socket
    mode_t old_umask = umask(077);

    struct sockaddr_un addr;
    int sock;
    int sock_len = 0;

    if ((sock= socket(AF_UNIX, SOCK_STREAM, 0)) == -1){

        FILE* qzlog = fopen( conf->logfile_name, "a");

        fprintf(qzlog, "%f %d %s:%d tagger server socket failed\n",
            gettime(), request_id, __func__, __LINE__);

        fflush(qzlog);
        exit(22);
    }

    int max_path_len = sizeof(addr.sun_path);
    if (strlen(conf->tagger_socket_path) > (max_path_len-1)){

        FILE* qzlog = fopen( conf->logfile_name, "a");

        fprintf(qzlog, "%f %d %s:%d tagger socket path length exceeded\n",
            gettime(), request_id, __func__, __LINE__);

        fflush(qzlog);
        exit(23);
    }

    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, max_path_len, "%s", conf->tagger_socket_path);

#ifdef HAVE_SOCKADDR_UN_SUN_LEN
    addr.sun_len =  SUN_LEN(&addr);
    sock_len = addr.sun_len;
#else
    sock_len = sizeof(addr) - max_path_len + strlen(addr.sun_path);
#endif

    if (bind(sock, (struct sockaddr*) &addr, sock_len) != 0){
        perror("tagger server socket bind");
        exit(24);
    }

    if (listen(sock, 16) != 0){
        perror("tagger server socket listen");
        exit(25);
    }
    umask(old_umask);

    int incoming;
    int bytesread;
    char inbuf[TAGBUF];
    unsigned char* output;
    uint64_t payload;
    struct cryptotag ctag;

    DEBUG(log, "%f %d %s:%d server ready\n",
        gettime(), request_id, __func__, __LINE__);

    if (debug) fclose(log);
    if (debug) log = fopen(conf->logfile_name, "a");

    for(;;){
        bzero(inbuf, TAGBUF);
        output = NULL;

        incoming = accept(sock, (struct sockaddr*) &addr, &sock_len);
        if (incoming == -1){

            FILE* qzlog = fopen( conf->logfile_name, "a");

            fprintf(qzlog, "%f %d %s:%d tagger server accept failed\n",
                gettime(), request_id, __func__, __LINE__);

            fclose(qzlog);
            continue;
        }
        DEBUG(log, "%f %d %s:%d accept socket %d\n",
            gettime(), request_id, __func__, __LINE__, incoming);

        bytesread = read(incoming, inbuf, TAGBUF-1);

        DEBUG(log, "%f %d %s:%d server read %d bytes\n",
            gettime(), request_id, __func__, __LINE__, bytesread);

        switch (bytesread){

            case sizeof(struct cryptotag):
                memcpy(&ctag, inbuf, sizeof(struct cryptotag));

                DEBUG(log, "%f %d %s:%d make_crypto_etag \n",
                    gettime(), request_id, __func__, __LINE__);

                output = make_crypto_etag(server_key, server_token,
                    ctag.domain_token, ctag.payload);

                DEBUG(log, "%f %d %s:%d OK\n",
                    gettime(), request_id, __func__, __LINE__);

                write(incoming, output, ETAG_STR_LEN);
                break;

            case ETAG_STR_LEN+2:
                // maybe quotes like so:
                // "6f4787943d83b470a5064c66e1517065.4f56f14e95c2012f517e19fe7819c63bf14618bd63c5bd87a3b3d470ad5580dc"
                if ( (('\'' == inbuf[0]) && ('\'' == inbuf[ETAG_STR_LEN+1]))  ||
                     (('"'  == inbuf[0]) && ('"'  == inbuf[ETAG_STR_LEN+1])) ){
                    // so try without the quotes
                    inbuf[ETAG_STR_LEN+1] = '\0';

                    ctag = validate_crypto_etag(server_key, server_token,
                        &(inbuf[1]));

                    write(incoming, &payload, sizeof(payload));

                    DEBUG(log, "%f %d %s:%d skipping quotes\n",
                        gettime(), request_id, __func__, __LINE__);

                }else{
                    payload = 0;
                    write(incoming, &payload, sizeof(payload));

                    DEBUG(log,"%f %d %s:%d length error inbuf[0]=%c "
                        "inbuf[ETAG_STR_LEN]=%c\n",
                        gettime(), request_id, __func__, __LINE__,
                        inbuf[0], inbuf[ETAG_STR_LEN]);
                }
                break;

            case ETAG_STR_LEN:
            case ETAG_STR_LEN+1:
                DEBUG(log, "%f %d %s:%d validate_crypto_etag\n",
                    gettime(), request_id, __func__, __LINE__);

                ctag = validate_crypto_etag(server_key, server_token, inbuf);

                DEBUG(log, "%f %d %s:%d validate_crypto_etag returned %"PRIu64"\n",
                    gettime(), request_id, __func__, __LINE__, payload);

                write(incoming, &ctag, sizeof(ctag));
                break;

            default:
                payload = 0;
                write(incoming, &payload, sizeof(payload));

                DEBUG(log, "%f %d %s:%d unexpected read length\n",
                    gettime(), request_id, __func__, __LINE__);

                break;
        }

        close(incoming);
        //if (output != NULL) free(output);
        if (debug) fclose(log);
        FLUSH;
    }
}

/*
 *  tagger_init
 *
 *  Launch a process that will process crypto etags.
 *  Setup the socke pair.
 *  And return the new pid.
 */
pid_t tagger_init(struct qz_config* conf, char* argv[]){

    pid_t pid;
    static char* tagger = "tagger";

    unlink(conf->tagger_socket_path);

    if ((pid = fork()) < 0){
        fprintf(stderr, "tagger fork failed\n");
        exit(26);
    }else{
        if (pid != 0){ //parent returns

            // drop key and token data
            bzero(conf->server_token, SERVER_TOKEN_HEX_LENGTH);
            bzero(conf->server_key, SERVER_KEY_HEX_LENGTH);

            // wait for socket to appear before returning.
            int tries;
            int st;
            struct stat sb;
            for (tries = 60; tries > 0; tries--){
                st = stat(conf->tagger_socket_path, &sb);
                if ((st == 0) && (sb.st_mode & S_IFSOCK)){
                    // Success
                    return pid;
                }
                usleep(10000);
            }
            // time out waiting for tagger
            fprintf(stderr, "time out waiting for tagger\n");
            exit(67);
        }
    }
    // child continues

    if (argv != NULL){
        argv[0] = tagger;
    }
    // does not return, false=no debug
    tagger_serve(conf, false);

    return -1;
}



/*
 *  open_socket
 *
 *  Return a socket fd for the named socket.
 */

int open_socket(char* sockname){
    int s;
    int sock_len = 0;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("tagger socket open failed");
        exit(27);
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    int max_path_len = sizeof(addr.sun_path);
    snprintf(addr.sun_path, max_path_len, "%s", sockname);

#ifdef HAVE_SOCKADDR_UN_SUN_LEN
    addr.sun_len =  SUN_LEN(&addr);
    sock_len = addr.sun_len;
#else
    sock_len = sizeof(addr) - max_path_len + strlen(addr.sun_path);
#endif

    if (connect(s, (struct sockaddr*) &addr, sock_len) == -1){
        // try again
        fprintf(stderr, "connect failed, retrying\n");
        sleep(2);
        if (connect(s, (struct sockaddr*) &addr, sock_len) == -1){
            perror("tagger socket connect failed");
            exit(28);
        }
    }
    return s;
}

/*
 *  make_etag
 *
 *  Turn payload into an etag and send it as a reply. 
 */

void make_etag(char* tagbuf, char* sockname,
     uint64_t domain_token, unsigned char payload[16]){

    int socket = open_socket(sockname);

    struct cryptotag ctag;
    ctag.domain_token = domain_token,
    memcpy(ctag.payload, payload, 16);

    DEBUG(stdout, "make_etag:%d domain_token %"PRIu64" payload %s\n",
        __LINE__, ctag.domain_token, ctag.payload);

    bzero(tagbuf, ETAG_STR_LEN);
    write(socket, &ctag, sizeof(ctag));
    read(socket, tagbuf, ETAG_STR_LEN);
    tagbuf[ETAG_STR_LEN] = '\0';

    close(socket);
}

/*
 *  validate_etag
 *
 *  Given an etag, decrypt it and decide to reply with the
 *  with the 128 bits of payload or not depending on the 
 *  domain_token matching.
 */
void validate_etag(char* payload, char* sockname, uint64_t domain_token, char* etag){

    bzero(payload, 16);

    if ((etag == NULL) || (strlen(etag) == 0)) return;

    int socket = open_socket(sockname);

    struct cryptotag ctag;
    write(socket, etag, strlen(etag));
    read(socket, &ctag, sizeof(ctag));

    FILE* log = NULL;
    if (debug) log = fopen("errlog", "a");

    DEBUG(log, "%f %d %s:%d ctag domain %"PRIu64" payload %s\n",
        gettime(), request_id, __func__, __LINE__,
        ctag.domain_token, ctag.payload);

    if (debug) fclose(log);

    close(socket);

    if (ctag.domain_token == domain_token){
        memcpy(payload, ctag.payload, 16);
    }
}

#ifdef TAGGER_MAIN

#include <signal.h>
#include <fcntl.h>

int main(int argc, char* argv[]){

    debug=true;
    request_id = 0;

    qzrandom64_init();
    uint64_t domain_token = 420;

    struct timespec start;
    struct timespec fin;
    double delta_time;
    struct cryptotag ctag;

    struct qz_config* conf = init_config();

    double begin_fopen = gettime();
    FILE* log = fopen(conf->logfile_name, "a");
    if (log == NULL){
        fprintf(stderr, "could not open logfile %s\n",
            conf->logfile_name);

        exit(65);
    }
    fprintf(log, "%f %d %s:%d fopen complete in %f\n",
        gettime(), request_id, __func__, __LINE__,
        gettime() - begin_fopen);

    pid_t tagger_pid = tagger_init(conf, argv);

    fprintf(log, "%f %d %s:%d tagger_pid=%d logfile=%s\n",
        gettime(), request_id, __func__, __LINE__,
        tagger_pid, conf->logfile_name);

    uint64_t k;
    unsigned char payload[16];
    char tagbuf[1024];

    clock_gettime(CLOCK_REALTIME, &start);

    for (k=1; k<10000; k++){
        request_id++;

        snprintf(payload, 16, "%"PRIu64, k);

        make_etag(tagbuf, conf->tagger_socket_path, domain_token, payload);

        fprintf(log, "%f %d %s:%d etag %s\n",
            gettime(), request_id, __func__, __LINE__,
            payload);

        bzero(&ctag, sizeof(ctag));
        validate_etag(payload, conf->tagger_socket_path, domain_token, tagbuf);

        fprintf(log, "%f %d %s:%d payload=%s\n",
            gettime(), request_id, __func__, __LINE__,
            payload);
    }

    clock_gettime(CLOCK_REALTIME, &fin);

    delta_time =  fin.tv_sec - start.tv_sec;
    delta_time += ((double)(fin.tv_nsec - start.tv_nsec))*0.000000001;

    fprintf(log, "%f %d %s:%d run time: %g\n",
        gettime(), request_id, __func__, __LINE__,
        delta_time);

    fprintf(log, "%f %d %s:%d sample tag %s\n",
        gettime(), request_id, __func__, __LINE__,
        tagbuf);

    // fail test 1
    debug=false;
    fprintf(log, "%f %d %s:%d fail test 1 - zero domain token\n",
        gettime(), request_id, __func__, __LINE__);

    char test_payload[16]  = "headace         ";
    make_etag(tagbuf, conf->tagger_socket_path, 0, test_payload);

    fprintf(log, "%f %d %s:%d fail test 1 - tagbuf=%s\n",
        gettime(), request_id, __func__, __LINE__, tagbuf);


    // fail test 2
    fprintf(log, "%f %d %s:%d fail test 2 - zero payload\n",
        gettime(), request_id, __func__, __LINE__);

    bzero(test_payload, 16);
    make_etag(tagbuf, conf->tagger_socket_path, domain_token, test_payload);

    fprintf(log, "%f %d %s:%d fail test 2 - tagbuf=%s\n",
        gettime(), request_id, __func__, __LINE__, tagbuf);


    // fail test 3
    fprintf(log, "%f %d %s:%d fail test 3 - validate null tag\n",
        gettime(), request_id, __func__, __LINE__);

    validate_etag(payload, conf->tagger_socket_path, 1, tagbuf);

    fprintf(log, "%f %d %s:%d fail test 3 - domain_token=%"PRIu64" payload=%s\n",
        gettime(), request_id, __func__, __LINE__, ctag.domain_token, ctag.payload);

    // fail test 4
    fprintf(log, "%f %d %s:%d fail test 4 - validate a different domain token\n",
        gettime(), request_id, __func__, __LINE__);

    char  headace[16] = "headace        \0";
    make_etag(tagbuf, conf->tagger_socket_path, domain_token, headace);

    bzero(&ctag, sizeof(ctag));
    validate_etag(payload, conf->tagger_socket_path, 1, tagbuf);

    fprintf(log, "%f %d %s:%d fail test 4 - domain_token=%"PRIu64" payload=%s\n",
        gettime(), request_id, __func__, __LINE__, ctag.domain_token, ctag.payload);

    // fin
    kill(tagger_pid, 15);

    return 0;
}


#endif


#ifdef TEST_TAGGER

/*
 *  test_tagger
 *
 *  A stand alone little ditty to send test data to a running tagger.
 */
int main(int argc, char* argv[]){

    qzrandom64_init();

    char* sockname = getenv("QZ_TAGGER_SOCKET");
    if (sockname == NULL){
        fprintf(stderr, "Set QZ_TAGGER_SOCKET in the environment\n");
        exit(29);
    }
    uint64_t payload;
    char tagbuf[1024];

    if (argc != 2){
        printf("%s will send test data to a running tagger\n", argv[0]);
        printf("It takes one argument.\n");
        printf("If it is 16 characters long then it is encoded\n");
        printf("other wise it is decoded\n");
        exit(30);
    }
    if (strlen(argv[1]) == 16){
        printf("make_etag(%s, %s, %s)\n","tagbuf", sockname, argv[1]);
        payload = strtoull(argv[1], NULL, 16);
        printf( "payload=%"PRIx64"\n", payload);
        make_etag(tagbuf, sockname, payload);
        printf("etag=%s\n", tagbuf);
    }else{
        printf("validate_etag(%s, %s)\n", sockname, argv[1]);
        payload = validate_etag(sockname, argv[1]);
        printf( "payload=%"PRIx64"\n", payload);
    }

    return 0;
}

#endif
