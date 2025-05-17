#pragma once

#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "ipwi4client.h"

class PWI4 : public IPWI4Client {

private:
    std::string base_url;
    CURL *curl;
    CURLcode res;

public:
    PWI4(const std::string &url);
    ~PWI4();

private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    std::string sendGetRequest(const std::string &endpoint);
    bool sendPostRequest(const std::string &endpoint, const nlohmann::json &postData);

// IPWI4Client interface
public:
    virtual void focuserConnect() override;
    virtual void focuserDisconnect() override;
    virtual void focuserEnable() override;
    virtual void focuserDisable() override;
    virtual void focuserGoto(float target) override;
    virtual void focuserStop() override;
    virtual bool focuserIsConnected() override;
    virtual bool focuserIsEnabled() override;
    virtual bool focuserIsMoving() override;
    virtual float focuserPosition() override;
};
