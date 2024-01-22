#include <iostream>
#include <errno.h>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main()
{
    char msearch[] = "M-SEARCH * HTTP/1.1\r\n HOST: 239.255.255.250:1900\r\nST: upnp:rootdevice\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\n\r\n";
    char rtnbuff[2048];

    int udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(1900);
    socket_addr.sin_addr.s_addr = INADDR_ANY;

    struct in_addr local_interface = {
        .s_addr = inet_addr("192.168.0.1")};

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(1900);
    dest_addr.sin_addr.s_addr = inet_addr("239.255.255.250");

    char multicast = 0;
    setsockopt(udp_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &multicast, sizeof(multicast));

    setsockopt(udp_socket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local_interface, sizeof(local_interface));

    int reuse = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    int returnedByteCount = sendto(udp_socket, &msearch, sizeof(msearch), 0, (sockaddr *)&dest_addr, sizeof(dest_addr));

    if (returnedByteCount < 0)
    {
        printf("Could not complete M-SEARCH request\n\nerror: %s\n", strerror(errno));
        close(udp_socket);
        return 1;
    }

    printf("sent packet with %d bytes...\n", returnedByteCount);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("awaiting response...\n");

    socklen_t sock_len = sizeof(dest_addr);
    returnedByteCount = recvfrom(udp_socket, rtnbuff, sizeof(rtnbuff), 0, (struct sockaddr *)&dest_addr, &sock_len);

    if (returnedByteCount == -1)
    {
        printf("Could not receive...\n\nerror: %s\n", strerror(errno));
        close(udp_socket);
        return 1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("stdout: %s", rtnbuff);
    close(udp_socket);
}