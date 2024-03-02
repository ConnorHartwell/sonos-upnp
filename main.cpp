#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <time.h>
#include <chrono>
#include <thread>
#include <vector>
#include <regex>
#include "msearch.h"

using namespace std;

class UPnPDevice {
public:
    string loc;
    string server;
    string search_target;

    UPnPDevice(string l, string serv, string st) {
        loc = l;
        server = serv;
        search_target = st;
    }
};

string getValueFromString(string fullStr, string matcher) {
    regex location_regex = regex(matcher.append(":(.*)"), regex_constants::icase);
    smatch match;
    bool result = regex_search(fullStr, match, location_regex);
    if(result == true) {
        cout << "Found matches" << endl;
        if(match.size() == 2) {
            return match[1];
        }
        else {
            return match[0];
        }
    }
    else {
        cout << "Didn't find matches for matcher: " << matcher << endl;
    }
}

UPnPDevice convert_msearch_response_to_device(string msearchResponse) {
    return UPnPDevice(
                getValueFromString(msearchResponse,"location"),
                getValueFromString(msearchResponse,"server"),
                getValueFromString(msearchResponse,"st")
            );
}

int main() {
    vector<string> deviceList = msearch::search_for_devices();
    vector<UPnPDevice> devices;

    for(int i = 0; i < deviceList.size(); i++) {
        devices.push_back(convert_msearch_response_to_device(deviceList[i]));
    }

    cout << devices[0].loc << endl << devices[0].search_target << endl << devices[0].server;
}


vector<string> msearch::search_for_devices() {
    int sock;
    struct sockaddr_in addr;
    int port = 1900;
    vector<string> results;
    string searchRequest =
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 1\r\n"
        "ST: upnp:rootdevice\r\n"
        "\r\n";
    char buffer[2048];
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);

    // Create a socket for UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Failed to create socket" << std::endl;
        return results;
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 100;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("239.255.255.250");
    addr.sin_port = htons(port);

    if (sendto(sock, searchRequest.c_str(), searchRequest.length(), 0,
               (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "Failed to send message" << std::endl;
        close(sock);
        return results;
    }

    cout << "SSDP M-SEARCH request sent." << endl;
    cout << "Listening for responses..." << endl;

    int initial_time = time(NULL);
    while (time(NULL) - initial_time < 3) {
        ssize_t recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr *)&recv_addr, &recv_addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            results.push_back(buffer);
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    close(sock);

    return results;
}