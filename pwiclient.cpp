#include "pwiclient.h"

#include <QDebug>

void debugOutput(std::initializer_list<std::string> outputs) {
    QTextStream qss;
    for (std::string s : outputs) {
        qss << QString(s.c_str());
    }
    qInfo() << qss.string();
}

std::string getFocuserProperty(const std::string &pwiResponse, const std::string property) {
    std::stringstream ss(pwiResponse);

    std::string line;
    while (std::getline(ss, line, '\n')) {
        if (line.starts_with("focuser")) {
            size_t propNameIdx = line.find('.') + 1;
            size_t propValueIdx = line.find('=') + 1;
            std::string curProp = line.substr(propNameIdx, (propValueIdx - 1) - propNameIdx);

            if (curProp.at(curProp.length() - 1) == '\r') {
                curProp = curProp.erase(curProp.length() - 1);
            }

            if (property == curProp) {
                return line.substr(propValueIdx);
            }
        }
    }

    throw std::logic_error(property + " not found");
}

float getFocuserPropertyFloat(const std::string &pwiResponse, const std::string property) {
    return std::stof(getFocuserProperty(pwiResponse, property));
}

bool getFocuserPropertyBool(const std::string &pwiResponse, const std::string property) {
    return getFocuserProperty(pwiResponse, property) == "true";
}

PWI4::PWI4(const std::string &url) : base_url(url) {}

PWI4::~PWI4() {}

cpr::Response PWI4::sendGetRequest(const std::string &endpoint,
                                   const std::initializer_list<cpr::Parameter> parameters={}) {
    const std::string url = base_url + endpoint;
    return cpr::Get(cpr::Url(url), cpr::Parameters(parameters));
}

void PWI4::sendGetRequestAsync(const std::string &endpoint, std::function<void (cpr::Response)> callback) {
    const std::string url = base_url + endpoint;
    cpr::GetCallback(callback, cpr::Url(url), cpr::Timeout{5000});
}

void PWI4::tryUpdateFocuserCache() {
    std::chrono::duration curTime = std::chrono::system_clock::now().time_since_epoch();
    m_focuserCache.m_propertyMutex.lock();
    if (curTime - m_focuserCache.m_propertyLastUpdate > PWI4::FocuserPropertyCache::STALE_TIME && !m_focuserCache.cacheUpdating) {
        m_focuserCache.cacheUpdating = true;
        m_focuserCache.m_propertyLastUpdate = curTime;

        // enable seems to be a good endpoint that doesn't functionally do anything but still returns app status
        sendGetRequestAsync("/focuser/enable", [=, this](cpr::Response r) {
            qInfo() << r.url.str() << r.status_code;
            switch (r.status_code) {
            case cpr::status::HTTP_OK:
                m_focuserCache.m_propertyMutex.lock();
                m_focuserCache.exists = true;
                m_focuserCache.isConnected = getFocuserPropertyBool(r.text, "is_connected");
                m_focuserCache.position = getFocuserPropertyFloat(r.text, "position");
                m_focuserCache.isMoving = getFocuserPropertyBool(r.text, "is_moving");

                m_focuserCache.m_propertyMutex.unlock();
                break;

            case cpr::status::HTTP_NOT_FOUND:
                qInfo() << "404 received. Response text: " << r.text;
                break;

            case 0:
                m_focuserCache.m_propertyMutex.lock();
                m_focuserCache.exists = false;
                m_focuserCache.m_propertyMutex.unlock();
                break;

            default:
                qInfo() << "Error received from" << r.url.str() << " | Error code: " << r.status_code;
                break;
            }

            m_focuserCache.cacheUpdating = false;
        });
    }
    m_focuserCache.m_propertyMutex.unlock();
}

// IPWIClient interface
void PWI4::focuserConnect() {
    sendGetRequest("/focuser/connect");
}

void PWI4::focuserDisconnect() {
    sendGetRequest("/focuser/disconnect");
}

void PWI4::focuserEnable() {
    throw std::logic_error("not implemented");
}

void PWI4::focuserDisable() {
    throw std::logic_error("not implemented");
}

void PWI4::focuserGoto(float target) {
    auto r = sendGetRequest("/focuser/goto", { {"target", std::to_string(target)} });
    qInfo() << r.url.str() << r.status_code;
}

void PWI4::focuserStop() {
    sendGetRequest("/focuser/stop");
}

bool PWI4::focuserExists() {
    tryUpdateFocuserCache();
    return m_focuserCache.exists;
}

bool PWI4::focuserIsConnected() {
    tryUpdateFocuserCache();
    return m_focuserCache.isConnected;
}

bool PWI4::focuserIsEnabled() {
    throw std::logic_error("not implemented");
}

bool PWI4::focuserIsMoving() {
    tryUpdateFocuserCache();
    return m_focuserCache.isMoving;
}

float PWI4::focuserPosition() {
    tryUpdateFocuserCache();
    return m_focuserCache.position;
}

std::string PWI4::clientEndpoint() {
    return base_url;
}
