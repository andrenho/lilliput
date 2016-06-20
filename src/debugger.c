#include "debugger.h"

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/socket.h>

#define PORT 5999

static void debugger_parse(char* str);

static int sockfd, newfd = -1;

void
debugger_init()
{
    // create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        syslog(LOG_ERR, "Debugger: failed to create socket.");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DEBUG, "Debugger: socket initialized.");

    // reuse socket
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1) {
        perror("setsockopt");
    }

    // bind socket
    struct sockaddr_in my_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = { .s_addr = INADDR_ANY },
    };

    if(bind(sockfd, (const struct sockaddr*)&my_addr, sizeof my_addr) == -1) {
        perror("bind");
        syslog(LOG_ERR, "Failed to bind socket.");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DEBUG, "Socket bound (port %d).", PORT);

    // listen on socket
    if(listen(sockfd, 1) == -1) {
        perror("listen");
        syslog(LOG_ERR, "Failed to listen socket.");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DEBUG, "Listening on socket.");

    // set connection as non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}


void 
debugger_destroy()
{
   shutdown(newfd, 2);
   shutdown(sockfd, 2);
}



void 
debugger_serve()
{
    socklen_t sin_size = 0;
    struct sockaddr_in their_addr = {0};
    
    if(newfd == -1) {
        // accept new connections
        if((newfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size)) == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // no connection present
            } else {
                perror("accept");
                syslog(LOG_ERR, "Failed to accept.");
                exit(EXIT_FAILURE);
            }
        } else {
            fcntl(newfd, F_SETFL, O_NONBLOCK);
            syslog(LOG_DEBUG, "Debugger connection established.");
            send(newfd, "? ", 2, 0);
        }
    } else {
        // read data
        static char str[1024];
        ssize_t n = recv(newfd, str, sizeof str - 1, 0);
        str[n] = '\0';
        if(n > 0) {
            debugger_parse(str);
        } else if(n == 0) {
            syslog(LOG_DEBUG, "Debugger connection closed.");
            newfd = -1;
        } else {
            if(errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recv");
            }
        }
    }
}


static void debugger_parse(char* str)
{
    syslog(LOG_DEBUG, "Debugger recv: %s", str);

    send(newfd, "? ", 2, 0);
}
