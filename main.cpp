#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <time.h>

using std::cout;
using std::cerr;

int main() {
    int sock;
    struct sockaddr_in addr;
    int port = 1900;
    std::string searchRequest =
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 1\r\n" // Maximum wait time in seconds
        "ST: upnp:rootdevice\r\n" // Search target
        "\r\n";
    char buffer[2048];
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);

    // Create a socket for UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("239.255.255.250");
    addr.sin_port = htons(port);

    if (sendto(sock, searchRequest.c_str(), searchRequest.length(), 0,
               (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "Failed to send message" << std::endl;
        close(sock);
        return -1;
    }

    cout << "SSDP M-SEARCH request sent." << std::endl;
    cout << "Listening for responses..." << std::endl;

    int initial_time = time(NULL);
    while (true) {
        ssize_t recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr *)&recv_addr, &recv_addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            cout << "Received response:\n" << buffer << std::endl;
        }

        if((time(NULL) - initial_time) > 2 ) {
            break;
        }
        usleep(100000); // Sleep for 100 milliseconds
    }

    // Close the socket
    close(sock);

    return 0;
}