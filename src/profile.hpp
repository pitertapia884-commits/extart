#pragma once

#include <webkit/webkit.h>

class Profile {
public:
    Profile();
    ~Profile();

    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;

    WebKitNetworkSession* network_session() const;
    WebKitWebContext* web_context() const;

private:
    WebKitNetworkSession* network_session_ = nullptr;
    WebKitWebContext* web_context_ = nullptr;
};
