#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>

using namespace std;
static const char *playString = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Play xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><Speed>1</Speed></u:Play></s:Body></s:Envelope>";
static const char *pauseString = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Pause xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID></u:Pause></s:Body></s:Envelope>";


size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int upnp_request(string method) {
    CURL* curl = curl_easy_init();
    if(!curl) {
        return -1;
    }

    const char* targetFunction;
    if(method == "play") {
        targetFunction = playString;
        method = "Play";
    } else if(method == "pause") {
        targetFunction = pauseString;
        method = "Pause";
    }else {
        return 0;
    }
    string actionString;
    actionString = "SOAPAction: \"urn:schemas-upnp-org:service:AVTransport:1#" + method;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, actionString.c_str());
    headers = curl_slist_append(headers, "content-type: text/xml; charset=\"utf-8\"");
    string readBuffer;
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.173:1400/MediaRenderer/AVTransport/Control");
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, targetFunction);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.4.0");
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(headers);
    headers = NULL;
    curl_easy_cleanup(curl);
    cout << readBuffer << endl;

    return 0;
}

int main() {
    string userInput;
    while(userInput != "end") {
        cout << "what do you want to do?" << endl << ">>";
        cin >> userInput;
        transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower);
        upnp_request(userInput);
    }
    cout << "Goodbye :)";
    return 0;
}