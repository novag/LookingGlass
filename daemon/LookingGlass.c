/**
 * LookingGlass
 *
 * @package     LookingGlass
 * @author      Hendrik Hagendorn <w0h@w0h.de>
 * @copyright   2015 Hendrik Hagendorn
 * @license     http://opensource.org/licenses/MIT MIT License
 * @version     1.0.0
 *
 * C LookingGlass, handles multiple client connections using libevent
 * gcc LookingGlass.c -lresolv -levent -o lg
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <resolv.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#define SERVER_PORT 4747

int bird = 1;

char *str_replace(const char *string, const char *substr, const char *replacement) {
    char *tok = NULL;
    char *newstr = NULL;
    char *oldstr = NULL;
    char *head = NULL;

    // If substr or replacement is NULL, duplicate string, let caller handle it
    if (substr == NULL || replacement == NULL) {
        return strdup(string);
    }
    newstr = strdup(string);
    head = newstr;
    while ((tok = strstr(head, substr))) {
        oldstr = newstr;
        newstr = malloc(strlen(oldstr) - strlen(substr) + strlen(replacement) + 1);
        // Failed to alloc mem, free old string, return NULL
        if (newstr == NULL) {
            free(oldstr);
            return NULL;
        }
        memcpy(newstr, oldstr, tok - oldstr);
        memcpy(newstr + (tok - oldstr), replacement, strlen(replacement));
        memcpy(newstr + (tok - oldstr) + strlen(replacement), tok + strlen(substr), strlen(oldstr) - strlen(substr) - (tok - oldstr));
        memset(newstr + strlen(oldstr) - strlen(substr) + strlen(replacement), 0, 1);
        // Move back head after the last replacement
        head = newstr + (tok - oldstr) + strlen(replacement);
        free(oldstr);
    }

    return newstr;
}

int whois(int fd, int pv, int argc, char *argv[]) {
    if (argc < 2) {
        return 2;
    }
    argv[1] = str_replace(argv[1], "'", "");

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    snprintf(command, 100, "whois -h 172.22.0.43 '%s' 2>&1", argv[1]);

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        write(fd, out, strlen(out));
    }

    pclose(fp);

    return 0;
}

int ping(int fd, int pv, int argc, char *argv[]) {
    if (argc < 2) {
        return 2;
    }
    argv[1] = str_replace(argv[1], "'", "");

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        snprintf(command, 100, "ping -c 5 '%s' 2>&1", argv[1]);
    } else {
        snprintf(command, 100, "ping6 -c 5 '%s' 2>&1", argv[1]);
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        write(fd, out, strlen(out));
    }

    pclose(fp);

    return 0;
}

int traceroute(int fd, int pv, int argc, char *argv[]) {
    if (argc < 2) {
        return 2;
    }
    argv[1] = str_replace(argv[1], "'", "");

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        snprintf(command, 100, "traceroute '%s' 2>&1", argv[1]);
    } else {
        snprintf(command, 100, "traceroute -6 '%s' 2>&1", argv[1]);
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        write(fd, out, strlen(out));
    }

    pclose(fp);

    return 0;
}

int b_summary(int fd, int pv, int argc, char *argv[]) {
    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        strcpy(command, "/usr/sbin/birdc -r show protocols");
    } else {
        strcpy(command, "/usr/sbin/birdc6 -r show protocols");
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        if (
            (strstr(out, "BIRD") == NULL) &&
            (strstr(out, "Access restricted") == NULL) &&
            (strstr(out, "Device") == NULL) &&
            (strstr(out, "Static") == NULL) &&
            (strstr(out, "Kernel") == NULL)
        ) {
            write(fd, out, strlen(out));
        }
    }

    pclose(fp);

    return 0;
}

int q_summary(int fd, int pv, int argc, char *argv[]) {
    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        strcpy(command, "/usr/bin/vtysh -c 'show ip bgp summary'");
    } else {
        strcpy(command, "/usr/bin/vtysh -c 'show bgp summary'");
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        write(fd, out, strlen(out));
    }

    pclose(fp);

    return 0;
}

int b_route(int fd, int pv, int argc, char *argv[]) {
    if (argc > 1) {
        argv[1] = str_replace(argv[1], "'", "");
    }

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        if (argc > 1) {
            snprintf(command, 100, "/usr/sbin/birdc -r show route for '%s' all 2>&1", argv[1]);
        } else {
            strcpy(command, "/usr/sbin/birdc -r show route stats");
        }
    } else {
        if (argc > 1) {
            snprintf(command, 100, "/usr/sbin/birdc6 -r show route for '%s' all 2>&1", argv[1]);
        } else {
            strcpy(command, "/usr/sbin/birdc6 -r show route stats");
        }
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        if (
            (strstr(out, "BIRD") == NULL) &&
            (strstr(out, "Access restricted") == NULL)
        ) {
            write(fd, out, strlen(out));
        }
    }

    pclose(fp);

    return 0;
}

int q_route(int fd, int pv, int argc, char *argv[]) {
    if (argc > 1) {
        argv[1] = str_replace(argv[1], "'", "");
    }

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        if (argc > 1) {
            snprintf(command, 100, "/usr/bin/vtysh -c 'show ip bgp %s' 2>&1", argv[1]);
        } else {
            strcpy(command, "/usr/bin/vtysh -c show ip bgp");
        }
    } else {
        if (argc > 1) {
            snprintf(command, 100, "/usr/bin/vtysh -c 'show bgp %s' 2>&1", argv[1]);
        } else {
            strcpy(command, "/usr/bin/vtysh -c show bgp");
        }
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        write(fd, out, strlen(out));
    }

    pclose(fp);

    return 0;
}

int b_as(int fd, int pv, int argc, char *argv[]) {
    if (argc < 2) {
        return 2;
    }
    argv[1] = str_replace(argv[1], "'", "");

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        snprintf(command, 100, "/usr/sbin/birdc -r show route where bgp_path.last = '%s' 2>&1", argv[1]);
    } else {
        snprintf(command, 100, "/usr/sbin/birdc6 -r show route where bgp_path.last = '%s' 2>&1", argv[1]);
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        if (
            (strstr(out, "BIRD") == NULL) &&
            (strstr(out, "Access restricted") == NULL)
        ) {
            write(fd, out, strlen(out));
        }
    }

    pclose(fp);

    return 0;
}

int q_as(int fd, int pv, int argc, char *argv[]) {
    if (argc < 2) {
        return 2;
    }
    argv[1] = str_replace(argv[1], "'", "");

    FILE *fp;
    char out[2048];
    char *command = malloc(100);

    if (pv == 4) {
        snprintf(command, 100, "/usr/bin/vtysh -c 'show ip bgp regexp %s$' 2>&1", argv[1]);
    } else {
        snprintf(command, 100, "/usr/bin/vtysh -c 'show bgp regexp %s$' 2>&1", argv[1]);
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        free(command);
        return 3;
    }
    free(command);

    while (fgets(out, sizeof(out), fp) != NULL) {
        write(fd, out, strlen(out));
    }

    pclose(fp);

    return 0;
}

int tcpconnect(int fd, int pv, int argc, char *argv[]) {
    if(argc < 3) {
        return 2;
    }

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    fd_set fdset;
    struct timeval tv;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;
    if(pv == 4) {
        hints.ai_family = AF_INET;
    } else {
        hints.ai_family = AF_INET6;
    }

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        write(fd, "Name or service not known\n", 26);
        return 0;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        connect(sockfd, p->ai_addr, p->ai_addrlen);

        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
            int so_error;
            socklen_t len = sizeof so_error;

            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&so_error), &len);
            close(sockfd);

            if (so_error) {
                continue;
            }
        } else {
            close(sockfd);
            continue;
        }
        close(sockfd);

        break;
    }

    if (p == NULL) {
        write(fd, "Connection failed\n", 18);
        return 0;
    }

    write(fd, "Connection successful\n", 22);

    freeaddrinfo(servinfo);
    close(sockfd);

    return 0;
}

char *date() {
    static char date[20];
    struct tm *sTm;
    time_t now = time(0);
    sTm = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", sTm);

    return date;
}

void query_as(char *arg, char *namelisten_addr, char *buf, int buflen) {
    u_char nsbuf[4096];
    ns_msg msg;
    ns_rr rr;
    int len;

    // Set namelisten_addr
    res_init();
    struct in_addr listen_addr_in_addr;
    if (inet_pton(AF_INET, namelisten_addr, &listen_addr_in_addr) > 0) {
        _res.nscount = 1;
        _res.nsaddr_list[0].sin_addr = listen_addr_in_addr;
    }

    len = res_query(arg, ns_c_any, ns_t_txt, nsbuf, sizeof(nsbuf));
    ns_initparse(nsbuf, len, &msg);
    ns_parserr(&msg, ns_s_an, 0, &rr);
    ns_sprintrr(&msg, &rr, NULL, NULL, buf, buflen);
}

struct Callback {
    const char *name;
    const int pv;
    int (*func)(int fd, int pv, int argc, char *argv[]);
    int (*bird)(int fd, int pv, int argc, char *argv[]);
    int (*quagga)(int fd, int pv, int argc, char *argv[]);
};

static const struct Callback function_map[] = {
    { "whois",       0, whois,      NULL,      NULL },
    { "tcpconnect",  4, tcpconnect, NULL,      NULL },
    { "tcpconnect6", 6, tcpconnect, NULL,      NULL },
    { "ping",        4, ping,       NULL,      NULL },
    { "ping6",       6, ping,       NULL,      NULL },
    { "traceroute",  4, traceroute, NULL,      NULL },
    { "traceroute6", 6, traceroute, NULL,      NULL },
    { "summary",     4, NULL,       b_summary, q_summary },
    { "summary6",    6, NULL,       b_summary, q_summary },
    { "route",       4, NULL,       b_route,   q_route },
    { "route6",      6, NULL,       b_route,   q_route },
    { "as",          4, NULL,       b_as,      q_as },
    { "as6",         6, NULL,       b_as,      q_as }
};

int call_function(const char *name, int fd, int argc, char *argv[]) {
    int i;

    for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++) {
        if (!strcmp(function_map[i].name, name)) {
            if (function_map[i].func) {
                return function_map[i].func(fd, function_map[i].pv, argc, argv);
            } else if (bird && function_map[i].bird) {
                return function_map[i].bird(fd, function_map[i].pv, argc, argv);
            } else if (!bird && function_map[i].quagga) {
                return function_map[i].quagga(fd, function_map[i].pv, argc, argv);
            }
        }
    }

    return 1;
}

static void on_event(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_ERROR) {
        fprintf(stdout, "%s LookingGlass: Error: %s.\n", date(), strerror(errno));
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
}

static void on_read(struct bufferevent *bev, void *ctx) {
    struct evbuffer *input;
    int fd;
    size_t len;

    char *line;
    char *emsg;

    char **argv = malloc(sizeof(char *));
    int argc = 0;
    char *arg_ptr, *save_ptr;

    /*
     * 1: Unknown command
     * 2: Too few arguments
     * 3: Internal error
     * 4: Missing command
     */
    int status;
    int i;

    input = bufferevent_get_input(bev);
    fd = bufferevent_getfd(bev);

    len = evbuffer_get_length(input);
    line = malloc(len + 1);
    evbuffer_copyout(input, line, len);

    if (len == 0) {
        status = 4;
        goto END;
    }

    if (line[len - 1] == '\n') {
        line[len - 1] = 0;
    } else {
        line[len] = 0;
    }

    arg_ptr = strtok_r(line, " ", &save_ptr);
    while (arg_ptr != NULL) {
        len = strlen(arg_ptr) + 1;

        argv[argc] = malloc(len);
        strncpy(argv[argc], arg_ptr, len);
        argv = realloc(argv, argc + 1);

        arg_ptr = strtok_r(NULL, " ", &save_ptr);

        argc++;
    }

    printf("%s LookingGlass: Received command \"%s\" with %d arguments", date(), argv[0], argc - 1);
    for (i = 1; i < argc; i++) {
        printf(" %d:\"%s\"", i, argv[i]);
    }
    printf(".\n");

    status = call_function(argv[0], fd, argc, argv);

END:
    switch (status) {
    case 1:
        emsg = "Unknown command";
        write(fd, emsg, strlen(emsg));
        fprintf(stdout, "%s LookingGlass: Error: Disconnecting client. Reason: %s: %s.\n", date(), emsg, argv[0]);
        break;
    case 2:
        emsg = "Too few arguments";
        write(fd, emsg, strlen(emsg));
        fprintf(stdout, "%s LookingGlass: Disconnecting client. Error: %s.\n", date(), emsg);
        break;
    case 3:
        emsg = "Internal error";
        write(fd, emsg, strlen(emsg));
        fprintf(stdout, "%s LookingGlass: Disconnecting client. Error: %s.\n", date(), emsg);
        break;
    case 4:
        emsg = "Missing command";
        write(fd, emsg, strlen(emsg));
        fprintf(stdout, "%s LookingGlass: Disconnecting client. Error: %s.\n", date(), emsg);
        break;
    default:
        printf("%s LookingGlass: Disconnecting client.\n", date());
        break;
    }

    bufferevent_free(bev);

    for (i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
    free(line);
}

static void on_accept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address,
                      int socklen, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, on_read, NULL, on_event, NULL);

    bufferevent_enable(bev, EV_READ | EV_WRITE);

    printf("%s LookingGlass: Connection accepted from %s.\n", date(), inet_ntoa(
               ((struct sockaddr_in *) address)->sin_addr));
}

int main(int argc, char *argv[]) {
    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in listen_addr;

    // Set software router
    if (argc == 2 && strcmp(argv[1], "quagga") == 0) bird = 0;

    base = event_base_new();
    if (!base) {
        fprintf(stdout, "%s LookingGlass: Error: %s.\n", date(), strerror(errno));
        return 1;
    }

    // Prepare sockaddr_in structure
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(SERVER_PORT);

    listener = evconnlistener_new_bind(base, on_accept, NULL,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr*) &listen_addr, sizeof(listen_addr));
    if (!listener) {
        fprintf(stdout, "%s LookingGlass: Error: %s.\n", date(), strerror(errno));
        return 1;
    }

    printf("%s LookingGlass: Sucessfully started.\n", date());

    event_base_dispatch(base);
    return 0;
}
