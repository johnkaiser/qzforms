
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

// for accept4 
#define _GNU_SOURCE

#include "tagger.h"
#include "qzrandom.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>
#include <poll.h>
#include <sched.h>
#include <errno.h>

extern pthread_mutex_t log_mutex;
extern double gettime(void);
static int request_id;
static char taglog_filename[MAXPATHLEN+2];

#define SOCKET_BACKLOG 32
#define TAGBUF 128
#define ERRORBUFMAX 1024

struct tag_data_conf {
    struct qz_config* conf;
    uint64_t server_token;
    unsigned char server_key[16];
    int sock;
    socklen_t sock_len;
    struct sockaddr_un addr;
};

/*
 *  set_server_token
 *
 * Set the server token from config or make one up.
 */
void set_server_token(struct tag_data_conf* tagdat){

    tagdat->server_token = 0;

    if (strlen(tagdat->conf->server_token) > 0){
        // if set, but not 16 chars, neither use conf nor random, but error out
        if (strlen(tagdat->conf->server_token) == 16){
           tagdat->server_token = strtoul(tagdat->conf->server_token, NULL, 16);
        }
    }else{
        tagdat->server_token = qzrandom64();
    }

    if (tagdat->server_token == 0){
        FILE* qzflog = fopen( tagdat->conf->logfile_name, "a");

        pthread_mutex_lock(&log_mutex);
        fprintf(qzflog, "%f %d %s:%d fail - bad server token in config\n",
            gettime(), request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        fflush(qzflog);
        exit(21);
    }
}

/*
 *  set_server_key
 *
 *  Set the server key from config or make one up.
 */
void set_server_key(struct tag_data_conf* tagdat){

    uint64_t rnbr[2];
    rnbr[0] = 0;
    rnbr[1] = 0;

    if (strlen(tagdat->conf->server_key) > 0){
        if (strlen(tagdat->conf->server_key) == 32){
            char keybuf[17];
            keybuf[16] = '\0';

            memcpy(keybuf, tagdat->conf->server_key, 16);
            rnbr[0] = strtoull(keybuf, NULL, 16);

            memcpy(keybuf, tagdat->conf->server_key+16, 16);
            rnbr[1] = strtoull(keybuf, NULL, 16);
       }
    }else{
        rnbr[0] = qzrandom64();
        rnbr[1] = qzrandom64();
    }

    if ((rnbr[0] > 0) && (rnbr[1] > 0)) {
        memcpy(tagdat->server_key, rnbr, 16);
    }else{
        FILE* qzflog = fopen( tagdat->conf->logfile_name, "a");

        pthread_mutex_lock(&log_mutex);
        fprintf(qzflog, "%f %d %s:%d fail - bad server key in config\n",
            gettime(), request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        fflush(qzflog);
        exit(20);
    }
}

/*
 *  open_server_socket
 *
 *  Setup the socket to accept connections.
 */
void open_server_socket(struct tag_data_conf* tagdat){

    // setup socket
    unlink(tagdat->conf->tagger_socket_path);
    mode_t old_umask = umask(077);

    tagdat->sock_len = 0;

    if ((tagdat->sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "tagger server socket failed\n");
        exit(22);
    }

    int max_path_len = sizeof(tagdat->addr.sun_path);
    if (strlen(tagdat->conf->tagger_socket_path) > (max_path_len-1)){
        fprintf(stderr, "tagger socket path length exceeded\n");
        exit(23);
    }

    tagdat->addr.sun_family = AF_UNIX;
    snprintf(tagdat->addr.sun_path, max_path_len, "%s",
        tagdat->conf->tagger_socket_path);

#ifdef HAVE_SOCKADDR_UN_SUN_LEN
    addr.sun_len =  SUN_LEN(&(tagdat->addr));
    tagdat->sock_len = tagdat->addr.sun_len;
#else
    tagdat->sock_len = sizeof(struct sockaddr_un) - max_path_len +
        strlen(tagdat->addr.sun_path);
#endif

    if (bind(tagdat->sock, (struct sockaddr*) &(tagdat->addr),
        tagdat->sock_len) != 0){

        perror("tagger server socket bind");
        exit(24);
    }

    if (listen(tagdat->sock, SOCKET_BACKLOG) != 0){
        perror("tagger server socket listen");
        exit(25);
    }
    umask(old_umask);
}

/*
 *  process_requests
 *
 *  Start handling requests and don't stop
 */
void process_requests(struct tag_data_conf* tagdat){

    int incoming;
    int bytesread;
    char inbuf[TAGBUF];
    unsigned char* output;
    struct cryptotag ctag;
    struct cryptovalidate* valtag;
    unsigned char blank[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    uint8_t ff = 0xff;
    unsigned char ones[16] = {ff, ff, ff, ff, ff, ff, ff, ff,
                              ff, ff, ff, ff, ff, ff, ff, ff};

    bool log_tagger_details = tagdat->conf->log_tagger_details;
    FILE* qzflog;
    char errorbuf[ERRORBUFMAX+2];

    if (log_tagger_details){
        // use a local log var and toss it on every use 
        qzflog = fopen(tagdat->conf->logfile_name, "a");

        fprintf(qzflog, "%f %d %s:%d server ready\n",
           gettime(), request_id, __func__, __LINE__);

        fclose(qzflog);
    }

    for(;;){
        bzero(inbuf, TAGBUF);
        errorbuf[0] = '\0';
        output = NULL;
        FILE* qzflog = NULL;

        if (log_tagger_details){
            qzflog = fopen(tagdat->conf->logfile_name, "a");

            pthread_mutex_lock(&log_mutex);
            fprintf(qzflog, "%f %d %s:%d calling accept4\n",
                gettime(), request_id, __func__, __LINE__);
            pthread_mutex_unlock(&log_mutex);
            fclose(qzflog);
        }
        incoming = accept4(tagdat->sock, (struct sockaddr*) &(tagdat->addr),
            &(tagdat->sock_len), SOCK_NONBLOCK);

        if (incoming == -1){
            strerror_r(errno, errorbuf, ERRORBUFMAX);
            qzflog = fopen(tagdat->conf->logfile_name, "a");

            pthread_mutex_lock(&log_mutex);
            fprintf(qzflog, "%f %d %s:%d tagger server accept failed %s\n",
                gettime(), request_id, __func__, __LINE__, errorbuf);
            pthread_mutex_unlock(&log_mutex);
            fclose(qzflog);
            errorbuf[0] = '\0';
            continue;
        }
        if (log_tagger_details){
            qzflog = fopen(tagdat->conf->logfile_name, "a");

            pthread_mutex_lock(&log_mutex);
            fprintf(qzflog, "%f %d %s:%d accept socket %d\n",
                gettime(), request_id, __func__, __LINE__, incoming);
            pthread_mutex_unlock(&log_mutex);
            fclose(qzflog);
        }
        // XXX linux vs bsd pollfd_t fds[2];
        struct pollfd fds[2];
        fds[0].fd = incoming;
        fds[0].events = POLLIN | POLLRDNORM | POLLPRI;
        if (poll(fds, 1, 0) < 0){

            strerror_r(errno, errorbuf, ERRORBUFMAX);
            qzflog = fopen(tagdat->conf->logfile_name, "a");

            pthread_mutex_lock(&log_mutex);
            fprintf(qzflog, "%f %d %s:%d poll fail: %s\n",
                gettime(), request_id, __func__, __LINE__,
                errorbuf);
            pthread_mutex_unlock(&log_mutex);
            fclose(qzflog);
            errorbuf[0] = '\0';
        }
        bytesread = read(incoming, inbuf, TAGBUF-1);
        if (bytesread < 1) strerror_r(errno, errorbuf, ERRORBUFMAX);

        if ((log_tagger_details) || (bytesread < 1)){
            qzflog = fopen(tagdat->conf->logfile_name, "a");

            pthread_mutex_lock(&log_mutex);
            fprintf(qzflog, "%f %d %s:%d%s server server read %d bytes %s\n",
                gettime(), request_id, __func__, __LINE__,
                (errorbuf[0] == '\0') ? "":" fail",
                bytesread, errorbuf);
            pthread_mutex_unlock(&log_mutex);
            fclose(qzflog);
        }
        switch (bytesread){

            case sizeof(struct cryptotag):
                memcpy(&ctag, inbuf, sizeof(struct cryptotag));

                if (log_tagger_details){
                    qzflog = fopen(tagdat->conf->logfile_name, "a");

                    pthread_mutex_lock(&log_mutex);
                    fprintf(qzflog, "%f %d %s:%d make_crypto_etag \n",
                        gettime(), request_id, __func__, __LINE__);
                    pthread_mutex_unlock(&log_mutex);
                    fclose(qzflog);
                }

                output = make_crypto_etag(tagdat->server_key, tagdat->server_token,
                    ctag.domain_token, ctag.payload);

                if (log_tagger_details){
                    qzflog = fopen(tagdat->conf->logfile_name, "a");
                    pthread_mutex_lock(&log_mutex);
                    fprintf(qzflog, "%f %d %s:%d OK\n",
                        gettime(), request_id, __func__, __LINE__);
                    pthread_mutex_unlock(&log_mutex);
                    fclose(qzflog);
                }
                if (output != NULL){
                    write(incoming, output, ETAG_STR_LEN);
                }else{
                    write(incoming, blank, sizeof(blank));
                }

                break;

            case sizeof(struct cryptovalidate):
                //memcpy(&valtag, inbuf, sizeof(struct cryptovalidate));
                valtag = (void*) inbuf;

                if (log_tagger_details){
                    qzflog = fopen(tagdat->conf->logfile_name, "a");

                    pthread_mutex_lock(&log_mutex);
                    fprintf(qzflog, "%f %d %s:%d validate_crypto_etag "
                        "domain_token=%"PRIx64"\n",
                        gettime(), request_id, __func__, __LINE__,
                        valtag->domain_token);
                    pthread_mutex_unlock(&log_mutex);
                    fclose(qzflog);
                }

                ctag = validate_crypto_etag(tagdat->server_key,
                    tagdat->server_token, valtag->etag);

                if (ctag.domain_token == valtag->domain_token){
                    write(incoming, &ctag.payload, 16);
                }else{
                    write(incoming, ones, sizeof(ones));

                    if (log_tagger_details){
                        qzflog = fopen(tagdat->conf->logfile_name, "a");

                        pthread_mutex_lock(&log_mutex);
                        fprintf(qzflog, "%f %d %s:%d domain_token validate "
                            "failed %"PRIx64" != %"PRIx64" payload=%s\n",
                            gettime(), request_id, __func__, __LINE__,
                            ctag.domain_token, valtag->domain_token, ctag.payload);
                        pthread_mutex_unlock(&log_mutex);
                        fclose(qzflog);
                    }
                }
                break;

            default:

                if (log_tagger_details){
                    qzflog = fopen(tagdat->conf->logfile_name, "a");

                    pthread_mutex_lock(&log_mutex);
                    fprintf(qzflog, "%f %d %s:%d unexpected read length %d\n",
                        gettime(), request_id, __func__, __LINE__, bytesread);
                    pthread_mutex_unlock(&log_mutex);
                    fclose(qzflog);
                }
                write(incoming, blank, sizeof(blank));

                break;
        }

        close(incoming);
    }
}

/*  tagger_serve
 *
 *  Setup a server to fork, read a socket
 *  and reply with a tag or data.
 *  Call tagger_init instead of this.
 */

void tagger_serve(struct qz_config* conf, bool debug){

    struct tag_data_conf tagdat;
    FILE* qzflog;
    tagdat.conf = conf;

    if (tagdat.conf->log_tagger_details){
        qzflog = fopen(conf->logfile_name, "a");

        fprintf(qzflog, "%f %d %s:%d begin tagger_serve\n",
            gettime(), request_id, __func__, __LINE__);

        fprintf(qzflog, "%f %d %s:%d qzrandom64 %"PRIx64"\n",
            gettime(), request_id, __func__, __LINE__, qzrandom64());

        fclose(qzflog);
    }

    // Get the server token from config or make one up.
    set_server_token(&tagdat);

    // Fill in the server key from config or one made up.
    set_server_key(&tagdat);

    // setup socket
    open_server_socket(&tagdat);

    // Does not return
    process_requests(&tagdat);

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

    // this is so that make_etag and validate_etag can log errors
    snprintf(taglog_filename, MAXPATHLEN, "%s", conf->logfile_name);

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
    tagger_serve(conf, true);

    return -1;
}

/*
 *  open_client_socket
 *
 *  Return a socket fd for the named socket.
 */

int open_client_socket(char* sockname){
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
     uint64_t domain_token, char payload[16]){

    bzero(tagbuf, ETAG_STR_LEN);

    if ((payload == NULL) || (payload[0] == '\0')){
        return;
    }

    int socket = open_client_socket(sockname);

    struct cryptotag ctag;
    ctag.domain_token = domain_token,
    memcpy(ctag.payload, payload, 16);

    int err_cnt = 0;
    while ((tagbuf[0] == '\0') && (err_cnt < 20)){
        while ( (write(socket, &ctag, sizeof(ctag)) < 0) && (err_cnt < 20)){
            FILE* taglog = fopen(taglog_filename, "a");
            char err_buf[1024];
            bzero(err_buf, 1024);
            strerror_r(errno, err_buf, 1024);
            err_cnt++;

            pthread_mutex_lock(&log_mutex);
            fprintf(taglog, "%f %d %s:%d %s %d %s\n",
                gettime(), request_id, __func__, __LINE__,
                "make_etag write to socket failed",
                err_cnt,
                err_buf);
            pthread_mutex_unlock(&log_mutex);
            fclose(taglog);

            close(socket);
            struct timespec ts = {.tv_sec = 0, .tv_nsec = 1000*err_cnt};
            nanosleep(&ts,NULL);
            sched_yield();
            socket = open_client_socket(sockname);
        }
        if (read(socket, tagbuf, ETAG_STR_LEN) < 0){
            char err_buf[1024];
            bzero(err_buf, 1024);
            strerror_r(errno, err_buf, 1024);
            FILE* taglog = fopen(taglog_filename, "a");
            pthread_mutex_lock(&log_mutex);
            fprintf(taglog, "%f %d %s:%d %s %s\n",
                gettime(), request_id, __func__, __LINE__,
                "make_etag read from socket failed",
                err_buf);
            pthread_mutex_unlock(&log_mutex);
            fclose(taglog);

        }
    }
    tagbuf[ETAG_STR_LEN] = '\0';

    close(socket);
}

/*
 *  validate_etag
 *
 *  Given an etag, decrypt it and decide to reply with the
 *  with the 128 bits of payload or not depending on the
 *  domain_token matching.
 *
 *  A payload of either all zeros or all ones is an error condition
 *  All zeros is a communication failure with the tagger process.
 *  All ones is a successful processing of an invalid etag.
 */
void validate_etag(char payload[16], char* sockname, uint64_t domain_token, char* etag){

    bzero(payload, 16);
    unsigned char zeros[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    if ((etag == NULL) || (strlen(etag) == 0)) return;

    struct cryptovalidate valtag;
    bzero(&valtag, sizeof(valtag));
    valtag.domain_token = domain_token;

    int etag_len = strlen(etag);

    switch (etag_len){
        case (ETAG_STR_LEN):
            break;
        case (ETAG_STR_LEN+2):
            // maybe quotes like so:
            // "6f4787943d83b470a5064c66e1517065.4f56f14e95c2012f517e19fe7819c63bf14618bd63c5bd87a3b3d470ad5580dc"
            if ( (('\'' == etag[0]) && ('\'' == etag[ETAG_STR_LEN+1]))  ||
                 (('"'  == etag[0]) && ('"'  == etag[ETAG_STR_LEN+1])) ){
                    // so try without the quotes
                    etag[ETAG_STR_LEN+1] = '\0';
                    etag++;
            }else{
               // Do not pass
               return;
            }
            break;
        default:
            // Up and die, payload has been wiped
            return;
    }
    memcpy(valtag.etag, etag, ETAG_STR_LEN);

    int socket = open_client_socket(sockname);

    ssize_t bytesread = 0;
    int err_cnt = 0;
    do{
        while (write(socket, &valtag, sizeof(valtag)) < 0){
            perror("validate_etag write to socket failed");
            err_cnt++;
            if (err_cnt > 20) break;
            struct timespec ts = {.tv_sec = 0, .tv_nsec = 1000*err_cnt};
            nanosleep(&ts,NULL);
            close(socket);
            sched_yield();
            socket = open_client_socket(sockname);
        }
        if ((bytesread = read(socket, payload, 16)) < 0){
            perror("validate_etag read from socket failed");
        }
    } while ((bytesread != 16) &&  (memcmp(payload, zeros, 16) != 0));

    close(socket);
}

#ifdef TAGGER_TEST

#include <signal.h>
#include <fcntl.h>
#include <string.h>
pthread_mutex_t log_mutex;

int main(int argc, char* argv[]){

    sigset_t nosigpipe;
    sigemptyset(&nosigpipe);
    sigaddset(&nosigpipe, SIGPIPE);
    sigprocmask(SIG_BLOCK, &nosigpipe, NULL);

    request_id = 0;
    pthread_mutex_init(&log_mutex,NULL);

    qzrandom_init();
    uint64_t domain_token = 420;

    struct timespec start;
    struct timespec fin;
    double delta_time;
    struct cryptotag ctag;

    struct qz_config* conf = init_config();
    snprintf(taglog_filename, MAXPATHLEN, "%s", conf->logfile_name);

    double begin_fopen = gettime();
    FILE* log = fopen(conf->logfile_name, "a");
    if (log == NULL){
        fprintf(stderr, "could not open logfile %s\n",
            conf->logfile_name);

        exit(65);
    }

    pthread_mutex_lock(&log_mutex);
    fprintf(log, "%f %d %s:%d fopen complete in %f\n",
        gettime(), request_id, __func__, __LINE__,
        gettime() - begin_fopen);

    fprintf(log, "%f %d %s:%d sizeof cryptotag %lu cryptovalidate %lu\n",
        gettime(), request_id, __func__, __LINE__,
        sizeof(struct cryptotag), sizeof(struct cryptovalidate));

    pthread_mutex_unlock(&log_mutex);

    pid_t tagger_pid = tagger_init(conf, argv);

    pthread_mutex_lock(&log_mutex);
    fprintf(log, "%f %d %s:%d tagger_pid=%d logfile=%s\n",
        gettime(), request_id, __func__, __LINE__,
        tagger_pid, conf->logfile_name);
    pthread_mutex_unlock(&log_mutex);

    uint64_t k;
    char out_payload[18];
    char in_payload[18];
    char tagbuf[1024];

    clock_gettime(CLOCK_REALTIME, &start);

    bzero(out_payload, 16);
    make_etag(tagbuf, conf->tagger_socket_path, domain_token, NULL);
    make_etag(tagbuf, conf->tagger_socket_path, domain_token, out_payload);
    validate_etag(in_payload, conf->tagger_socket_path, domain_token, tagbuf);

    int rounds = 10000;
    int success_count = 0;
    for (k=1; k<rounds; k++){
        request_id++;

        bzero(out_payload,16);
        snprintf(out_payload, 16, "%"PRIu64, k);

        bzero(tagbuf, 1024);
        make_etag(tagbuf, conf->tagger_socket_path, domain_token, out_payload);

        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d etag %s %s\n",
            gettime(), request_id, __func__, __LINE__,
            out_payload, tagbuf);
        pthread_mutex_unlock(&log_mutex);

        bzero(&ctag, sizeof(ctag));
        bzero(in_payload,16);
        validate_etag(in_payload, conf->tagger_socket_path, domain_token, tagbuf);
        int cmp_rslt;
        if ((cmp_rslt = strncmp(out_payload, in_payload, 16)) == 0 ) success_count++;

        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d payload=%s%s\n",
            gettime(), request_id, __func__, __LINE__,
            out_payload, (cmp_rslt == 0) ? "" : " fail payload mismatch");
        pthread_mutex_unlock(&log_mutex);
    }

    clock_gettime(CLOCK_REALTIME, &fin);

    delta_time =  fin.tv_sec - start.tv_sec;
    delta_time += ((double)(fin.tv_nsec - start.tv_nsec))*0.000000001;

    pthread_mutex_lock(&log_mutex);
    fprintf(log, "%f %d %s:%d run time: %g success count %d out of %d\n",
        gettime(), request_id, __func__, __LINE__,
        delta_time, success_count, rounds-1);
    pthread_mutex_unlock(&log_mutex);

    if (false){
        fprintf(log, "%f %d %s:%d sample tag %s\n",
            gettime(), request_id, __func__, __LINE__,
            tagbuf);

        fprintf(log, "%f %d %s:%d fail test 1 - zero domain token\n",
            gettime(), request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        char test_payload[16]  = "headace         ";
        make_etag(tagbuf, conf->tagger_socket_path, 0, test_payload);

        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 1 - tagbuf=%s\n",
            gettime(), request_id, __func__, __LINE__, tagbuf);
        pthread_mutex_unlock(&log_mutex);


        // fail test 2
        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 2 - zero payload\n",
            gettime(), request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        bzero(test_payload, 16);
        make_etag(tagbuf, conf->tagger_socket_path, domain_token, test_payload);

        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 2 - tagbuf=%s\n",
            gettime(), request_id, __func__, __LINE__, tagbuf);
        pthread_mutex_unlock(&log_mutex);


        // fail test 3
        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 3 - validate null tag\n",
            gettime(), request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        validate_etag(test_payload, conf->tagger_socket_path, 1, tagbuf);

        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 3 - domain_token=%"PRIu64" payload=%s\n",
            gettime(), request_id, __func__, __LINE__, ctag.domain_token, ctag.payload);
        pthread_mutex_unlock(&log_mutex);

        // fail test 4
        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 4 - validate a different domain token\n",
            gettime(), request_id, __func__, __LINE__);
        pthread_mutex_unlock(&log_mutex);

        char  headace[16] = "headace        \0";
        make_etag(tagbuf, conf->tagger_socket_path, domain_token, headace);

        bzero(&ctag, sizeof(ctag));
        validate_etag(test_payload, conf->tagger_socket_path, 1, tagbuf);

        pthread_mutex_lock(&log_mutex);
        fprintf(log, "%f %d %s:%d fail test 4 - domain_token=%"PRIu64" payload=%s\n",
            gettime(), request_id, __func__, __LINE__, ctag.domain_token, ctag.payload);
        pthread_mutex_unlock(&log_mutex);
    }
    // fin
    kill(tagger_pid, 15);

    return 0;
}


#endif


#ifdef TAGGER_CLIENT

#include "hex_to_uchar.h"
#include <signal.h>

pthread_mutex_t log_mutex;

/*
 *  tagger_client
 *
 *  A stand alone little ditty to send test data to a running tagger.
 */
int main(int argc, char* argv[]){

    double start = gettime();
    pthread_mutex_init(&log_mutex, NULL);

    sigset_t nosigpipe;
    sigemptyset(&nosigpipe);
    sigaddset(&nosigpipe, SIGPIPE);
    sigprocmask(SIG_BLOCK, &nosigpipe, NULL);

    qzrandom_init();
    uint64_t domain_token = 0;

    int ch;
    int n=1;
    char* sockname = NULL;
    char* testdata = NULL;
    char* logfile = "logs/tagger_client.log";
    snprintf(taglog_filename, MAXPATHLEN, "%s", logfile);

    while ((ch = getopt(argc, argv, "h?d:l:n:s:t:")) != -1) {
        switch(ch){
            case 'l':
                logfile = optarg;
                break;

            case 'n':
                n=atoi(optarg);
                break;

            case 's':
                sockname = optarg;
                break;
           
            case 't':
                testdata = optarg;
                break;

            case 'd':
                domain_token =  strtoul(optarg, NULL, 16);
                break;

            case 'h':
            case '?':
                printf("This will send test data to a running tagger\n");
                printf("Use -s <socketname> or the environment variable\n");
                printf("TAGGER_SOCKET to identify the server.\n");
                printf("Use -n <count> to set the number of tests to run.\n");
                printf("Use -t <testdata> to send specific data to the server.\n");
                printf("Use -d <domain_token> to set the domain token to a\n");
                printf("specific hex value. A random value will be used if this\n");
                printf("is not set.\n");
                exit(0);
        }
    }

    if (sockname == NULL){
        sockname = getenv("TAGGER_SOCKET");
    }    
    if (sockname == NULL){
    
        fprintf(stderr, "Set TAGGER_SOCKET in the environment\n");
        fprintf(stderr, "or set -s <sockname> on the command line.\n");
        exit(29);
    }

    if (domain_token == 0){
        domain_token = qzrandom64();
    }

    FILE* log = fopen(logfile, "a");

    char tagbuf[1024];
    bzero(tagbuf, 1024);
    char payload[18];
    bzero(payload, 18);
    
    if (testdata != NULL){

        if (strlen(testdata) == 16){

            fprintf(log, "%f %d %s:%d payload %s\n",
                gettime(), 16, __func__, __LINE__, testdata);

            make_etag(tagbuf, sockname, domain_token, testdata);

            fprintf(log, "%f %d %s:%d etag %s\n",
                gettime(), 16, __func__, __LINE__, 
                (tagbuf[0] == '\0') ? "tag is null":tagbuf);

            validate_etag(payload, sockname, domain_token, tagbuf);

            fprintf(log, "%f %d %s:%d validate %s\n",
                gettime(), 16, __func__, __LINE__, 
                (payload[0] == '\0') ? "payload is null":payload);

            n--;
        }

        if (strlen(testdata) == 97){

            validate_etag(payload, sockname, domain_token, testdata);

            fprintf(log, "%f %d %s:%d validate %s\n",
                gettime(), 97, __func__, __LINE__, 
                (payload[0] == '\0') ? "payload is null":payload);

            n--;
        }    
    }

    int error_count = 0;
    while (n>0){

        snprintf(payload, 16, "%d", n);
        request_id = n;

        fprintf(log, "\n%f %d %s:%d payload %s\n",
            gettime(), request_id , __func__, __LINE__, payload);

        make_etag(tagbuf, sockname, domain_token, payload);

        fprintf(log, "%f %d %s:%d etag %s\n",
            gettime(), request_id, __func__, __LINE__, 
            (tagbuf[0] == '\0') ? "fail tag is null":tagbuf);

        validate_etag(payload, sockname, domain_token, tagbuf);

        fprintf(log, "%f %d %s:%d validate %s\n",
            gettime(), request_id, __func__, __LINE__,
            (payload[0] == '\0') ? "fail payload is null":payload);

        if (atoi(payload) != n){
            error_count++;
            fprintf(log, "%f %d %s:%d fail payload does not match test data\n",
                gettime(), n, __func__, __LINE__);

        }
        n--;
    }
    fprintf(log, "%f %d %s:%d error_count %d\n",
        gettime(), 0, __func__, __LINE__, error_count);

    fprintf(log, "%f %d %s:%d run_time %f\n",
        gettime(), 0, __func__, __LINE__, gettime() - start);

    return 0;
}

#endif


#ifdef TAGGER_SERVER
pthread_mutex_t log_mutex;

int main(void){

    struct tag_data_conf tagdat;
    FILE* qzflog;

    pthread_mutex_init(&log_mutex, NULL);

    tagdat.conf = init_config();
    qzflog = fopen(tagdat.conf->logfile_name, "a");

    fprintf(qzflog, "%f %d %s:%d begin tagger_server\n",
        gettime(), request_id, __func__, __LINE__);

    fprintf(qzflog, "%f %d %s:%d qzrandom64 %"PRIx64"\n",
        gettime(), request_id, __func__, __LINE__, qzrandom64());

    fclose(qzflog);
    // Get the server token from config or make one up.
    set_server_token(&tagdat);

    // Fill in the server key from config or one made up.
    set_server_key(&tagdat);

    // setup socket
    open_server_socket(&tagdat);

    // Does not return
    process_requests(&tagdat);

}

#endif
