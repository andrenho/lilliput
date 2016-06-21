#include "debugger.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/socket.h>

#include "memory.h"

#define PORT 5999

static void dsend(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
static void debugger_parse(char* str);
static void debugger_parse_memory(char* par[10]);

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
            const char* welcome = "Welcome do lilliput debugger. Please type 'h' for help.\n";
            send(newfd, welcome, strlen(welcome), 0);
            send(newfd, "? ", 2, 0);
        }
    } else {
        // read data
        static char str[1024];
        ssize_t n = recv(newfd, str, sizeof str - 1, 0);
        str[n] = '\0';
        if(n > 0) {
            while(str[strlen(str)-1] == '\r' || str[strlen(str)-1] == '\n')
                str[strlen(str)-1] = '\0';
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


static void
dsend(const char* fmt, ...)
{
    static char buf[4096];

    va_list arg;

    va_start(arg, fmt);
    vsnprintf(buf, sizeof buf - 1, fmt, arg);
    va_end(arg);

    send(newfd, buf, strlen(buf), 0);
    send(newfd, "\n", 1, 0);
}


#define EXPECT(par, c) {        \
    int n = 11;                 \
    for(int i=0; i<10; ++i) {   \
        if(par[i] == NULL) {    \
            n = i+1;            \
            break;              \
        }                       \
    }                           \
    if(n != c) {                \
        dsend("- Invalid number of arguments (excpected %d)", c); \
        return;                 \
    }                           \
}


static void 
debugger_parse(char* str)
{
    syslog(LOG_DEBUG, "Debugger recv: %s", str);

    char* cmd;
    char* par[10];

    cmd = strtok(str, " ");
    for(size_t i=0; i < sizeof par; ++i) {
        par[i] = strtok(NULL, " ");
        if(!par[i]) {
            for(size_t j=i; j<10; ++j) {
                par[j] = NULL;
            }
            break;
        }
    }

    if(strcmp(cmd, "h") == 0) {
        dsend("m r ADDR           read byte from memory address");
        dsend("m r16 ADDR         read word from memory address");
        dsend("m r32 ADDR         read dword from memory address");
        dsend("m w ADDR DATA      write byte from memory address");
        dsend("m w16 ADDR DATA    write word from memory address");
        dsend("m w32 ADDR DATA    write dword from memory address");
        dsend("m l ADD1 ADDR2     list memory data from ADDR1 to ADDR2");
        dsend("m offset [ADDR]    read/write memory offset register");
        dsend("----");
        dsend("d                  disconnect");
        dsend("q                  quit emulator");

    } else if(strcmp(cmd, "m") == 0) {
        debugger_parse_memory(par);

    } else if(strcmp(cmd, "d") == 0) {
        shutdown(newfd, 2);
        newfd = -1;
        syslog(LOG_DEBUG, "Debugger connection closed.");

    } else if(strcmp(cmd, "q") == 0) {
        syslog(LOG_DEBUG, "Debugger requested quit.");
        exit(EXIT_SUCCESS);
    
    } else {
        dsend("- Syntax error.");
    }

    send(newfd, "? ", 2, 0);
}


static void 
debugger_parse_memory(char* par[10])
{
    if(strcmp(par[0], "r") == 0) {
        EXPECT(par, 3);
        uint32_t offset = memory_set_offset(0);
        dsend("%02X", memory_get((uint32_t)strtoll(par[1], NULL, 0)));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "r16") == 0) {
        EXPECT(par, 3);
        uint32_t offset = memory_set_offset(0);
        dsend("%04X", memory_get16((uint32_t)strtoll(par[1], NULL, 0)));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "r32") == 0) {
        EXPECT(par, 3);
        uint32_t offset = memory_set_offset(0);
        dsend("%08X", memory_get32((uint32_t)strtoll(par[1], NULL, 0)));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "w") == 0) {
        EXPECT(par, 4);
        uint32_t offset = memory_set_offset(0);
        memory_set((uint32_t)strtoll(par[1], NULL, 0), (uint8_t)strtoll(par[2], NULL, 0));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "w16") == 0) {
        EXPECT(par, 4);
        uint32_t offset = memory_set_offset(0);
        memory_set16((uint32_t)strtoll(par[1], NULL, 0), (uint16_t)strtoll(par[2], NULL, 0));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "w32") == 0) {
        EXPECT(par, 4);
        uint32_t offset = memory_set_offset(0);
        memory_set32((uint32_t)strtoll(par[1], NULL, 0), (uint32_t)strtoll(par[2], NULL, 0));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "l") == 0) {
        EXPECT(par, 4);
        uint32_t a = (uint32_t)strtoll(par[1], NULL, 0),
                 b = (uint32_t)strtoll(par[2], NULL, 0);
        if(b < a) 
            b = a;
        int i = 0;
        char* str = malloc((b - a) * 3 + 1);
        uint32_t offset = memory_set_offset(0);
        for(uint32_t addr = a; addr <= b; ++addr) {
            sprintf(&str[i], "%02X ", memory_get(addr));
            i += 3;
        }
        memory_set_offset(offset);
        dsend("%s", str);
        free(str);
    } else if(strcmp(par[0], "offset") == 0) {
        if(par[1] == NULL) {
            dsend("%08X", memory_offset());
        } else if(par[2] == NULL) {
            memory_set_offset((uint32_t)strtoll(par[1], NULL, 0));
        } else {
            dsend("- Syntax error.");
        }
    }
}


#undef EXPECT
