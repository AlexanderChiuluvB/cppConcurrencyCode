//
// Created by alex on 2020/9/15.
//

#include <csignal>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>

static bool stop = false;
static void handle_term(int sig)
{
    stop = true;
}

int main(int argc, char* argv[])
{

    signal(SIGTERM, handle_term);

    if(argc <= 3)
    {
        printf("usage: %s ip_address port_number back_log", basename(argv[0]));
    }

    const char * ip = argv[1];
    int portNumber = atoi(argv[2]);
    int backLog = atoi(argv[3]);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert (sock >= 0);

    //create a ipv4 address
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    //ipv4
    address.sin_family = AF_INET;
    address.sin_port = htons(portNumber);
    //把ipv4 / ipv6 地址转换为网络字节序整数表示的IP地址
    inet_pton(AF_INET, ip, &address.sin_addr);

    //绑定socket与地址
    int ret = bind(sock, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, backLog);
    assert(ret != -1);

    while(!stop)
    {
        sleep(1);
    }

    close(sock);
    return 0;
}