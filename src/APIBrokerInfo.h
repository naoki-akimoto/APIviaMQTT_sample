#ifndef APIBROKERINFO_H
#define APIBROKERINFO_H

struct APIBrokerInfo {
    std::string accessToken;
    std::string thingID;
    std::string clientID;
    std::string host;
    int port;
    std::string username;
    std::string password;
};

#endif//APIBROKERINFO_H
