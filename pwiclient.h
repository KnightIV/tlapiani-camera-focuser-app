#pragma once

#include "ipwi4client.h"

#include <string>
#include <mutex>
#include <initializer_list>
#include <chrono>

#include <cpr/cpr.h>

class PWI4 : public IPWI4Client {

private:
    static constexpr float MAX_FOCUSER_POSITION = 17570;

    struct FocuserPropertyCache {
        static constexpr std::chrono::duration<double, std::milli> STALE_TIME = std::chrono::milliseconds(200);
        std::mutex m_propertyMutex;
        std::chrono::duration<double, std::milli> m_propertyLastUpdate;
        bool cacheUpdating = false;

        bool exists = false;
        bool isConnected = false;
        bool isMoving = false;
        float position = 0;
    };

private:
    std::string base_url;
    PWI4::FocuserPropertyCache m_focuserCache;

public:
    PWI4(const std::string &url);
    ~PWI4();

private:
    cpr::Response sendGetRequest(const std::string &endpoint,
                                 const std::initializer_list<cpr::Parameter> parameters);
    void sendGetRequestAsync(const std::string &endpoint,
                             std::function<void (cpr::Response)> callback);
    void tryUpdateFocuserCache();

// IPWI4Client interface
public:
    virtual void focuserConnect() override;
    virtual void focuserDisconnect() override;
    virtual void focuserEnable() override;
    virtual void focuserDisable() override;
    virtual void focuserGoto(float target) override;
    virtual void focuserStop() override;

    virtual bool focuserExists() override;
    virtual bool focuserIsConnected() override;
    virtual bool focuserIsEnabled() override;
    virtual bool focuserIsMoving() override;
    virtual float focuserPosition() override;
    virtual std::string clientEndpoint() override;
};
