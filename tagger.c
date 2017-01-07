
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

/*
 *  tagger
 * 
 *  Launch a process to create and validate crypto etags.
 */

//    #include "qz.h"

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

#define TAGBUF 1024
#define TAG_MAX_LENGTH 50

#define DEBUG if (debug) fprintf
#define FLUSH if (debug) fflush(log)

void tagger_serve(struct qz_config* conf, bool debug){
    FILE* log;
    if (debug) log = fopen(conf->logfile_name, "a");

    DEBUG(log, "begin tagger_serve\n");
    DEBUG(log, "qzrandom64 %llx\n", qzrandom64());
    DEBUG(log, "strlen(conf->server_token)=%lu\n", strlen(conf->server_token));
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
    DEBUG(log, "server_token=%llx\n", server_token);
    FLUSH;
    
    if (server_token == 0){
        FILE* qzlog = fopen( conf->logfile_name, "a");
        fprintf(qzlog, "fail - bad server token in config %s len=%ld\n",
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

    DEBUG(log, "server_key=%llx%llx\n", rnbr[0], rnbr[1]);
    FLUSH;
    // setup key 
    BF_KEY* bf_key;
    bf_key = malloc(sizeof(BF_KEY));
    BF_set_key(bf_key, 16, server_key);

    // setup socket
    mode_t old_umask = umask(077);

    struct sockaddr_un addr;
    int sock;
    int sock_len = 0;

    if ((sock= socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "tagger server socket failed\n");
        exit(22);
    }

    int max_path_len = sizeof(addr.sun_path);
    if (strlen(conf->tagger_socket_path) > (max_path_len-1)){
        fprintf(stderr, "tagger socket path length exceeded\n");
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

    DEBUG(log, "server ready\n");
    FLUSH;
    for(;;){
        bzero(inbuf, TAGBUF);
        output = NULL;

        incoming = accept(sock, (struct sockaddr*) &addr, &sock_len);
        if (incoming == -1){
            fprintf(stderr, "tagger server accept failed\n");
            continue;
        }
        DEBUG(log, "accept socket %d\n", incoming);
        bytesread = read(incoming, inbuf, TAGBUF-1);
        DEBUG(log, "server read %d bytes\n", bytesread);

        switch (bytesread){
                
            case 8:
                //sscanf(&payload, "%lu", incoming); 
                memcpy(&payload, inbuf, 8);
                DEBUG(log, "make_crypto_etag \n");
                output = make_crypto_etag(bf_key, server_token, payload);
                DEBUG(log, "OK\n");
                write(incoming, output, strlen(output)+1);
                break;
      
            case 51:
                // maybe quotes like so:
                // "61ee029125dd839a.4cf0d06856f9bad03e85cc5002fc0b37"
                if ( (('\'' == inbuf[0]) && ('\'' == inbuf[50]))  ||
                     (('"'  == inbuf[0]) && ('"'  == inbuf[50])) ){
                    // so try without the quotes 
                    inbuf[50] = '\0'; 
                    payload = validate_crypto_etag(bf_key, server_token, 
                        &(inbuf[1]));
                    write(incoming, &payload, sizeof(payload));
                    DEBUG(log, "skipping quotes\n");
                }else{    
                    payload = 0;
                    write(incoming, &payload, sizeof(payload));
                    DEBUG(log, "inbuf[50]=%c\n", inbuf[50]);
                }    
                break;
                     
            case 49:
            case 50:
                DEBUG(log, "validate_crypto_etag \n");
                payload = validate_crypto_etag(bf_key, server_token, inbuf);
                DEBUG(log, "OK\n");
                write(incoming, &payload, sizeof(payload));
                break;
           
            default:
                payload = 0;
                write(incoming, &payload, sizeof(payload));
                DEBUG(log, "unexpected read length\n");
                break;
        }

        close(incoming);
        if (output != NULL) free(output);
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
                usleep(200);
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
        usleep(1000);
        if (connect(s, (struct sockaddr*) &addr, sock_len) == -1){
            perror("tagger socket connect failed");
            exit(28);
        }    
    }
    return s;
}

void make_etag(char* tagbuf, char* sockname, uint64_t payload){
    int socket = open_socket(sockname);
    int bytesread;

    write(socket, &payload, sizeof(payload));
    bytesread = read(socket, tagbuf, TAG_MAX_LENGTH);

    close(socket);
}

uint64_t validate_etag(char* sockname, char* etag){
    uint64_t payload;

    if ((etag == NULL) || (strlen(etag) == 0)) return 0;

    int socket = open_socket(sockname);

    write(socket, etag, strlen(etag));
    read(socket, &payload, sizeof(payload));

    close(socket);
    return payload;
}

#ifdef TAGGER_MAIN

#include <signal.h>

int main(int argc, char* argv[]){

    qzrandom64_init();

    struct timespec start;
    struct timespec fin;
    double delta_time;

    struct qz_config* conf = init_config();

    pid_t tagger_pid = tagger_init(conf, argv);
    printf("tagger_pid=%d\n", tagger_pid);
    sleep(1);

    int k;
    uint64_t payload;
    char tagbuf[1024];

    clock_gettime(CLOCK_REALTIME, &start);

    //for (k=1; k<1000000; k++){
    for (k=1; k<10000; k++){
        make_etag(tagbuf, conf->tagger_socket_path, k);
        printf("etag %d %s  ", k, tagbuf);
        payload = validate_etag(conf->tagger_socket_path, tagbuf);
        printf(" payload=%llu\n", payload);
    } 

    clock_gettime(CLOCK_REALTIME, &fin);

    delta_time =  fin.tv_sec - start.tv_sec;
    delta_time += ((double)(fin.tv_nsec - start.tv_nsec))*0.000000001;
    printf("run time: %g\n", delta_time);
    kill( tagger_pid, 15);

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
        printf( "payload=%llx\n", payload);
        make_etag(tagbuf, sockname, payload);
        printf("etag=%s\n", tagbuf);
    }else{ 
        printf("validate_etag(%s, %s)\n", sockname, argv[1]);
        payload = validate_etag(sockname, argv[1]);
        printf( "payload=%llx\n", payload);
    }    
    
    return 0;
}

#endif
