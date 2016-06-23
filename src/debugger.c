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
#include "video.h"

#define PORT 5999

static void debugger_accept();
static void debugger_recv();
static void debugger_respond();

static void dsend(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
static void debugger_parse(char* str);
static void debugger_parse_memory(char* par[10]);
static void debugger_parse_video(char* par[10]);

static int sockfd, newfd = -1;

static char buffer[4096];

// {{{ MANAGE CONNECTION

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
    if(newfd == -1) {
        debugger_accept();
    } else {
        debugger_recv();
        debugger_respond();
    }
}


static void
debugger_accept()
{
    socklen_t sin_size = 0;
    struct sockaddr_in their_addr = {0};
    
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
        const char* welcome = "Welcome do lilliput debugger. Please type 'h' for help.\n+";
        send(newfd, welcome, strlen(welcome), 0);
        syslog(LOG_DEBUG, "send: %s", welcome);
    }
}

// }}}

// {{{ RECEIVE/SEND DATA

static void
debugger_recv()
{
    size_t pos = strlen(buffer);
    ssize_t n = recv(newfd, &buffer[pos], sizeof buffer - pos, 0);

    if(n > 0) {
        buffer[pos+(size_t)n] = '\0';   // add a null terminator to the end
        syslog(LOG_DEBUG, "recv: %s", buffer);
    } else if(n == 0) {
        syslog(LOG_DEBUG, "Debugger connection closed.");
        newfd = -1;
    } else {
        if(errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recv");
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
    syslog(LOG_DEBUG, "send: %s", buf);
}

// }}}

// {{{ PARSE DATA

static long int 
find_enter(char* s)
{
    char *a = strchr(s, '\r'),
         *b = strchr(s, '\n');
    char *c;
    if(a && !b)
        c = a;
    else if(b && !a)
        c = b;
    else if(a && b)
        c = (a < b) ? a : b;
    else
        return -1;

    return c - s;
}


static void
debugger_respond()
{
    long int enter;
    while((enter = find_enter(buffer)) != -1) {
        // parse block
        char str[enter+1];
        strncpy(str, buffer, (size_t)enter);
        str[enter] = '\0';
        debugger_parse(str);
        send(newfd, "+", 1, 0); syslog(LOG_DEBUG, "send: +");
        // remove block from buffer
        while(buffer[enter] == '\r' || buffer[enter] == '\n')
            ++enter;
        memmove(buffer, &buffer[enter], strlen(&buffer[enter])+1);
    }
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
        dsend("m r ADDR              read byte from memory address");
        dsend("m r16 ADDR            read word from memory address");
        dsend("m r32 ADDR            read dword from memory address");
        dsend("m w ADDR DATA         write byte from memory address");
        dsend("m w16 ADDR DATA       write word from memory address");
        dsend("m w32 ADDR DATA       write dword from memory address");
        dsend("m l ADD1 ADDR2        list memory data from ADDR1 to ADDR2");
        dsend("m offset [ADDR]       read/write memory offset register");
        dsend("----");
        dsend("v clr COLOR           video clear screen");
        dsend("v border COLOR        video set border to COLOR");
        dsend("v ch CHAR X Y BG FG   video write char in screen");
        dsend("v px X Y COLOR        video draw pixel");
        dsend("----");
        dsend("d                     disconnect");
        dsend("q                     quit emulator");

    } else if(strcmp(cmd, "m") == 0) {
        debugger_parse_memory(par);

    } else if(strcmp(cmd, "v") == 0) {
        debugger_parse_video(par);

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
debugger_parse_memory(char* par[10])
{
    if(par[0] == '\0') {
        dsend("- Invalid number of arguments.");
        return;
    }

    if(strcmp(par[0], "r") == 0) {
        EXPECT(par, 3);
        uint32_t offset = memory_set_offset(0);
        dsend("0x%02X", memory_get((uint32_t)strtoll(par[1], NULL, 0)));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "r16") == 0) {
        EXPECT(par, 3);
        uint32_t offset = memory_set_offset(0);
        dsend("0x%04X", memory_get16((uint32_t)strtoll(par[1], NULL, 0)));
        memory_set_offset(offset);
    } else if(strcmp(par[0], "r32") == 0) {
        EXPECT(par, 3);
        uint32_t offset = memory_set_offset(0);
        dsend("0x%08X", memory_get32((uint32_t)strtoll(par[1], NULL, 0)));
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
            sprintf(&str[i], "0x%02X ", memory_get(addr));
            i += 3;
        }
        memory_set_offset(offset);
        dsend("%s", str);
        free(str);
    } else if(strcmp(par[0], "offset") == 0) {
        if(par[1] == NULL) {
            dsend("0x%08X", memory_offset());
        } else if(par[2] == NULL) {
            memory_set_offset((uint32_t)strtoll(par[1], NULL, 0));
        } else {
            dsend("- Syntax error.");
        }
    } else {
        dsend("- Syntax error.");
    }
}


static void 
debugger_parse_video(char* par[10])
{
    if(strcmp(par[0], "clr") == 0) {
        EXPECT(par, 3);
        video_clrscr((uint8_t)strtoll(par[1], NULL, 0));
    } else if(strcmp(par[0], "border") == 0) {
        EXPECT(par, 3);
        video_setbordercolor((uint8_t)strtoll(par[1], NULL, 0));
    } else if(strcmp(par[0], "ch") == 0) {
        EXPECT(par, 7);
        char c;
        if(strlen(par[1]) > 1) {
            c = (char)strtoll(par[1], NULL, 0);
        } else {
            c = par[1][0];
        }
        video_setchar(c, 
                (uint16_t)strtoll(par[2], NULL, 0), 
                (uint16_t)strtoll(par[3], NULL, 0),
                (uint8_t)strtoll(par[4], NULL, 0), 
                (uint8_t)strtoll(par[5], NULL, 0));
    } else if(strcmp(par[0], "px") == 0) {
        EXPECT(par, 5);
        video_drawpoint(
                (uint16_t)strtoll(par[1], NULL, 0), 
                (uint16_t)strtoll(par[2], NULL, 0),
                (uint8_t)strtoll(par[3], NULL, 0));
    } else {
        dsend("- Syntax error.");
    }
}

#undef EXPECT

// }}}
