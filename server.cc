#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <thread>
#include <fcntl.h>
#include "server.h"

const static char * HTML = "<html><head><title>LED MATRIX</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link href=\"//maxcdn.bootstrapcdn.com/bootswatch/3.3.4/lumen/bootstrap.min.css\" rel=\"stylesheet\"></head><body><div class=\"container\"><h1>LED MATRIX</h1><form method=\"GET\"><input type=\"text\" name=\"text\" autofocus/><input type=\"submit\"/></form></div></body></html>";

int listenfd, clients[CONNMAX];
void (*callback)(char*);

void urldecode2(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
                if (a >= 'a')
                        a -= 'a'-'A';
                if (a >= 'A')
                        a -= ('A' - 10);
                else
                        a -= '0';
                if (b >= 'a')
                        b -= 'a'-'A';
                if (b >= 'A')
                        b -= ('A' - 10);
                else
                        b -= '0';
                *dst++ = 16*a+b;
                src+=3;
        } else if (*src == '+') {
                *dst++ = ' ';
                src++;
        } else {
                *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

//start server
void startServer() {
    struct addrinfo hints, *res, *p;

    // Setting all elements to -1: signifies there is no client connected
    for (int i = 0; i < CONNMAX; i++) {
        clients[i] = -1;
    }

    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo() error");
        exit(1);
    }
    // set options
    int optval = 1;

    // socket and bind
    for (p = res; p != NULL; p = p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) {
            continue;
        }
        // set SO_REUSEADDR on a socket to true (1):
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror ("setsockopt()");
            exit(1);
        }
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
    }
    if (p == NULL) {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 100) != 0 ) {
        perror("listen() error");
        exit(1);
    }
}

//client connection
void respond(int n) {
    char mesg[SERVER_BUFFER_SIZE], output[SERVER_BUFFER_SIZE], *reqline[3];
    int rcvd;

    memset((void*)mesg, (int)'\0', SERVER_BUFFER_SIZE);

    rcvd = recv(clients[n], mesg, SERVER_BUFFER_SIZE, 0);

    if (rcvd < 0) {
        // receive error
        fprintf(stderr, ("recv() error\n"));
    } else if (rcvd == 0) {
        // receive socket closed
        //fprintf(stderr, "Client disconnected upexpectedly.\n");
    } else {
        // message received
        //printf("%s", mesg);
        reqline[0] = strtok(mesg, " \t\n"); // method
        if (strncmp(reqline[0], "GET\0", 4) == 0) {
            reqline[1] = strtok(NULL, " \t"); // path
            reqline[2] = strtok(NULL, " \t\n"); // protocol
            //printf("\n%s\n\n",reqline[1]);
            if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8) != 0) {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            } else {
                if (strncmp(reqline[1], "/\0", 2) == 0 || strncmp(reqline[1], "/?", 2) == 0) {
                    // check for query string
                    if (strncmp(reqline[1], "/?", 2) == 0) {
                        char * s = strstr(reqline[1], "text=");
                        if (s) {
                            // found text
                            s = s + 5;
                            char * e = strstr(s, "&");
                            if (e) {
                                *e = 0;
                            }
                            urldecode2(output, s);
                            if (strlen(output)) {
                                //printf("Decoded string: %s\n", output);
                                callback(output);
                            }
                        }
                    }
                    // show form
                    send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
                    write(clients[n], HTML, strlen(HTML));
                }else {
                    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                }
            }
        }
    }

    //Closing SOCKET
    shutdown(clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
}

void setupServer(void (*fn)(char*)) {
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int slot=0;
    callback = fn;

    startServer();

    // ACCEPT connections
    while (1) {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot] < 0) {
            perror("accept() error");
        } else {
            // use C++ threads
            std::thread cfork(respond, slot);
            cfork.detach();
            /*if (fork() == 0) {
                respond(slot);
                exit(0);
            }*/
        }

        while (clients[slot] != -1) {
            slot = (slot+1) % CONNMAX;
        }
    }
}
